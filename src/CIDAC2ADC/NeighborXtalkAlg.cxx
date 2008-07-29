// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/CIDAC2ADC/NeighborXtalkAlg.cxx,v 1.3 2008/06/09 21:05:33 fewtrell Exp $

/** @file
    @author fewtrell
*/

// LOCAL INCLUDES
#include "NeighborXtalkAlg.h"
#include "src/lib/Util/RootFileAnalysis.h"
#include "src/lib/Specs/singlex16.h"
#include "src/lib/Util/CGCUtil.h"

// GLAST INCLUDES
#include "CalUtil/SimpleCalCalib/SplineUtil.h"
#include "CalUtil/CalVec.h"
#include "digiRootData/DigiEvent.h"
#include "CalUtil/SimpleCalCalib/NeighborXtalk.h"
#include "CalUtil/SimpleCalCalib/CalPed.h"

// EXTLIB INCLUDES
#include "TH1.h"

// STD INCLUDES
#include <sstream>

namespace {

  using namespace calibGenCAL;

  /// spec alternate LCI loop scheme used in run #077015240
  class  alt_singlex16 {
  public:

    alt_singlex16(const singlex16 &sx16) {
      
      CIDAC_GROUP_LEN[0] = 33;
      CIDAC_GROUP_LEN[1] = 28;
      CIDAC_GROUP_LEN[2] = 112;
      
      // starting index of each group
      CIDAC_GROUP_START[0]=0;
      for(unsigned short i=1;i<N_CIDAC_GROUPS;i++)
        CIDAC_GROUP_START[i]=CIDAC_GROUP_START[i-1]+CIDAC_GROUP_LEN[i-1];

      // number of pulses per column
      for(unsigned short i=0;i<N_CIDAC_GROUPS;i++)
        N_PULSES_PER_COL[i] = sx16.nPulsesPerDAC*CIDAC_GROUP_LEN[i];
      
      // number of pulses per group
      for(unsigned short i=0;i<N_CIDAC_GROUPS;i++)
        N_PULSES_PER_GROUP[i]= N_PULSES_PER_COL[i]*CalUtil::ColNum::N_VALS;
      

      //first pulse in group
      GROUP_FIRST_PULSE[0]=0;
      for(unsigned short i=1;i<N_CIDAC_GROUPS;i++)
        GROUP_FIRST_PULSE[i] = GROUP_FIRST_PULSE[i-1] + N_PULSES_PER_GROUP[i-1];

      //last pulse in group
      for(unsigned short i=1;i<N_CIDAC_GROUPS;i++)
        GROUP_LAST_PULSE[i]=GROUP_FIRST_PULSE[i]+N_PULSES_PER_GROUP[i]-1;
    }

    /// number of groups of CI DAC values with the same stepsize
    static const unsigned short N_CIDAC_GROUPS = 3;
    
    /// number of CI DAC values in each group
    unsigned short CIDAC_GROUP_LEN[N_CIDAC_GROUPS];

    /// starting index of each group
    unsigned CIDAC_GROUP_START[N_CIDAC_GROUPS];

    /// number of pulses per column
    unsigned N_PULSES_PER_COL[N_CIDAC_GROUPS];

    /// number of pulses per group
    unsigned N_PULSES_PER_GROUP[N_CIDAC_GROUPS];
    
    ///first pulse in a group
    unsigned GROUP_FIRST_PULSE[N_CIDAC_GROUPS];
    
    ///last pulse in a group
    unsigned GROUP_LAST_PULSE[N_CIDAC_GROUPS];
  };
  
}

namespace calibGenCAL {

  using namespace std;
  using namespace CalUtil;
  using namespace SplineUtil;

  void NeighborXtalkAlg::EventData::nextEvent() {
    iGoodEvt++;

    /// standard loop scheme
    if (!m_altLoopScheme) { 
      // which column are we testing?
      testCol = iGoodEvt/m_singlex16.nPulsesPerXtal();
      // which DAC setting are we on?
      testDAC = (iGoodEvt%m_singlex16.nPulsesPerXtal())/m_singlex16.nPulsesPerDAC;
      // how many samples for current DAC setting?
      iSamp   = (iGoodEvt%m_singlex16.nPulsesPerXtal())%m_singlex16.nPulsesPerDAC;
    } 

    /// alternate loop scheme
    else {
      static alt_singlex16 asx(m_singlex16);

      //in what group of dac settigs are we?
      unsigned short iGroup = 0;
      for(unsigned int igr=0;igr<asx.N_CIDAC_GROUPS;igr++)
        if (iGoodEvt>=asx.GROUP_FIRST_PULSE[igr] && iGoodEvt<=asx.GROUP_LAST_PULSE[igr])
          iGroup=igr;

      // which column are we testing?
      testCol = (iGoodEvt-asx.GROUP_FIRST_PULSE[iGroup])/asx.N_PULSES_PER_COL[iGroup];

      // which DAC setting are we on?
      testDAC = asx.CIDAC_GROUP_START[iGroup]+
        ((iGoodEvt-asx.GROUP_FIRST_PULSE[iGroup])%asx.N_PULSES_PER_COL[iGroup])/m_singlex16.nPulsesPerDAC;

      // how many samples for current DAC setting?
      iSamp = ((iGoodEvt-asx.GROUP_FIRST_PULSE[iGroup])%asx.N_PULSES_PER_COL[iGroup])%m_singlex16.nPulsesPerDAC;

    }
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
                                               singlex16::MAX_DAC+1, -0.5, singlex16::MAX_DAC+.5);
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
    const unsigned nEventsReq = m_singlex16.totalPulsesCOLWISE();

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

      DigiEvent const*const digiEvent = rootFile.getDigiEvent();
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

    const unsigned nDigis = calDigiCol->GetEntries();
    // event should have 1 hit for every xtal in each tower
    // we support any nTowers
    if (nDigis > 0 && nDigis%tXtalIdx::N_VALS == 0) {
      //loop through each 'hit' in one event
      TIter calDigiIter(calDigiCol);

      const CalDigi *pdig = 0;
      while ((pdig = dynamic_cast<CalDigi *>(calDigiIter.Next()))) {
        const CalDigi &cdig = *pdig;    // shorthand.
        processHit(cdig);
      }                                 // foreach xtal

      eventData.nextEvent();
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
          h.SetAxisRange(-0.5, m_singlex16.MAX_DAC+.5);
        }

        // fill histogram
        h.Fill(adc);

        // save histogram data if we're on last sample for current
        // dac settigns
        if (eventData.iSamp == (m_singlex16.nPulsesPerDAC - 1)) {
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
                                  m_singlex16.CIDACTestVals()[eventData.testDAC],
                                  av);
        }
      }
    }
  }
}; // namespace calibGenCAL
