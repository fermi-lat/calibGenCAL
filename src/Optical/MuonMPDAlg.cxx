// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Algs/MuonMPDAlg.cxx,v 1.6 2008/01/22 19:40:59 fewtrell Exp $

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "MuonMPDAlg.h"
#include "src/lib/Hists/MPDHists.h"
#include "src/lib/Specs/CalGeom.h"
#include "src/lib/Util/RootFileAnalysis.h"
#include "src/lib/Util/TwrHodoscope.h"
#include "src/lib/Util/CGCUtil.h"

// GLAST INCLUDES
#include "digiRootData/DigiEvent.h"
#include "CalUtil/SimpleCalCalib/CIDAC2ADC.h"
#include "CalUtil/SimpleCalCalib/CalPed.h"
#include "CalUtil/SimpleCalCalib/CalAsym.h"
#include "CalUtil/SimpleCalCalib/CalMPD.h"

// EXTLIB INCLUDES

// STD INCLUDES
#include <sstream>

namespace calibGenCAL {

  using namespace std;
  using namespace CalUtil;
  using namespace CalGeom;

  MuonMPDAlg::MuonMPDAlg(const CalPed &ped,
                         const CIDAC2ADC &dac2adc,
                         const CalAsym &calAsym,
                         MPDHists &mpdHists) :
    algData(calAsym),
    eventData(ped, dac2adc),
    m_mpdHists(mpdHists)
  {
  }

  void MuonMPDAlg::fillHists(const unsigned nEntries,
                             const vector<string> &rootFileList) {
    m_mpdHists.initHists();

    RootFileAnalysis rootFile(0,
                              &rootFileList,
                              0);

    // enable only needed branches in root file
    rootFile.getDigiChain()->SetBranchStatus("*", 0);
    rootFile.getDigiChain()->SetBranchStatus("m_calDigiCloneCol");
    rootFile.getDigiChain()->SetBranchStatus("m_summary");

    const unsigned nEvents = rootFile.getEntries();
    LogStrm::get() << __FILE__ << ": Processing: " << nEvents << " events." << endl;

    ///////////////////////////////////////////
    // DIGI Event Loop - Fill Twr Hodoscopes //
    ///////////////////////////////////////////
    for (eventData.eventNum = 0; eventData.eventNum < nEvents; eventData.eventNum++) {
      eventData.next();

      if (eventData.eventNum % 10000 == 0) {
        // quit if we have enough entries in each histogram
        const unsigned currentMin = m_mpdHists.getMinEntries();
        if (currentMin >= nEntries) break;
        LogStrm::get() << "Event: " << eventData.eventNum
                         << " min entries per histogram: " << currentMin
                         << endl;
        LogStrm::get().flush();
      }

      if (!rootFile.getEvent(eventData.eventNum)) {
        LogStrm::get() << "Warning, event " << eventData.eventNum << " not read." << endl;
        continue;
      }

      DigiEvent const*const digiEvent = rootFile.getDigiEvent();
      if (!digiEvent) {
        LogStrm::get() << __FILE__ << ": Unable to read DigiEvent " << eventData.eventNum  << endl;
        continue;
      }

      processEvent(*digiEvent);
    }
  }

  bool MuonMPDAlg::passCutX(const TwrHodoscope &hscope) {
    // max 2 hits on any layer
    if (hscope.maxPerLyr > 2)
      return false;

    // need vertical connect 4 in X dir
    if (hscope.nLyrsX != 4 || hscope.nColsX != 1)
      return false;

    // need at least 2 points to get an orthogonal track
    if (hscope.nLyrsY < 2) return false;

    return true;
  }

  bool MuonMPDAlg::passCutY(const TwrHodoscope &hscope) {
    // max 2 hits on any layer
    if (hscope.maxPerLyr > 2)
      return false;

    // need vertical connect 4 in Y dir
    if (hscope.nLyrsY != 4 || hscope.nColsY != 1)
      return false;

    // need at least 2 points to get an orthogonal track
    if (hscope.nLyrsX < 2) return false;

    return true;
  }

  void MuonMPDAlg::processEvent(const DigiEvent &digiEvent) {
    // now trying to work w/ 1 range data, just don't fill all hists.
    //   // check that we are in 4 range mode
    //   EventSummaryData &summary = digiEvent.getEventSummaryData();
    //   if (!summary.readout4())
    //     return;

    TClonesArray const*const calDigiCol = digiEvent.getCalDigiCol();


    if (!calDigiCol) {
      LogStrm::get() << "no calDigiCol found for event#" << eventData.eventNum << endl;
      return;
    }

    TIter calDigiIter(calDigiCol);

    const CalDigi      *pCalDigi = 0;

    // loop through each 'hit' in one event
    while ((pCalDigi = dynamic_cast<CalDigi *>(calDigiIter.Next()))) {
      const CalDigi &calDigi = *pCalDigi;            // use reference to avoid -> syntax

      //-- XtalId --//
      const idents::CalXtalId id(calDigi.getPackedId());   // get interaction information

      // retrieve tower info
      const TwrNum twr = id.getTower();

      // add hit to appropriate hodoscope.
      eventData.hscopes[twr].addHit(calDigi);
    }

    ///////////////////////////////////////////
    // Search Twr Hodoscopes for good events //
    ///////////////////////////////////////////
    for (TwrNum twr; twr.isValid(); twr++) {
      TwrHodoscope &hscope = eventData.hscopes[twr];

      processTower(hscope);
    }
  }

  void MuonMPDAlg::processTower(TwrHodoscope &hscope) {
    // summarize the event for each hodoscope
    hscope.summarizeEvent();

    // CHECK BOTH DIRECTIONS FOR USABLE EVENT

    /** Note: 'direction' refers to the direction of xtals which have vertical
        'connect-4' deposits.  For MevPerDAC, the orthogonal hits will be used to
        determine the pathlength for these 4 hits.
    */

    for (DirNum dir; dir.isValid(); dir++) {
      // skip if we don't have a good track
      if (dir == X_DIR && !passCutX(hscope)) continue;
      else if (dir == Y_DIR && !passCutY(hscope)) continue;

      // count used events

      // copy hit lists to local var since
      // i will be removing some hits and
      // the lists may need to be reused for
      // the next oritentation (X vs Y)
      vector<XtalIdx> hitList;
      vector<XtalIdx> hitListOrtho;

      if (dir == X_DIR) {
        algData.nXEvents++;
        hitList      = hscope.hitListX;
        hitListOrtho = hscope.hitListY;
      } else {
        algData.nYEvents++;
        hitList      = hscope.hitListY;
        hitListOrtho = hscope.hitListX;
      }

      //-- GET HODOSCOPIC TRACK FROM ORTHOGONAL XTALS --//
      algData.graph.Set(hitListOrtho.size());

      // fill in each point val
      for (unsigned i = 0; i < hitListOrtho.size(); i++) {
        const XtalIdx xtalIdx(hitListOrtho[i]);
        const LyrNum  lyr(xtalIdx.getLyr());
        const ColNum  col(xtalIdx.getCol());
        algData.graph.SetPoint(i, lyr.val(), col.val());
      }

      // fit straight line through graph
      algData.graph.Fit(&algData.lineFunc, "WQN");

      // throw out events which are greater than about 30 deg from vertical
      float lineSlope(algData.lineFunc.GetParameter(1));
      if (abs(lineSlope) > 0.5) continue;

      //-- THROW OUT HITS NEAR END OF XTAL --//
      // loop through each hit in X direction, remove bad xtals
      // bad xtals have energy centroid at col 0 or 11 (-5 or +5 since center of xtal is 0)
      for (unsigned i = 0; i < hitList.size(); i++) {
        const XtalIdx xtalIdx(hitList[i]);
        const LyrNum  lyr(xtalIdx.getLyr());

        const float   hitPos(algData.lineFunc.Eval(lyr.val()));    // find column for given lyr

        //throw out event if energy centroid is in column 0 or 11 (3cm from end)
        if (hitPos < 1 || hitPos > 10) {
          hitList.erase(hitList.begin()+i);
          i--;
        }
      }

      // occasionally there will be no good hits!
      if (hitList.size() < 1) continue;

      // improve hodoscopic slope if there are enough good points left
      // to fit a straight line
      if (hitList.size() > 1) {
        //-- IMPROVE TRACK W/ ASYMMETRY FROM GOOD XTALS --//
        // now that we have eliminated events on the ends of xtals, we can use
        // asymmetry to get a higher precision slope
        algData.graph.Set(hitList.size());    // reset graph size in case we removed any invalid points.
        for (unsigned i = 0; i < hitList.size(); i++) {
          const XtalIdx xtalIdx(hitList[i]);
          const LyrNum  lyr(xtalIdx.getLyr());

          // calcuate the log ratio = log(POS/NEG)
          const float   dacP(hscope.dac[tDiodeIdx(xtalIdx.getTXtalIdx(), POS_FACE, LRG_DIODE)]);
          const float   dacN(hscope.dac[tDiodeIdx(xtalIdx.getTXtalIdx(), NEG_FACE, LRG_DIODE)]);
          const float   asymLL(log(dacP/dacN));

          // get new position from asym
          const float   hitPos(algData.calAsym.asym2pos(xtalIdx, LRG_DIODE, asymLL));

          algData.graph.SetPoint(i, lyr.val(), hitPos);
        }

        algData.graph.Fit(&algData.lineFunc, "WQN");
        lineSlope = algData.lineFunc.GetParameter(1);
      }

      // NUMERIC CONSTANTS
      // converts between lyr/col units & mm
      // real trigonometry is needed for pathlength calculation
      static const float slopeFactor(CalGeom::cellHorPitch/CalGeom::cellVertPitch);

      //-- Pathlength Correction --//
      //slope = rise/run = dy/dx = colPos/lyrNum
      const float    tan(lineSlope*slopeFactor);
      const float    sec(sqrt(1 + tan*tan));             //sec proportional to hyp which is pathlen.

      // poulate histograms & apply pathlength correction
      const unsigned nHits = hitList.size();
      for (unsigned i = 0; i < nHits; i++) {
        const XtalIdx xtalIdx(hitList[i]);

        // calculate meanDAC for each diode size.
        CalVec<DiodeNum, float> meanDAC;
        for (DiodeNum diode; diode.isValid(); diode++) {
          meanDAC[diode]  = sqrt(hscope.dac[tDiodeIdx(xtalIdx.getTXtalIdx(), POS_FACE, diode)] *
                                 hscope.dac[tDiodeIdx(xtalIdx.getTXtalIdx(), NEG_FACE, diode)]);

          meanDAC[diode] /= sec;
        }

        m_mpdHists.fillDacLL(xtalIdx, meanDAC[LRG_DIODE]);
        m_mpdHists.fillL2S(xtalIdx, meanDAC[LRG_DIODE], meanDAC[SM_DIODE]);

        // load dacL2S profile

        algData.nXtals++;
      }
    }
  }

  MuonMPDAlg::AlgData::AlgData(const CalAsym &asym) :
    canvas("canvas", "event display", 800, 600),
    graph(4),
    lineFunc("line", "pol1", 0, 8),
    viewHist("viewHist", "viewHist",
             8, -0.5, 7.5,    //X-limits lyr
             12, -0.5, 11.5),
    //Y-limits col
    calAsym(asym)
  {
    init();

    ////////////////////////////////////////////////////
    // INITIALIZE ROOT PLOTTING OBJECTS FOR LINE FITS //
    ////////////////////////////////////////////////////
    // viewHist is used to set scale before drawing TGraph
    viewHist.Draw();
    graph.Draw("*");
    canvas.Update();
  }

}; // namespace calibGenCAL
