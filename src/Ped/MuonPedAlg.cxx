// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/Ped/MuonPedAlg.cxx,v 1.5 2008/05/13 16:54:00 fewtrell Exp $

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "src/lib/Util/RootFileAnalysis.h"
#include "MuonPedAlg.h"
#include "src/lib/Util/ROOTUtil.h"
#include "src/lib/Util/CGCUtil.h"

// GLAST INCLUDES
#include "digiRootData/Gem.h"
#include "digiRootData/DigiEvent.h"
#include "CalUtil/CalVec.h"
#include "CalUtil/SimpleCalCalib/CalPed.h"
#include "enums/GemConditionSummary.h"

// EXTLIB INCLUDES
#include "TH1S.h"
#include "TF1.h"
#include "TStyle.h"

// STD INCLUDES
#include <sstream>
#include <cmath>

namespace calibGenCAL {

  using namespace CalUtil;
  using namespace std;

  void MuonPedAlg::initHists() {
    for (RngIdx rngIdx; rngIdx.isValid(); rngIdx++)
      if (m_histograms[rngIdx] == 0) {
        const string histname = genHistName(rngIdx);

        m_histograms[rngIdx] = new TH1S(histname.c_str(),
                                        histname.c_str(),
                                        1000, 0.5, 1000.5);
      }
  }

  void MuonPedAlg::fillHists(const unsigned nEntries,
                             const vector<string> &rootFileList,
                             const CalPed *roughPeds,
                             TRIGGER_CUT trigCut) {
    /////////////////////////////////////////
    /// Initialize Object Data //////////////
    /////////////////////////////////////////
    initHists();

    algData.roughPeds = roughPeds;
    algData.trigCut   = trigCut;

    /////////////////////////////////////////
    /// Open ROOT Event File  ///////////////
    /////////////////////////////////////////
    RootFileAnalysis rootFile(0,
                              &rootFileList,
                              0);

    // enable only needed branches in root file
    rootFile.getDigiChain()->SetBranchStatus("*", 0);
    rootFile.getDigiChain()->SetBranchStatus("m_calDigiCloneCol");
    if (algData.trigCut == PERIODIC_TRIGGER)
      rootFile.getDigiChain()->SetBranchStatus("m_summary");
    if (algData.trigCut == PERIODIC_TRIGGER || algData.trigCut == EXTERNAL_TRIGGER)
      rootFile.getDigiChain()->SetBranchStatus("m_gem");

    unsigned nEvents = rootFile.getEntries();
    LogStrm::get() << __FILE__ << ": Processing: " << nEvents << " events." << endl;

    /////////////////////////////////////////
    /// Event Loop //////////////////////////
    /////////////////////////////////////////
    // in periodic trigger mode we will skip these events
    for (eventData.eventNum = 0; eventData.eventNum < nEvents; eventData.next()) {
      /////////////////////////////////////////
      /// Load new event //////////////////////
      /////////////////////////////////////////
      if (eventData.eventNum % 2000 == 0) {
        // quit if we have enough entries in each histogram
        const unsigned currentMin = getMinEntries();
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
        LogStrm::get() << __FILE__ << ": Unable to read DigiEvent: " << eventData.eventNum  << endl;
        continue;
      }

      processEvent(*digiEvent);
    }
  }

  void MuonPedAlg::fitHists(CalPed &calPed) {
    for (RngIdx rngIdx; rngIdx.isValid(); rngIdx++) {
      // skip absent histograms
      if (!m_histograms[rngIdx])
        continue;

      // select histogram from list
      TH1S &h = *m_histograms[rngIdx];

      // skip empty histograms
      if (h.GetEntries() == 0)
        continue;

      // trim outliers
      float av = h.GetMean(); float err = h.GetRMS();
      for ( unsigned short iter = 0; iter < 3; iter++ ) {
        h.SetAxisRange(av-3*err, av+3*err);
        av = h.GetMean(); err = h.GetRMS();
      }

      // gaussian fit
      h.Fit("gaus", "Q", "", av-3*err, av+3*err );
      h.SetAxisRange(av-150, av+150);

      // assign values to permanent arrays
      calPed.setPed(rngIdx,
                    ((TF1&)*h.GetFunction("gaus")).GetParameter(1));
      calPed.setPedSig(rngIdx,
                       ((TF1&)*h.GetFunction("gaus")).GetParameter(2));
    }
  }

  void MuonPedAlg::loadHists(const TDirectory &readDir) {
    for (RngIdx rngIdx; rngIdx.isValid(); rngIdx++) {
      const string histname = genHistName(rngIdx);

      TH1S  *hist_ptr = retrieveROOTObj < TH1S > (readDir, histname);
      if (!hist_ptr)
        continue;

      m_histograms[rngIdx] = hist_ptr;
    }
  }

  string MuonPedAlg::genHistName(RngIdx rngIdx) {
    ostringstream tmp;

    tmp << "muonpeds_" << rngIdx.toStr();
    return tmp.str();
  }

  unsigned MuonPedAlg::getMinEntries() {
    unsigned retVal = ULONG_MAX;

    for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
      const RngIdx rngIdx(xtalIdx,
                    POS_FACE,
                    LEX8);

      const unsigned nEntries = (unsigned)m_histograms[rngIdx]->GetEntries();

      // only count histograms that have been filled
      // (some histograms will never be filled if we are
      // not using all 16 towers)
      if (nEntries != 0)
        retVal = min(retVal, nEntries);
    }

    // case where there are no fills at all
    if (retVal == ULONG_MAX)
      return 0;

    return retVal;
  }

  void MuonPedAlg::processEvent(const DigiEvent &digiEvent) {
    /////////////////////////////////////////
    /// Event/Trigger level cuts ////////////
    /////////////////////////////////////////

    //-- retrieve trigger data
    const Gem *gem = 0;
    unsigned   gemConditionsWord = 0;


    if (algData.trigCut == PERIODIC_TRIGGER ||
        algData.trigCut == EXTERNAL_TRIGGER) {
      gem = &(digiEvent.getGem());
      gemConditionsWord = gem->getConditionSummary();
    }

    //-- PERIODIC_TRIGGER CUT
    if (algData.trigCut == PERIODIC_TRIGGER) {
      // quick check if we are in 4-range mode
      const EventSummaryData &summary = digiEvent.getEventSummaryData();
      if (&summary == 0) {
        LogStrm::get() << "Warning, eventSummary data not found for event: "
                         << eventData.eventNum << endl;
        return;
      }
      eventData.fourRange = const_cast<EventSummaryData&>(summary).readout4();

      const float gemDeltaEventTime = gem->getDeltaEventTime()*0.05;
      if (gemConditionsWord != enums::PERIODIC ||     // skip unless we are periodic trigger only
          eventData.prev4Range      ||   // avoid bias from 4 range readout in prev event
          gemDeltaEventTime < 100)      // avoid bias from shaped readout noise from adjacent event
        return;
    }

    //-- EXTERNAL_TRIGGER CUT
    if (algData.trigCut == EXTERNAL_TRIGGER) // cut on external trigger only
      if (gemConditionsWord != enums::EXTERNAL)
        return;

    const TClonesArray *calDigiCol = digiEvent.getCalDigiCol();
    if (!calDigiCol) {
      LogStrm::get() << "no calDigiCol found for event#" << eventData.eventNum << endl;
      return;
    }

    TIter calDigiIter(calDigiCol);

    const CalDigi      *pCalDigi = 0;

    /////////////////////////////////////////
    /// Xtal Hit Loop ///////////////////////
    /////////////////////////////////////////
    while ((pCalDigi = dynamic_cast<CalDigi *>(calDigiIter.Next())))
      processHit(*pCalDigi);
  }

  void MuonPedAlg::processHit(const CalDigi &calDigi) {
    //-- XtalId --//
    const idents::CalXtalId id(calDigi.getPackedId());  // get interaction information

    // skip hits not for current tower.
    const XtalIdx xtalIdx(id);

    const unsigned nRO = calDigi.getNumReadouts();

    if (nRO != 4) {
      ostringstream tmp;
      tmp << __FILE__  << ":"     << __LINE__ << " "
          << "Event# " << eventData.eventNum << " Invalid nReadouts, expecting 4";
      throw runtime_error(tmp.str());
    }

    // 1st look at LEX8 vals
    CalVec<FaceNum, float> adcL8;
    for (FaceNum face; face.isValid(); face++) {
      adcL8[face] = calDigi.getAdcSelectedRange(LEX8.val(), (CalXtalId::XtalFace)face.val());
      // check for missing readout
      if (adcL8[face] < 0) {
        LogStrm::get() << "Couldn't get LEX8 readout for event=" << eventData.eventNum << endl;
        return;
      }
    }

    /////////////////////////////////////////
    /// 'Rough' pedestal mode (fist pass) ///
    /////////////////////////////////////////
    if (!algData.roughPeds)
      for (FaceNum face; face.isValid(); face++) {
        const RngIdx rngIdx(xtalIdx,
                      face,
                      LEX8);

        m_histograms[rngIdx]->Fill(adcL8[face]);
      }
    else  {
      /////////////////////////////////////////
      /// Cut outliers (2nd pass) /////////////
      /////////////////////////////////////////

      // skip outliers (outside of 5 sigma on either side)
      if (fabs(adcL8[NEG_FACE] - algData.roughPeds->getPed(RngIdx(xtalIdx, NEG_FACE, LEX8))) >=
          5*algData.roughPeds->getPedSig(RngIdx(xtalIdx, NEG_FACE, LEX8)) ||
          fabs(adcL8[POS_FACE] - algData.roughPeds->getPed(RngIdx(xtalIdx, POS_FACE, LEX8))) >=
          5*algData.roughPeds->getPedSig(RngIdx(xtalIdx, POS_FACE, LEX8)))
        return;

      //-- Fill histograms for all 4 ranges
      for (unsigned short n = 0; n < nRO; n++) {
        const CalXtalReadout &readout = *calDigi.getXtalReadout(n);

        for (FaceNum face; face.isValid(); face++) {
          // check that we are in the expected readout mode
          const RngNum rng(readout.getRange((CalXtalId::XtalFace)face.val()));
          const unsigned short adc(readout.getAdc((CalXtalId::XtalFace)face.val()));
          const RngIdx rngIdx(xtalIdx,
                        face,
                        rng);

          m_histograms[rngIdx]->Fill(adc);
        }
      }
    }
  }

  void MuonPedAlg::trimHists() {
    for (RngIdx rngIdx; rngIdx.isValid(); rngIdx++)
      if (m_histograms[rngIdx])
        if (!m_histograms[rngIdx]->GetEntries()) {
          delete m_histograms[rngIdx];
          m_histograms[rngIdx] = 0;
        }
  }

}; // namespace calibGenCAL
