// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/CIDAC2ADC/IntNonlinAlg.cxx,v 1.11 2008/07/21 17:59:40 fewtrell Exp $

/** @file
    @author fewtrell
*/

// LOCAL INCLUDES
#include "IntNonlinAlg.h"
#include "src/lib/Specs/singlex16.h"
#include "src/lib/Util/RootFileAnalysis.h"
#include "src/lib/Util/CGCUtil.h"
#include "src/lib/Util/string_util.h"

// GLAST INCLUDES
#include "CalUtil/CalVec.h"
#include "digiRootData/DigiEvent.h"
#include "digiRootData/Configuration.h"
#include "digiRootData/MetaEvent.h"
#include "CalUtil/SimpleCalCalib/CIDAC2ADC.h"
#include "CalUtil/SimpleCalCalib/CalPed.h"

// EXTLIB INCLUDES
#include "TH1S.h"
#include "TProfile.h"
#include "TNtuple.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TGraph.h"

// STD INCLUDES
#include <sstream>
#include <cmath>


namespace {
  /// warning limit on RMS per ADC range
  static const float rmsWarnLimit[CalUtil::RngIdx::N_VALS] = {10,
                                                              3,
                                                              10,
                                                              3};
};

namespace calibGenCAL {

  using namespace CalUtil;
  using namespace std;

  IntNonlinAlg::IntNonlinAlg(const singlex16 &sx16,
                             const bool hugeTuple) :
    m_singlex16(sx16),
    m_fitResults(0),
    m_hugeTuple(0) 
  {
    
    // init fit results tuple
    m_fitResults = new TNtuple("inl_fit_results",
                               "intNonlin_fit_results",
                               "twr:lyr:col:face:rng:dac_idx:dac_val:raw_adcmean:raw_adcrms:final_adcmean:final_adcrms");

    /// initialize optional tuple w/ all readouts
    if (hugeTuple) {
      m_hugeTuple = new TTree("inl_readout_tuple",
                              "inl_readout_tuple");
      //-- Add Branches to tree --//
      if (!m_hugeTuple->Branch("rngIdx",         &m_tupleData.rngIdx,         "rngIdx/s")        ||
          !m_hugeTuple->Branch("goodEventNum",   &m_tupleData.goodEvt,        "goodEventNum/s")  ||
          !m_hugeTuple->Branch("cidac",          &m_tupleData.cidac,          "cidac/s")         ||
          !m_hugeTuple->Branch("adc",            &m_tupleData.adc,           "adc/s")) {
        LogStrm::get() << "ERROR: Couldn't initialize tuple." << endl;
      }
    }
  }

  
  void IntNonlinAlg::AlgData::initHists() {
    adcHists = new TObjArray(RngIdx::N_VALS);

    // delete histograms w/ TObjArray, do not save in file...
    adcHists->SetOwner(1);
    for (RngIdx rngIdx; rngIdx.isValid(); rngIdx++) {
      {
        ostringstream tmp;
        tmp << "adc" << rngIdx.val();
        TH1S *h = new TH1S(tmp.str().c_str(),
                           tmp.str().c_str(),
                           singlex16::MAX_DAC+1, -0.5, singlex16::MAX_DAC+.5);
        h->SetDirectory(0);
        
        (*adcHists)[rngIdx.val()] = h;
      }

      {
        ostringstream tmp;
        tmp << "ciRaw_" << rngIdx.toStr();
        profiles[rngIdx] = new TProfile(tmp.str().c_str(),
                                        tmp.str().c_str(),
                                        (singlex16::MAX_DAC+1)/2,
                                        0,
                                        singlex16::MAX_DAC+1);
      }
    }
  }

  void IntNonlinAlg::readRootData(const string &rootFileName,
                                  CIDAC2ADC &adcMeans,
                                  const DiodeNum diode,
                                  const bool bcastMode) {
    algData.diode     = diode;
    algData.bcastMode = bcastMode;
    algData.adcMeans  = &adcMeans;

    // open input root file.
    vector<string> rootFileList;
    rootFileList.push_back(rootFileName);
    RootFileAnalysis rootFile(0,
                              &rootFileList,
                              0);

    // enable only needed branches in root file
    rootFile.getDigiChain()->SetBranchStatus("*", 0);
    rootFile.getDigiChain()->SetBranchStatus("m_calDigiCloneCol");
    rootFile.getDigiChain()->SetBranchStatus("m_metaEvent");


    //-- CHECK N EVENTS --//
    const unsigned nEvents    = rootFile.getEntries();
    const unsigned nEventsReq = (bcastMode) ? 
      m_singlex16.totalPulsesBCAST() : 
      m_singlex16.totalPulsesCOLWISE();

    // 10 % margin is overly cautious.  occasionally there are a 1 or 2 'extra' events.
    if (abs((long)(nEvents - nEventsReq)) > .10*nEventsReq) {
      ostringstream tmp;

      tmp << __FILE__ << ":" << __LINE__ << " "
          << "Wrong # of events:"
          << " bcastMode=" << bcastMode
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

  void IntNonlinAlg::processEvent(const DigiEvent &digiEvent) {
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
    if (!algData.bcastMode)
      eventData.testCol = eventData.iGoodEvt/m_singlex16.nPulsesPerXtal();
    // which DAC setting are we on?
    eventData.testDAC = (eventData.iGoodEvt%m_singlex16.nPulsesPerXtal())/m_singlex16.nPulsesPerDAC;
    // how many samples for current DAC setting?
    eventData.iSamp   = (eventData.iGoodEvt%m_singlex16.nPulsesPerXtal())%m_singlex16.nPulsesPerDAC;

    if (!checkLCICfg(digiEvent))
      return;

    const unsigned nDigis = calDigiCol->GetEntries();
    // event should have 1 hit for every xtal in each tower
    // we support any nTowers
    if (nDigis > 0 && nDigis%tXtalIdx::N_VALS == 0) {
      eventData.iGoodEvt++;

      //loop through each 'hit' in one event
      TIter calDigiIter(calDigiCol);

      const CalDigi *pdig = 0;
      while ((pdig = dynamic_cast<CalDigi*>(calDigiIter.Next()))) {
        const CalDigi &cdig = *pdig;    // shorthand.
        processHit(cdig);
      }                                 // foreach xtal
    }
    else
      LogStrm::get() << " event " << eventData.eventNum << " contains "
                     << nDigis << " digis - event skipped" << endl;
  }

  void IntNonlinAlg::processHit(const CalDigi &cdig) {
    const CalXtalId id(cdig.getPackedId());  // get interaction information

    // skip if not for current column
    const ColNum col(id.getColumn());


    if (col != eventData.testCol && !algData.bcastMode)
      return;

    const TwrNum twr(id.getTower());
    const LyrNum lyr(id.getLayer());

    // Loop through each readout on current xtal
    const unsigned short numRo = cdig.getNumReadouts();
    for (unsigned short iRo = 0; iRo < numRo; iRo++) {
      const CalXtalReadout &acRo = *(cdig.getXtalReadout(iRo));

      for (FaceNum face; face.isValid(); face++) {
        const RngNum rng(acRo.getRange((CalXtalId::XtalFace)face.val()));

        // only interested in current diode!
        if (rng.getDiode() != algData.diode) continue;

        // retrieve adc value
        const unsigned short adc(acRo.getAdc((CalXtalId::XtalFace)face.val()));

        // fill histogram
        const RngIdx rngIdx(twr,
                            lyr,
                            col,
                            face,
                            rng);

        TH1S & h = *(dynamic_cast<TH1S *>(algData.adcHists->At(rngIdx.val())));

        const float cidac = m_singlex16.CIDACTestVals()[eventData.testDAC];

        // reset histogram if we're starting a new DAC setting
        if (eventData.iSamp == 0) {
          h.Reset();
          h.SetAxisRange(-0.5, m_singlex16.MAX_DAC+.5);
          // update noise graph
          delete algData.noiseGraphs[rngIdx];
          algData.noiseGraphs[rngIdx] = new TGraph(m_singlex16.nPulsesPerDAC);
        }

        // fill histogram
        h.Fill(adc);
        // fill optional profile
        algData.profiles[rngIdx]->Fill(cidac, adc);
        // fill noise graph
        algData.noiseGraphs[rngIdx]->SetPoint(eventData.iSamp, eventData.iSamp, adc);

        /// fill optional tuple
        if (m_hugeTuple) {
          m_tupleData.rngIdx = rngIdx.val();
          m_tupleData.goodEvt = eventData.iGoodEvt;
          m_tupleData.cidac = (unsigned short)cidac;
          m_tupleData.adc = adc;
          m_hugeTuple->Fill();
        }

        // save histogram data if we're on last sample for current
        // dac settigns
        if (eventData.iSamp == (m_singlex16.nPulsesPerDAC - 1)) {
          //-- TRIM OUTLIERS
          float adcmean, adcrms;
          adcmean  = h.GetMean();
          adcrms = h.GetRMS();

          // remember initial average & rms
          const float raw_adcmean = adcmean;
          const float raw_adcrms  = adcrms;
          for ( unsigned short iter = 0; iter < 3; iter++ ) {
            h.SetAxisRange(adcmean-3*adcrms, adcmean+3*adcrms);
            adcmean  = h.GetMean();
            adcrms = h.GetRMS();
          }
          // assign to table
          algData.adcMeans->getPtsADC(rngIdx).push_back(adcmean);
          algData.adcMeans->getPtsDAC(rngIdx).push_back(cidac);

          // fill nTuple
          m_fitResults->Fill(twr.val(),
                             lyr.val(),
                             col.val(),
                             face.val(),
                             rng.val(),
                             eventData.testDAC,
                             cidac,
                             raw_adcmean,
                             raw_adcrms,
                             adcmean,
                             adcrms);

          /// throw warning if RMS > 3*average pedestal sigma for given range
          if (adcrms > rmsWarnLimit[rng.val()]) {
            LogStrm::get() << "WARNING: adc_rms " << adcrms
                           << " " << rngIdx.toStr()
                           << "  cidac "  << cidac
                           << endl;

            /// draw optional graph of this channel if we have the info
            ostringstream plotname;

            plotname << "noise_warning_dac_" << lpad(toString(cidac),4,'0')
                     << "_" << rngIdx.toStr();

            /// fix precision @ 000.00
            plotname.setf(ios::fixed,ios::floatfield);
            plotname.precision(2);
            plotname << "_rms_" << adcrms;

            TCanvas c(plotname.str().c_str(),
                      plotname.str().c_str(),
                      -1);
            algData.noiseGraphs[rngIdx]->GetHistogram()->Draw(); /// if you know a better way to do this, please tell me.
            algData.noiseGraphs[rngIdx]->Draw("*");
            c.Write();
          }
        }
      }   // foreach face
    }     // foreach readout
  }


  
  /// return false if current event LCI config does not match that expected by this analysis algorithm
  bool IntNonlinAlg::checkLCICfg(const DigiEvent &digiEvent) {
    /// retrieve / check for LCI configuration data
    const MetaEvent& metaEvt = digiEvent.getMetaEvent(); 
    const LciCalConfiguration* lciCalConf = metaEvt.lciCalConfiguration(); 
    if (lciCalConf == 0) {
      static unsigned short nWarnings = 0;
      if (nWarnings < 10000) {
        LogStrm::get() << __FILE__ << ": LCI Cal Configuration not found" << endl;
        nWarnings++;
      }

      /// this is a warning, not a failure
      return true;
    }

    /// check charge injection DAC value
    const UShort_t dac = lciCalConf->injected();
    const UShort_t expectedDAC = static_cast<unsigned short>(m_singlex16.CIDACTestVals()[eventData.testDAC]);

    if (dac != expectedDAC) {
      LogStrm::get() << __FILE__ << ": LCI DAC: " << dac 
                     << " does not match expected CIDAC: " << expectedDAC
                     << " eventNum: " << eventData.eventNum
                     << endl;
      return false;
    }

    return true;

  }

} // namespace calibGenCAL


