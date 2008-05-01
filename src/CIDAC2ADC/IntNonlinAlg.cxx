// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/CIDAC2ADC/IntNonlinAlg.cxx,v 1.2 2008/04/28 14:58:29 fewtrell Exp $

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
#include "TF1.h"

// STD INCLUDES
#include <sstream>
#include <cmath>


namespace {
  /// how many points for each smoothing 'group'?  (per adc range)
  static const unsigned short SMOOTH_GRP_WIDTH[] = {
    3, 4, 3, 4
  };
  /// how many points at beginning of curve to extrapolate from following points
  static const unsigned short EXTRAP_PTS_LO[]    = {
    2, 2, 2, 2
  };
  /// how many points to extrapolate beginning of curve _from_
  static const unsigned short EXTRAP_PTS_LO_FROM[] = {
    5, 5, 5, 5
  };
  /// how many points at end of curve not to smooth (simply copy them over verbatim from raw data)
  static const unsigned short SMOOTH_SKIP_HI[]   = {
    6, 10, 6, 10
  };

};

namespace calibGenCAL {

  using namespace CalUtil;
  using namespace std;
  using namespace singlex16;
  
  void IntNonlinAlg::AlgData::initHists() {
    adcHists = new TObjArray(RngIdx::N_VALS);

    // delete histograms w/ TObjArray, do not save in file...
    adcHists->SetOwner(1);
    for (RngIdx rngIdx; rngIdx.isValid(); rngIdx++) {
      {
        ostringstream tmp;
        tmp << "adc" << rngIdx.val();
        (*adcHists)[rngIdx.val()] = new TH1S(tmp.str().c_str(),
                                             tmp.str().c_str(),
                                             MAX_DAC+1, -0.5, MAX_DAC+.5);
      }

      {
        ostringstream tmp;
        tmp << "ciRaw_" << rngIdx.toStr();
        profiles[rngIdx] = new TProfile(tmp.str().c_str(),
                                        tmp.str().c_str(),
                                        (MAX_DAC+1)/2,
                                        0,
                                        MAX_DAC+1);

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
    const unsigned nEventsReq = (bcastMode) ? TOTAL_PULSES_BCAST : TOTAL_PULSES_COLWISE;

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
      eventData.testCol = eventData.iGoodEvt/N_PULSES_PER_XTAL;
    // which DAC setting are we on?
    eventData.testDAC = (eventData.iGoodEvt%N_PULSES_PER_XTAL)/N_PULSES_PER_DAC;
    // how many samples for current DAC setting?
    eventData.iSamp   = (eventData.iGoodEvt%N_PULSES_PER_XTAL)%N_PULSES_PER_DAC;

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

        // reset histogram if we're starting a new DAC setting
        if (eventData.iSamp == 0) {
          h.Reset();
          h.SetAxisRange(-0.5, MAX_DAC+.5);
        }

        // fill histogram
        h.Fill(adc);
        // fill optional profile
        const float cidac = CIDAC_TEST_VALS[eventData.testDAC];
        algData.profiles[rngIdx]->Fill(cidac, adc);

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
          algData.adcMeans->getPtsADC(rngIdx).push_back(av);
          algData.adcMeans->getPtsDAC(rngIdx).push_back(cidac);
        }
      }   // foreach face
    }     // foreach readout
  }

  void IntNonlinAlg::genSplinePts(const CIDAC2ADC &adcMeans,
                                  CIDAC2ADC &cidac2adc) {
    // Loop through all 4 energy ranges
    for (RngNum rng; rng.isValid(); rng++)
      // loop through each xtal face.
      for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++) {
        const RngIdx rngIdx(faceIdx,
                            rng);

        // skip missing channels
        if (adcMeans.getPtsADC(rngIdx).empty())
          continue;

        // point to current adc vector
        const vector<float> &curADC = adcMeans.getPtsADC(rngIdx);

        // point to output splines
        vector<float> &splineADC = cidac2adc.getPtsADC(rngIdx);
        vector<float> &splineDAC = cidac2adc.getPtsDAC(rngIdx);

        smoothSpline(curADC, splineADC, splineDAC, rng);
      } // xtalFace lop
    // range loop

    // subtract pedestals
    cidac2adc.pedSubtractADCSplines();
  }

  void IntNonlinAlg::smoothSpline(const vector<float> &curADC,
                                  vector<float> &splineADC,
                                  vector<float> &splineDAC,
                                  const RngNum rng
                                  ) {
    // following vals only change w/ rng, so i get them outside the other loops.
    const unsigned short grpWid       = SMOOTH_GRP_WIDTH[rng.val()];
    const unsigned short extrapLo     = EXTRAP_PTS_LO[rng.val()];
    const unsigned short extrapLoFrom = EXTRAP_PTS_LO_FROM[rng.val()];
    const unsigned short skpHi        = SMOOTH_SKIP_HI[rng.val()];


    // 2 dimensional poly line f() to use for spline fitting.
    float *tmpADC(new float[N_CIDAC_VALS]);

    TF1 splineFunc("spline_fitter",
                   "pol2",
                   0,
                   MAX_DAC);

    //-- GET UPPER ADC BOUNDARY for this channel --//
    const float adc_max  = curADC[N_CIDAC_VALS-1];
    // last idx will be last index that is <= 0.99*adc_max
    // it is the last point we intend on using.
    unsigned short last_idx = 0;
    while (curADC[last_idx] <= 0.99*adc_max  
           && 
           last_idx < N_CIDAC_VALS-1) //bounds check
      last_idx++;
    if (last_idx > 0)
      last_idx--;

    //-- CREATE GRAPH OBJECT for fitting --//
    copy(curADC.begin(),
         curADC.end(),
         tmpADC);
    TGraph myGraph(last_idx+1,
                   CIDAC_TEST_VALS,
                   tmpADC);

    // PART I: EXTRAPOLATE INITIAL POINTS FROM MEAT OF CURVE
    for (unsigned short i = 0; i < extrapLo; i++) {
      // put new DAC val on global output list
      const float &  dac  = CIDAC_TEST_VALS[i];
      splineDAC.push_back(dac);

      // extrapolate associated adc value from points later in curve
      // 1st non extrapolated point
      const unsigned short pt2  = extrapLo;
      // n points into curve from pt2
      const unsigned short pt1  = pt2+extrapLoFrom-1;

      const float &  dac2 = CIDAC_TEST_VALS[pt2];
      const float &  adc2 = curADC[pt2];

      const float &  dac1 = CIDAC_TEST_VALS[pt1];
      const float &  adc1 = curADC[pt1];

      float adc  = linear_extrap(dac1, dac2, dac,
                                 adc1, adc2);

      splineADC.push_back(adc);
    }

    // PART II: GROUP & SMOOTH MIDDLE RANGE  ///////////////////////////////
    // start one grp above skiplo & go as hi as you can w/out entering skpHi
    for (unsigned short ctrIdx = extrapLo + grpWid - 1;
         ctrIdx <= last_idx - max<unsigned short>(skpHi, grpWid-1);  // allow last group to overlap skpHi section.

         ctrIdx += grpWid) {
      const unsigned short lp  = ctrIdx - grpWid;                          // 1st point in group
      const unsigned short hp  = ctrIdx + grpWid;                          // last point in group

      // fit curve to grouped points
      myGraph.Fit(&splineFunc, "QN", "",
                  CIDAC_TEST_VALS[lp],
                  CIDAC_TEST_VALS[hp]);
      const float myPar1 = splineFunc.GetParameter(0);
      const float myPar2 = splineFunc.GetParameter(1);
      const float myPar3 = splineFunc.GetParameter(2);

      // use DAC value from center point
      const float fitDAC = CIDAC_TEST_VALS[ctrIdx];
      // eval smoothed ADC value
      const float fitADC = myPar1 + fitDAC*(myPar2 + fitDAC*myPar3);

      // put new ADC val on list
      splineADC.push_back(fitADC);

      // put new DAC val on global output list
      splineDAC.push_back(fitDAC);
    }

    // PART III: NO SMOOTHING/GROUPING FOR LAST SKPHI PTS //////////////////
    // copy SKPHI points directly from face of array.
    for (unsigned short i = last_idx+1 - skpHi;
         i <= last_idx;
         i++) {
      // put new ADC val on list
      splineADC.push_back(curADC[i]);

      // put new DAC val on global output list
      splineDAC.push_back(CIDAC_TEST_VALS[i]);
    }

    //-- EXTRAPOLATE FINAL POINT --//
    const unsigned short nPts = splineADC.size();
    const float dac1          = splineDAC[nPts-2];
    const float dac2          = splineDAC[nPts-1];
    const float adc1          = splineADC[nPts-2];
    const float adc2          = splineADC[nPts-1];
    const float slope         = (dac2 - dac1)/(adc2 - adc1);
    const float dac_max       = dac2 + (adc_max - adc2)*slope;

    // put final point on the list.
    splineADC.push_back(adc_max);
    splineDAC.push_back(dac_max);

    delete [] tmpADC;
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
    const UShort_t expectedDAC = static_cast<unsigned short>(CIDAC_TEST_VALS[eventData.testDAC]);

    if (dac != expectedDAC) {
      LogStrm::get() << __FILE__ << ": LCI DAC: " << dac 
                     << " does not match expected CIDAC: " << expectedDAC
                     << " eventNum: " << eventData.eventNum
                     << endl;
      return false;
    }

    return true;

  }
}; // namespace calibGenCAL

