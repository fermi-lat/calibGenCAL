// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/NeighborXtalk.cxx,v 1.6 2007/01/05 17:25:34 fewtrell Exp $

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
#include "TProfile.h"

// STD INCLUDES
#include <sstream>

using namespace std;
using namespace CalUtil;
using namespace singlex16;
using namespace CGCUtil;
using namespace SplineUtil;

NeighborXtalkAlg::NeighborXtalkAlg()
{
}

void NeighborXtalkAlg::AlgData::initHists() {
  adcHists = new TObjArray(RngIdx::N_VALS);

  // delete histograms w/ TObjArray, do not save in file...
  adcHists->SetOwner(1);
  for (RngIdx rngIdx; rngIdx.isValid(); rngIdx++) {
    {
      ostringstream tmp;
      tmp << "adc" << rngIdx.val();
      (*adcHists)[rngIdx.val()] = new TH1S(tmp.str().c_str(),
                                           tmp.str().c_str(),
                                           4096, -0.5, 4095.5);
    }

    {
      ostringstream tmp;
      tmp << "ciRaw_" << rngIdx.val();
      profiles[rngIdx] = new TProfile(tmp.str().c_str(),
                                      tmp.str().c_str(),
                                      N_CIDAC_VALS,
                                      -0.5,
                                      N_CIDAC_VALS-0.5);
    }
  }
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
}  

void NeighborXtalkAlg::readRootData(const string &rootFileName,
                                    NeighborXtalk &xtalk,
                                    DiodeNum diode) {
  algData.diode     = diode;
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

  unsigned nEvents = rootFile.getEntries();
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


  
