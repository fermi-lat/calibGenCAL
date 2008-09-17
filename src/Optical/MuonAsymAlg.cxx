// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/Optical/MuonAsymAlg.cxx,v 1.2 2008/04/22 18:36:04 fewtrell Exp $

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "MuonAsymAlg.h"
#include "src/lib/Util/RootFileAnalysis.h"
#include "src/lib/Util/TwrHodoscope.h"
#include "src/lib/Hists/AsymHists.h"
#include "src/lib/Util/CGCUtil.h"
#include "src/lib/Specs/CalGeom.h"

// GLAST INCLUDES
#include "digiRootData/DigiEvent.h"
#include "CalUtil/SimpleCalCalib/CalAsym.h"

// EXTLIB INCLUDES
#include "TH2S.h"

// STD INCLUDES
#include <sstream>

namespace calibGenCAL {

  using namespace std;
  using namespace CalUtil;

  MuonAsymAlg::MuonAsymAlg(const CalPed &ped,
                           const CIDAC2ADC &dac2adc,
                           AsymHists &asymHists) :
    eventData(ped, dac2adc),
    m_asymHists(asymHists)
  {
  }

  bool MuonAsymAlg::passCutX(const TwrHodoscope &hscope) {
    // max 2 hits on any layer
    if (hscope.maxPerLyr > 2)
      return false;

    // need vertical connect 4 in X dir
    if (hscope.nLyrsX != 4 || hscope.nColsX != 1)
      return false;

    // skip extreme ends of xtal, as variance is high.
    if (hscope.firstColX == 0 || hscope.firstColX == 11)
      return false;

    // need at least one hit in ortho direction,
    // otherwise there is nothing to calibrate
    if (!hscope.nLyrsY) return false;

    return true;
  }

  bool MuonAsymAlg::passCutY(const TwrHodoscope &hscope) {
    // max 2 hits on any layer
    if (hscope.maxPerLyr > 2)
      return false;

    // need vertical connect 4 in Y dir
    if (hscope.nLyrsY != 4 || hscope.nColsY != 1)
      return false;

    // skip extreme ends of xtal, as variance is high.
    if (hscope.firstColY == 0 || hscope.firstColY == 11)
      return false;

    // need at least one hit in ortho direction,
    // otherwise there is nothing to calibrate
    if (!hscope.nLyrsX) return false;

    return true;
  }

  void MuonAsymAlg::processEvent(const DigiEvent &digiEvent) {
    // check that we are in 4 range mode
    EventSummaryData &summary = const_cast<EventSummaryData&>(digiEvent.getEventSummaryData());
    if (!summary.readout4())
      return;

    TClonesArray const*const calDigiCol = digiEvent.getCalDigiCol();
    if (!calDigiCol) {
      LogStrm::get() << "no calDigiCol found for event#" << eventData.eventNum << endl;
      return;
    }

    TIter calDigiIter(calDigiCol);

    const CalDigi      *pCalDigi = 0;

    //loop through each 'hit' in one event
    while ((pCalDigi = dynamic_cast<CalDigi *>(calDigiIter.Next()))) {
      const CalDigi &calDigi = *pCalDigi;            // use reference to avoid -> syntax

      //-- XtalId --//
      const idents::CalXtalId id(calDigi.getPackedId());   // get interaction information

      // retrieve tower info
      const TwrNum twr (id.getTower());

      // add hit to appropriate hodoscope.
      eventData.hscopes[twr].addHit(calDigi);
    }

    // process each tower for possible good muon event
    for (TwrNum twr; twr.isValid(); twr++) {
      TwrHodoscope &hscope = eventData.hscopes[twr];
      processTower(hscope);
    }  // per tower loop
  }

  void MuonAsymAlg::processTower(TwrHodoscope &hscope) {
    // summarize the event for each hodoscope
    hscope.summarizeEvent();

    for (DirNum dir; dir.isValid(); dir++) {
      unsigned short pos;
      vector<XtalIdx> *pHitList, *pHitListOrtho;

      // DIRECTION SPECIFIC SETUP //

      /** Note: 'direction' refers to the direction of xtals which have vertical
          'connect-4' deposits.  For asymmetry, we use this vertical column
          to calibrate the signal in the orthogonal crystals.
      */
      if (dir == X_DIR) {
        if (!passCutX(hscope)) continue;     // skip this direction if track is bad
        pos           = hscope.firstColX;
        pHitList      = &hscope.hitListX;    // hit list in test direction
        pHitListOrtho = &hscope.hitListY;    // ortho direction
        algData.nXDirs++;
      } else {
        // Y_DIR
        if (!passCutY(hscope)) continue;     // skip this direction if track is bad
        pos           = hscope.firstColY;
        pHitList      = &hscope.hitListY;    // hit list in test direction
        pHitListOrtho = &hscope.hitListX;    // ortho direction
        algData.nYDirs++;
      }

      algData.nGoodDirs++;

      // use references to avoid -> notation
      vector<XtalIdx> &hitListOrtho = *pHitListOrtho;

      // loop through each orthogonal hit
      for (unsigned i = 0; i < hitListOrtho.size(); i++) {
        const XtalIdx xtalIdx(hitListOrtho[i]);

        // calcuate the 4 log ratios = log(POS/NEG)
        for (AsymType asymType; asymType.isValid(); asymType++) {
          const float dacP = hscope.dac[tDiodeIdx(xtalIdx.getTXtalIdx(), POS_FACE, asymType.getDiode(POS_FACE))];
          const float dacN = hscope.dac[tDiodeIdx(xtalIdx.getTXtalIdx(), NEG_FACE, asymType.getDiode(NEG_FACE))];

          m_asymHists.fill(asymType, 
                           xtalIdx, 
                           xtalSliceToMMFromCtr(pos,ColNum::N_VALS), 
                           dacP, 
                           dacN);
        }
        //           logStrm << "HIT: " << eventData.eventNum
        //                   << " " << xtalIdx.val()
        //                   << " " << m_histograms[ASYM_SS][xtalIdx]->GetEntries()
        //                   << endl;

        algData.nHits++;
      }   // per hit loop
    }     // per direction loop
  }

  void MuonAsymAlg::fillHists(unsigned nEntries,
                              const vector<string> &rootFileList) {
    RootFileAnalysis rootFile(0,
                              &rootFileList,
                              0);

    // enable only needed branches in root file
    rootFile.getDigiChain()->SetBranchStatus("*", 0);
    rootFile.getDigiChain()->SetBranchStatus("m_calDigiCloneCol");
    rootFile.getDigiChain()->SetBranchStatus("m_summary");

    const unsigned nEvents = rootFile.getEntries();
    LogStrm::get() <<
      __FILE__ << ": Processing: " << nEvents << " events." << endl;

    // Basic digi-event loop
    for (eventData.eventNum = 0; eventData.eventNum < nEvents; eventData.eventNum++) {
      eventData.next();
      if (eventData.eventNum % 10000 == 0) {
        // quit if we have enough entries in each histogram
        const unsigned currentMin = m_asymHists.getMinEntries();
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
    }  // per event loop

    LogStrm::get() << "Asymmetry histograms filled nEvents=" << algData.nGoodDirs
                     << " algData.nXDirs="               << algData.nXDirs
                     << " algData.nYDirs="               << algData.nYDirs << endl;
    LogStrm::get() << " nHits measured="       <<               algData.nHits
                     << " Bad hits="             << algData.nBadHits
                     << endl;
  }

}; // namespace calibGenCAL
