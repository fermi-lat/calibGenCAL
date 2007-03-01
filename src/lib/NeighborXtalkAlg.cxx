// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/NeighborXtalkAlg.cxx,v 1.3 2007/02/28 21:03:07 fewtrell Exp $

/** @file
    @author fewtrell
 */

// LOCAL INCLUDES
#include "NeighborXtalkAlg.h"
#include "RootFileAnalysis.h"
#include "NeighborXtalk.h"
#include "CalPed.h"
#include "singlex16.h"
#include "CGCUtil.h"
#include "SplineUtil.h"

// GLAST INCLUDES
#include "CalUtil/CalVec.h"
#include "digiRootData/DigiEvent.h"

// EXTLIB INCLUDES

// STD INCLUDES
#include <sstream>
#include <cstdlib>

using namespace std;
using namespace CalUtil;
using namespace singlex16;
using namespace CGCUtil;
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
                                             4096, -0.5, 4095.5);
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
  unsigned nEvents    = rootFile.getEntries();
  unsigned nEventsReq = TOTAL_PULSES_COLWISE;

  // 10 % margin is overly cautious.  occasionally there are a 1 or 2 'extra' events.
  if (abs((long)(nEvents - nEventsReq)) > .10*nEventsReq) {
    ostringstream tmp;

    tmp << __FILE__ << ":" << __LINE__ << " "
        << "Wrong # of events:"
        << " nEventsRequired=" << nEventsReq
        << " nEventsFound=" << nEvents;

    throw runtime_error(tmp.str());
  }

  LogStream::get() << __FILE__ << ": Processing: " << nEvents << " events." << endl;

  // BEGINNING OF EVENT LOOP
  for (eventData.eventNum = 0 ; eventData.eventNum < nEvents; eventData.eventNum++) {
    if (eventData.eventNum%1000 == 0) {
      LogStream::get() << " event " << eventData.eventNum << '\n';
      LogStream::get().flush();
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

  unsigned nDigis = calDigiCol->GetEntries();
  // event should have 1 hit for every xtal in each tower
  // we support any nTowers
  if (nDigis > 0 && nDigis%tXtalIdx::N_VALS == 0) {
    eventData.iGoodEvt++;

    //loop through each 'hit' in one event
    TIter calDigiIter(calDigiCol);

    const CalDigi *pdig = 0;
    while ((pdig = (CalDigi *)calDigiIter.Next())) {
      const CalDigi &cdig = *pdig;    // shorthand.
      processHit(cdig);
    }                                 // foreach xtal
  }
  else
    LogStream::get() << " event " << eventData.eventNum << " contains "
                     << nDigis << " digis - skipped" << endl;
}

void NeighborXtalkAlg::processHit(const CalDigi &cdig) {
  CalXtalId      id      = cdig.getPackedId(); // get interaction information

  ColNum         col     = id.getColumn();
  // only process collumns neighboring the current column.
  unsigned short colDiff = abs((unsigned short)col - (unsigned short)eventData.testCol);


  if (colDiff != 1)
    return;
  TwrNum         twr     = id.getTower();
  LyrNum         lyr     = id.getLayer();

  // Loop through each readout on current xtal
  unsigned short numRo   = cdig.getNumReadouts();
  for (unsigned short iRo = 0; iRo < numRo; iRo++) {
    const CalXtalReadout &acRo = *(cdig.getXtalReadout(iRo));

    for (FaceNum face; face.isValid(); face++) {
      RngNum   rng(acRo.getRange ((CalXtalId::XtalFace)face.val()));

      // only interested in current diode!
      DiodeNum diode(rng.getDiode());

      // only processing X8 data
      if (rng.getTHX() != THX8) continue;

      // retrieve adc value
      unsigned short adc = acRo.getAdc((CalXtalId::XtalFace)face.val());

      // fill histogram
      DiodeIdx diodeIdx(twr,
                        lyr,
                        col,
                        face,
                        diode);

      TH1S & h = *((TH1S *)algData.adcHists->At(diodeIdx.val()));

      // reset histogram if we're starting a new DAC setting
      if (eventData.iSamp == 0) {
        h.Reset();
        h.SetAxisRange(-0.5, 4095.5);
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
        RngIdx srcIdx(twr,
                      lyr,
                      eventData.testCol,
                      face,
                      LEX8);

        // 'dest' channel is currently measured channel
        RngIdx destIdx(twr,
                       lyr,
                       col,
                       face,
                       rng);

        algData.xtalk->setPoint(destIdx,
                                srcIdx,
                                CIDAC_TEST_VALS[eventData.testDAC],
                                av);
      }
    }
  }
}
