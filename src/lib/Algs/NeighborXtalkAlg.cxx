// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Algs/NeighborXtalkAlg.cxx,v 1.5 2007/06/13 22:42:12 fewtrell Exp $

/** @file
    @author fewtrell
*/

// LOCAL INCLUDES
#include "NeighborXtalkAlg.h"
#include "../Util/RootFileAnalysis.h"
#include "../CalibDataTypes/NeighborXtalk.h"
#include "../CalibDataTypes/CalPed.h"
#include "../Specs/singlex16.h"
#include "../Util/SplineUtil.h"
#include "../Util/CGCUtil.h"

// GLAST INCLUDES
#include "CalUtil/CalVec.h"
#include "digiRootData/DigiEvent.h"

// EXTLIB INCLUDES
#include "TH1.h"

// STD INCLUDES
#include <sstream>
#include <cstdlib>

namespace calibGenCAL {

  using namespace std;
  using namespace CalUtil;
  using namespace singlex16;
  using namespace SplineUtil;

  NeighborXtalkAlg::NeighborXtalkAlg()
  {
  }

  void NeighborXtalkAlg::AlgData::initHists() {
    adcHists = new TObjArray(DiodeIdx::N_VALS);

    // delete histograms w/ TObjArray, do not save in file...
    adcHists->SetOwner(1);
    for (DiodeIdx diodeIdx; diodeIdx.isValid(); diodeIdx++) {
      {
        ostringstream tmp;
        tmp << "adc" << diodeIdx.val();
        (*adcHists)[diodeIdx.val()] = new TH1S(tmp.str().c_str(),
                                               tmp.str().c_str(),
                                               MAX_DAC+1, -0.5, MAX_DAC+.5);
      }
    }
  }

  void NeighborXtalkAlg::readRootData(const string &rootFileName,
                                      NeighborXtalk &xtalk) {
    algData.xtalk  = &xtalk;

    // open input root file.
    vector<string> rootFileList;
    rootFileList.push_back(rootFileName);
    RootFileAnalysis rootFile(0,
                              &rootFileList,
                              0);

    // enable only needed branches in root file
    rootFile.getDigiChain()->SetBranchStatus("*", 0);
    rootFile.getDigiChain()->SetBranchStatus("m_calDigiCloneCol");

    //-- CHECK N EVENTS --//
    const unsigned nEvents    = rootFile.getEntries();
    const unsigned nEventsReq = TOTAL_PULSES_COLWISE;

    // 10 % margin is overly cautious.  occasionally there are a 1 or 2 'extra' events.
    if (abs((long)(nEvents - nEventsReq)) > .10*nEventsReq) {
      ostringstream tmp;

      tmp << __FILE__ << ":" << __LINE__ << " "
          << "Wrong # of events:"
          << " nEventsRequired=" << nEventsReq
          << " nEventsFound=" << nEvents;

      throw runtime_error(tmp.str());
    }

    LogStrm::get() << __FILE__ << ": Processing: " << nEvents << " events." << endl;

    // BEGINNING OF EVENT LOOP
    for (eventData.eventNum = 0 ; eventData.eventNum < nEvents; eventData.eventNum++) {
      if (eventData.eventNum%1000 == 0) {
        LogStrm::get() << " event " << eventData.eventNum << '\n';
        LogStrm::get().flush();
      }

      rootFile.getEvent(eventData.eventNum);

      const DigiEvent *digiEvent = rootFile.getDigiEvent();
      if (!digiEvent) {
        ostringstream tmp;
        tmp << __FILE__ << ":" << __LINE__ << " "
            << "No DigiEvent found event #" << eventData.eventNum;
      }

      processEvent(*digiEvent);
    }  // end analysis code in event loop
  }

  void NeighborXtalkAlg::processEvent(const DigiEvent &digiEvent) {
    const TClonesArray *calDigiCol = digiEvent.getCalDigiCol();


    if (!calDigiCol) {
      ostringstream tmp;
      tmp << __FILE__ << ":" << __LINE__ << " "
          << "Empty calDigiCol event #" << eventData.eventNum;
      throw runtime_error(tmp.str());
    }

    // -- Determine test config for this event -- //
    //    note: only count good events            //

    // which column are we testing?
    eventData.testCol = eventData.iGoodEvt/N_PULSES_PER_XTAL;
    // which DAC setting are we on?
    eventData.testDAC = (eventData.iGoodEvt%N_PULSES_PER_XTAL)/N_PULSES_PER_DAC;
    // how many samples for current DAC setting?
    eventData.iSamp   = (eventData.iGoodEvt%N_PULSES_PER_XTAL)%N_PULSES_PER_DAC;

    const unsigned nDigis = calDigiCol->GetEntries();
    // event should have 1 hit for every xtal in each tower
    // we support any nTowers
    if (nDigis > 0 && nDigis%tXtalIdx::N_VALS == 0) {
      eventData.iGoodEvt++;

      //loop through each 'hit' in one event
      TIter calDigiIter(calDigiCol);

      const CalDigi *pdig = 0;
      while ((pdig = dynamic_cast<CalDigi *>(calDigiIter.Next()))) {
        const CalDigi &cdig = *pdig;    // shorthand.
        processHit(cdig);
      }                                 // foreach xtal
    }
    else
      LogStrm::get() << " event " << eventData.eventNum << " contains "
                       << nDigis << " digis - skipped" << endl;
  }

  void NeighborXtalkAlg::processHit(const CalDigi &cdig) {
    const CalXtalId      id(cdig.getPackedId()); // get interaction information

    const ColNum         col(id.getColumn());
    // only process collumns neighboring the current column.
    const unsigned short colDiff(abs(col.val() - eventData.testCol.val()));


    if (colDiff != 1)
      return;
    const TwrNum         twr(id.getTower());
    const LyrNum         lyr(id.getLayer());

    // Loop through each readout on current xtal
    const unsigned short numRo   = cdig.getNumReadouts();
    for (unsigned short iRo = 0; iRo < numRo; iRo++) {
      const CalXtalReadout &acRo = *(cdig.getXtalReadout(iRo));

      for (FaceNum face; face.isValid(); face++) {
        const RngNum   rng(acRo.getRange ((CalXtalId::XtalFace)face.val()));

        // only interested in current diode!
        const DiodeNum diode(rng.getDiode());

        // only processing X8 data
        if (rng.getTHX() != THX8) continue;

        // retrieve adc value
        const unsigned short adc = acRo.getAdc((CalXtalId::XtalFace)face.val());

        // fill histogram
        const DiodeIdx diodeIdx(twr,
                          lyr,
                          col,
                          face,
                          diode);

        TH1S & h = *(dynamic_cast<TH1S *>(algData.adcHists->At(diodeIdx.val())));

        // reset histogram if we're starting a new DAC setting
        if (eventData.iSamp == 0) {
          h.Reset();
          h.SetAxisRange(-0.5, MAX_DAC+.5);
        }

        // fill histogram
        h.Fill(adc);

        // save histogram data if we're on last sample for current
        // dac settigns
        if (eventData.iSamp == (N_PULSES_PER_DAC - 1)) {
          float av, err;
          // trim outliers
          av  = h.GetMean();
          err = h.GetRMS();
          for ( unsigned short iter = 0; iter < 3; iter++ ) {
            h.SetAxisRange(av-3*err, av+3*err);
            av  = h.GetMean();
            err = h.GetRMS();
          }

          // assign to table
          // 'source' channel is current injected channel / LEX8
          const DiodeIdx srcIdx(twr,
                          lyr,
                          eventData.testCol,
                          face,
                          LRG_DIODE);

          algData.xtalk->setPoint(diodeIdx,
                                  srcIdx,
                                  CIDAC_TEST_VALS[eventData.testDAC],
                                  av);
        }
      }
    }
  }
}; // namespace calibGenCAL
