// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Algs/MuonMPDAlg.cxx,v 1.1 2007/03/27 18:50:49 fewtrell Exp $

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "MuonMPDAlg.h"
#include "../Util/RootFileAnalysis.h"
#include "../CalibDataTypes/CIDAC2ADC.h"
#include "../Util/TwrHodoscope.h"
#include "../Specs/CalGeom.h"
#include "../CalibDataTypes/CalPed.h"
#include "../CalibDataTypes/CalAsym.h"
#include "../CalibDataTypes/CalMPD.h"
#include "../Util/CGCUtil.h"
#include "../Hists/MPDHists.h"

// GLAST INCLUDES
#include "digiRootData/DigiEvent.h"

// EXTLIB INCLUDES

// STD INCLUDES
#include <sstream>

using namespace std;
using namespace CalUtil;
using namespace CalGeom;
using namespace CGCUtil;

MuonMPDAlg::MuonMPDAlg(const CalPed &ped,
                       const CIDAC2ADC &dac2adc,
                       const CalAsym &calAsym,
                       MPDHists &mpdHists) :
  algData(calAsym),
  eventData(ped, dac2adc),
  m_mpdHists(mpdHists)
{
}

void MuonMPDAlg::fillHists(unsigned nEntries,
                           const vector<string> &rootFileList) {
  m_mpdHists.initHists();

  RootFileAnalysis rootFile(0,
                            &rootFileList,
                            0);

  // enable only needed branches in root file
  rootFile.getDigiChain()->SetBranchStatus("*", 0);
  rootFile.getDigiChain()->SetBranchStatus("m_calDigiCloneCol");
  rootFile.getDigiChain()->SetBranchStatus("m_summary");

  unsigned nEvents = rootFile.getEntries();
  LogStream::get() << __FILE__ << ": Processing: " << nEvents << " events." << endl;

  ///////////////////////////////////////////
  // DIGI Event Loop - Fill Twr Hodoscopes //
  ///////////////////////////////////////////
  for (eventData.eventNum = 0; eventData.eventNum < nEvents; eventData.eventNum++) {
    eventData.next();

    if (eventData.eventNum % 10000 == 0) {
      // quit if we have enough entries in each histogram
      unsigned currentMin = m_mpdHists.getMinEntries();
      if (currentMin >= nEntries) break;
      LogStream::get() << "Event: " << eventData.eventNum
                       << " min entries per histogram: " << currentMin
                       << endl;
      LogStream::get().flush();
    }

    if (!rootFile.getEvent(eventData.eventNum)) {
      LogStream::get() << "Warning, event " << eventData.eventNum << " not read." << endl;
      continue;
    }

    DigiEvent *digiEvent = rootFile.getDigiEvent();
    if (!digiEvent) {
      LogStream::get() << __FILE__ << ": Unable to read DigiEvent " << eventData.eventNum  << endl;
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

void MuonMPDAlg::processEvent(DigiEvent &digiEvent) {
  // now trying to work w/ 1 range data, just don't fill all hists.
  //   // check that we are in 4 range mode
  //   EventSummaryData &summary = digiEvent.getEventSummaryData();
  //   if (!summary.readout4())
  //     return;

  const TClonesArray *calDigiCol = digiEvent.getCalDigiCol();


  if (!calDigiCol) {
    LogStream::get() << "no calDigiCol found for event#" << eventData.eventNum << endl;
    return;
  }

  TIter calDigiIter(calDigiCol);

  const CalDigi      *pCalDigi = 0;

  // loop through each 'hit' in one event
  while ((pCalDigi = (CalDigi *)calDigiIter.Next())) {
    const CalDigi &calDigi = *pCalDigi;            // use reference to avoid -> syntax

    //-- XtalId --//
    idents::CalXtalId id(calDigi.getPackedId());   // get interaction information

    // retrieve tower info
    TwrNum twr = id.getTower();

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
      XtalIdx xtalIdx = hitListOrtho[i];
      LyrNum  lyr     = xtalIdx.getLyr();
      ColNum  col     = xtalIdx.getCol();
      algData.graph.SetPoint(i, lyr, col);
    }

    // fit straight line through graph
    algData.graph.Fit(&algData.lineFunc, "WQN");

    // throw out events which are greater than about 30 deg from vertical
    float lineSlope = algData.lineFunc.GetParameter(1);
    if (abs(lineSlope) > 0.5) continue;

    //-- THROW OUT HITS NEAR END OF XTAL --//
    // loop through each hit in X direction, remove bad xtals
    // bad xtals have energy centroid at col 0 or 11 (-5 or +5 since center of xtal is 0)
    for (unsigned i = 0; i < hitList.size(); i++) {
      XtalIdx xtalIdx = hitList[i];
      LyrNum  lyr     = xtalIdx.getLyr();

      float   hitPos  = algData.lineFunc.Eval(lyr);    // find column for given lyr

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
        XtalIdx xtalIdx = hitList[i];
        LyrNum  lyr     = xtalIdx.getLyr();

        // calcuate the log ratio = log(POS/NEG)
        float   dacP    = hscope.dac[tDiodeIdx(xtalIdx.getTXtalIdx(), POS_FACE, LRG_DIODE)];
        float   dacN    = hscope.dac[tDiodeIdx(xtalIdx.getTXtalIdx(), NEG_FACE, LRG_DIODE)];
        float   asymLL  = log(dacP/dacN);

        // get new position from asym
        float   hitPos  = algData.calAsym.asym2pos(xtalIdx, LRG_DIODE, asymLL);

        algData.graph.SetPoint(i, lyr, hitPos);
      }

      algData.graph.Fit(&algData.lineFunc, "WQN");
      lineSlope = algData.lineFunc.GetParameter(1);
    }

    // NUMERIC CONSTANTS
    // converts between lyr/col units & mm
    // real trigonometry is needed for pathlength calculation
    static const float slopeFactor = CalGeom::cellHorPitch/CalGeom::cellVertPitch;

    //-- Pathlength Correction --//
    //slope = rise/run = dy/dx = colPos/lyrNum
    float    tan   = lineSlope*slopeFactor;
    float    sec   = sqrt(1 + tan*tan);             //sec proportional to hyp which is pathlen.

    // poulate histograms & apply pathlength correction
    unsigned nHits = hitList.size();
    for (unsigned i = 0; i < nHits; i++) {
      XtalIdx xtalIdx = hitList[i];

      // calculate meanDAC for each diode size.
      CalArray<DiodeNum, float> meanDAC;
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

