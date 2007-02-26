// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/IntNonlin.cxx,v 1.6 2007/01/05 17:25:34 fewtrell Exp $

/** @file
    @author fewtrell
*/

// LOCAL INCLUDES
#include "IntNonlinAlg.h"
#include "RootFileAnalysis.h"
#include "CIDAC2ADC.h"
#include "CalPed.h"
#include "singlex16.h"

// GLAST INCLUDES
#include "CalUtil/CalVec.h"
#include "digiRootData/DigiEvent.h"

// EXTLIB INCLUDES
#include "TH1S.h"
#include "TProfile.h"
#include "TF1.h"

// STD INCLUDES
#include <sstream>

using namespace CGCUtil;
using namespace CalUtil;
using namespace std;
using namespace singlex16;


IntNonlinAlg::IntNonlinAlg()
{
}

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

void IntNonlinAlg::processHit(const CalDigi &cdig) {
  CalXtalId id  = cdig.getPackedId();  // get interaction information

  // skip if not for current column
  ColNum    col = id.getColumn();


  if (col != eventData.testCol && !algData.bcastMode)
    return;

  TwrNum twr   = id.getTower();
  LyrNum lyr   = id.getLayer();

  // Loop through each readout on current xtal
  unsigned short numRo = cdig.getNumReadouts();
  for (unsigned short iRo = 0; iRo < numRo; iRo++) {
    const CalXtalReadout &acRo = *(cdig.getXtalReadout(iRo));

    for (FaceNum face; face.isValid(); face++) {
      RngNum rng = acRo.getRange((CalXtalId::XtalFace)face.val());

      // only interested in current diode!
      if (rng.getDiode() != algData.diode) continue;

      // retrieve adc value
      unsigned short adc = acRo.getAdc((CalXtalId::XtalFace)face.val());

      // fill histogram
      RngIdx rngIdx(twr,
                    lyr,
                    col,
                    face,
                    rng);

      TH1S & h = *((TH1S *)algData.adcHists->At(rngIdx.val()));

      // reset histogram if we're starting a new DAC setting
      if (eventData.iSamp == 0) {
        h.Reset();
        h.SetAxisRange(-0.5, 4095.5);
      }

      // fill histogram
      h.Fill(adc);
      // fill optional profile
      algData.profiles[rngIdx]->Fill(eventData.testDAC, adc);

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
        algData.adcMeans->getPtsDAC(rngIdx).push_back(CIDAC_TEST_VALS[eventData.testDAC]);
      }
    }   // foreach face
  }     // foreach readout
}

void IntNonlinAlg::readRootData(const string &rootFileName,
                             CIDAC2ADC &adcMeans,
                             DiodeNum diode,
                             bool bcastMode) {
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

void IntNonlinAlg::genSplinePts(CIDAC2ADC &adcMeans,
                             CIDAC2ADC &cidac2adc) {
  // Loop through all 4 energy ranges
  for (RngNum rng; rng.isValid(); rng++)
    // loop through each xtal face.
    for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++) {
      RngIdx rngIdx(faceIdx,
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
                             RngNum rng
                             ) {
  // following vals only change w/ rng, so i get them outside the other loops.
  unsigned short grpWid       = SMOOTH_GRP_WIDTH[rng.val()];
  unsigned short extrapLo     = EXTRAP_PTS_LO[rng.val()];
  unsigned short extrapLoFrom = EXTRAP_PTS_LO_FROM[rng.val()];
  unsigned short skpHi        = SMOOTH_SKIP_HI[rng.val()];


  // 2 dimensional poly line f() to use for spline fitting.
  float * tmpADC(new float[N_CIDAC_VALS]);

  TF1     splineFunc("spline_fitter",
                     "pol2",
                     0,
                     4095);

  //-- GET UPPER ADC BOUNDARY for this channel --//
  float adc_max  = curADC[N_CIDAC_VALS-1];
  // last idx will be last index that is <= 0.99*adc_max
  // it is the last point we intend on using.
  unsigned short last_idx = 0;
  while (curADC[last_idx] <= 0.99*adc_max)
    last_idx++;
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
    unsigned short pt2  = extrapLo;
    // n points into curve from pt2
    unsigned short pt1  = pt2+extrapLoFrom-1;

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
    unsigned short lp  = ctrIdx - grpWid;                          // 1st point in group
    unsigned short hp  = ctrIdx + grpWid;                          // last point in group

    // fit curve to grouped points
    myGraph.Fit(&splineFunc, "QN", "",
                CIDAC_TEST_VALS[lp],
                CIDAC_TEST_VALS[hp]);
    float myPar1 = splineFunc.GetParameter(0);
    float myPar2 = splineFunc.GetParameter(1);
    float myPar3 = splineFunc.GetParameter(2);

    // use DAC value from center point
    float fitDAC = CIDAC_TEST_VALS[ctrIdx];
    // eval smoothed ADC value
    float fitADC = myPar1 + fitDAC*(myPar2 + fitDAC*myPar3);

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
  unsigned short nPts = splineADC.size();
  float dac1          = splineDAC[nPts-2];
  float dac2          = splineDAC[nPts-1];
  float adc1          = splineADC[nPts-2];
  float adc2          = splineADC[nPts-1];
  float slope         = (dac2 - dac1)/(adc2 - adc1);
  float dac_max       = dac2 + (adc_max - adc2)*slope;

  // put final point on the list.
  splineADC.push_back(adc_max);
  splineDAC.push_back(dac_max);

  delete [] tmpADC;
}

