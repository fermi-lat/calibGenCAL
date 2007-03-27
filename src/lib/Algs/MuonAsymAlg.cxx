// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/MuonAsymALg.cxx,v 1.15 2007/02/27 20:44:13 fewtrell Exp $

/** @file
    @author Zachary Fewtrell
 */

// LOCAL INCLUDES
#include "MuonAsymAlg.h"
#include "../Util/RootFileAnalysis.h"
#include "../Util/TwrHodoscope.h"
#include "../CalibDataTypes/CalAsym.h"
#include "../Util/CGCUtil.h"
#include "../Hists/AsymHists.h"

// GLAST INCLUDES
#include "digiRootData/DigiEvent.h"

// EXTLIB INCLUDES
#include "TH2S.h"

// STD INCLUDES
#include <sstream>

using namespace std;
using namespace CalUtil;
using namespace CGCUtil;

MuonAsymALg::MuonAsymALg(const CalPed &ped,
                   const CIDAC2ADC &dac2adc,
                   AsymHists &asymHists) :
  eventData(ped, dac2adc),
  m_asymHists(asymHists)
{
}

bool MuonAsymALg::passCutX(const TwrHodoscope &hscope) {
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

bool MuonAsymALg::passCutY(const TwrHodoscope &hscope) {
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

void MuonAsymALg::processEvent(DigiEvent &digiEvent) {
  // check that we are in 4 range mode
  EventSummaryData &summary = digiEvent.getEventSummaryData();
  if (!summary.readout4())
    return;

  const TClonesArray *calDigiCol = digiEvent.getCalDigiCol();
  if (!calDigiCol) {
    LogStream::get() << "no calDigiCol found for event#" << eventData.eventNum << endl;
    return;
  }

  TIter calDigiIter(calDigiCol);

  const CalDigi      *pCalDigi = 0;

  //loop through each 'hit' in one event
  while ((pCalDigi = (CalDigi *)calDigiIter.Next())) {
    const CalDigi &calDigi = *pCalDigi;            // use reference to avoid -> syntax

    //-- XtalId --//
    idents::CalXtalId id(calDigi.getPackedId());   // get interaction information

    // retrieve tower info
    TwrNum twr = id.getTower();

    // add hit to appropriate hodoscope.
    eventData.hscopes[twr].addHit(calDigi);
  }

  // process each tower for possible good muon event
  for (TwrNum twr; twr.isValid(); twr++) {
    TwrHodoscope &hscope = eventData.hscopes[twr];
    processTower(hscope);
  }  // per tower loop
}

void MuonAsymALg::processTower(TwrHodoscope &hscope) {
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
      XtalIdx xtalIdx = hitListOrtho[i];

      // calcuate the 4 log ratios = log(POS/NEG)
      for (AsymType asymType; asymType.isValid(); asymType++) {
        float dacP = hscope.dac[tDiodeIdx(xtalIdx.getTXtalIdx(), POS_FACE, asymType.getDiode(POS_FACE))];
        float dacN = hscope.dac[tDiodeIdx(xtalIdx.getTXtalIdx(), NEG_FACE, asymType.getDiode(NEG_FACE))];
        float asym = log(dacP/dacN);

        m_asymHists.getHist(asymType, xtalIdx)->Fill(pos, asym);
      }
      //           logStrm << "HIT: " << eventData.eventNum
      //                   << " " << xtalIdx.val()
      //                   << " " << m_histograms[ASYM_SS][xtalIdx]->GetEntries()
      //                   << endl;

      algData.nHits++;
    }   // per hit loop
  }     // per direction loop
}

void MuonAsymALg::fillHists(unsigned nEntries,
                         const vector<string> &rootFileList) {
  m_asymHists.initHists();

  RootFileAnalysis rootFile(0,
                            &rootFileList,
                            0);

  // enable only needed branches in root file
  rootFile.getDigiChain()->SetBranchStatus("*", 0);
  rootFile.getDigiChain()->SetBranchStatus("m_calDigiCloneCol");
  rootFile.getDigiChain()->SetBranchStatus("m_summary");

  unsigned nEvents = rootFile.getEntries();
  LogStream::get() <<
  __FILE__ << ": Processing: " << nEvents << " events." << endl;

  // Basic digi-event loop
  for (eventData.eventNum = 0; eventData.eventNum < nEvents; eventData.eventNum++) {
    eventData.next();
    if (eventData.eventNum % 10000 == 0) {
      // quit if we have enough entries in each histogram
      unsigned currentMin = m_asymHists.getMinEntries();
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
  }  // per event loop

  LogStream::get() << "Asymmetry histograms filled nEvents=" << algData.nGoodDirs
                   << " algData.nXDirs="               << algData.nXDirs
                   << " algData.nYDirs="               << algData.nYDirs << endl;
  LogStream::get() << " nHits measured="       <<               algData.nHits
                   << " Bad hits="             << algData.nBadHits
                   << endl;
}

