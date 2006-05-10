// LOCAL INLCUDES
#include "CfRoot.h"

// GLAST INCLUDES

// EXTLIB INCLUDES

// STD INCLUDES
#include <sstream>

CfRoot::CfRoot(const vector<string> &digiFileNames, 
               CfData  &data, 
               CfCfg &cfg,
               DiodeNum diode) :
  RootFileAnalysis(vector<string>(0), digiFileNames, vector<string>(0)),
  m_curDiode(diode),
  m_cfData(data),
  m_cfg(cfg),
  m_evtId(0),
  m_nEvtMax(0),
  m_iGoodEvt(0),
  m_ciHists(tRngIdx::N_VALS)
{
  createHists();
}

CfRoot::~CfRoot() {
  // delete internal histograms.
  m_ciHists.Delete();
}

void CfRoot::createHists() {
  for (tRngIdx rngIdx; rngIdx.isValid(); rngIdx++) {
    ostringstream tmp;
    tmp << "ciHist_" << rngIdx;

    // delete histograms w/ TObjArray, do not save in file...
    m_ciHists.SetOwner(1);

    m_ciHists[rngIdx.val()] =  new TH1S(tmp.str().c_str(),
                                           tmp.str().c_str(),
                                           4100,0,4100);
  }

}

bool  CfRoot::isRngEnabled(RngNum rng) {
  if (rng.getDiode() == m_curDiode) return true;
  return false;
}

void CfRoot::ProcessEvt() {

  // -- Determine test config for this event -- //
  //    note: only count good events            //

  // which column are we testing?
  ColNum testCol;
  if (!m_cfg.broadcastMode)
    testCol = m_iGoodEvt/m_cfg.nPulsesPerXtal;
  // which DAC setting are we on?
  int testDAC = (m_iGoodEvt%m_cfg.nPulsesPerXtal)/m_cfg.nPulsesPerDAC;
  // how many samples for current DAC setting?
  int iSamp   = (m_iGoodEvt%m_cfg.nPulsesPerXtal)%m_cfg.nPulsesPerDAC;

  const TClonesArray* calDigiCol = m_digiEvt->getCalDigiCol();
  if (!calDigiCol) {
    ostringstream tmp;
    tmp << __FILE__ << ":" << __LINE__ << " " 
        << "Empty calDigiCol event #" << m_evtId;
    throw tmp.str();
  }
  TIter calDigiIter(calDigiCol);

  // Loop through each xtal interaction
  CalDigi *pdig = 0;

  int nDigis = calDigiCol->GetEntries();
  // event should have 1 hit for every xtal in each tower
  // we support any nTowers
  if(nDigis > 0 && nDigis%tXtalIdx::N_VALS == 0){
    m_iGoodEvt++;

    //loop through each 'hit' in one event
    while ((pdig = (CalDigi*)calDigiIter.Next())) {  
      CalDigi &cdig = *pdig; // use ref to reduce '->'

      CalXtalId id = cdig.getPackedId();  // get interaction information

      // skip if not for current column
      ColNum col = id.getColumn();
      if (col != testCol && !m_cfg.broadcastMode) 
        continue;

      // skip if not for current tower
      TwrNum twr = id.getTower();
      if ((int)twr != m_cfg.twrBay) continue;

      LyrNum lyr = id.getLayer();

      // Loop through each readout on current xtal
      int numRo = cdig.getNumReadouts();
      for (int iRo=0; iRo<numRo; iRo++){
        const CalXtalReadout &acRo = *(cdig.getXtalReadout(iRo));
        for (FaceNum face; face.isValid(); face++) {
          RngNum rng = acRo.getRange((CalXtalId::XtalFace)(unsigned short)face);
          // only interested in current diode!
          if (!isRngEnabled(rng)) continue;

          // retrieve adc value
          int adc = acRo.getAdc((CalXtalId::XtalFace)(unsigned short)face);

          // fill histogram
          tRngIdx rngIdx(lyr,col,face,rng);
          TH1S& h = *((TH1S*)m_ciHists[rngIdx.val()]);

          // reset histogram if we're starting a new DAC setting
          if(iSamp == 0){
            h.Reset();
            h.SetAxisRange(0,4100);
          }

          h.Fill(adc);
          // oh yeah, also fill up profile histograms
          if (m_cfg.genHistfile)
            m_cfData.m_ciRawProfs[rngIdx]->Fill(testDAC, adc);

          // save histogram data if we're on last sample for current
          // dac settigns
          if(iSamp == (m_cfg.nPulsesPerDAC - 1)){
            float av,err;
            // trim outliers
            av = h.GetMean();
            err =h.GetRMS();
            for( int iter=0; iter<3;iter++ ) {
              h.SetAxisRange(av-3*err,av+3*err);
              av  = h.GetMean(); 
              err = h.GetRMS();
            }
            // assign to table
            m_cfData.m_adcMean[rngIdx][testDAC] = av;
          }
        } // foreach face
      } // foreach readout
    } // foreach xtal
  }
  else {
    m_cfg.ostrm << " event " << m_evtId << " contains " 
                << nDigis << " digis - skipped" << endl;
    m_nEvtMax++;
  }
}

void CfRoot::EvtLoop(Int_t nEvtAsked)
{
  // Purpose and Method:  Event Loop

  //
  //  COMMENT OUT ANY BRANCHES YOU ARE NOT INTERESTED IN.
  //
  if (m_digiEnabled) {
    m_digiChain.SetBranchStatus("*",0);  // disable all branches
    // activate desired brances
    m_digiChain.SetBranchStatus("m_cal*",1);
    m_digiChain.SetBranchStatus("m_eventId", 1);
  }

  //
  // DO WE HAVE ENOUGH EVENTS IN FILE?
  //
  Int_t nEvts = getEntries();
  m_cfg.ostrm << "\nNum Events in File is: " << nEvts << endl;
  Int_t curI=0;
  m_nEvtMax = min(nEvtAsked+m_startEvt,nEvts);

  if (nEvtAsked+m_startEvt >  nEvts) {
    ostringstream tmp;
    tmp << __FILE__ << ":" << __LINE__ << " " 
        << "not enough entries in file to proceed, we need " << nEvtAsked;
    throw tmp.str();
  }

  // BEGINNING OF EVENT LOOP
  for (Int_t iEvt=m_startEvt; iEvt<m_nEvtMax; iEvt++, curI=iEvt) {
    if (m_digiEvt) m_digiEvt->Clear();

    getEvent(iEvt);
    // Digi ONLY analysis
    if (m_digiEvt) {
      m_evtId = m_digiEvt->getEventId();
      if(m_evtId%1000 == 0) {
        m_cfg.ostrm << " event " << m_evtId << '\r';
        m_cfg.ostrm.flush();
      }

      ProcessEvt();
    }
  }  // end analysis code in event loop

  m_startEvt = curI;
}

  
  
