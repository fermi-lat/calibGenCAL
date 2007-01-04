// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/IntNonlin.cxx,v 1.4 2006/09/29 19:11:44 fewtrell Exp $

/** @file
    @author fewtrell
 */

// LOCAL INCLUDES
#include "IntNonlin.h"
#include "RootFileAnalysis.h"
#include "CIDAC2ADC.h"
#include "CalPed.h"

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

static const float CIDAC_TEST_VALS[] =
{
  0,    2,     4,      6,      8,      10,      12,      14,      16,      18,     20,    22,    24,    26,   28,   30,
  32,
  34,   36,    38,     40,     42,     44,      46,      48,      50,      52,     54,    56,    58,    60,   62,   64,
  80,   96,    112,    128,    144,    160,     176,     192,     208,     224,    240,   256,   272,   288,
  304,  320,   336,    352,    368,    384,     400,     416,     432,     448,    464,   480,   496,   512,
  543,  575,   607,    639,    671,    703,     735,     767,     799,     831,    863,   895,   927,   959,
  991,  1023,  1055,   1087,   1119,   1151,    1183,    1215,    1247,    1279,
  1311, 1343,  1375,   1407,   1439,   1471,    1503,    1535,    1567,    1599,
  1631, 1663,  1695,   1727,   1759,   1791,    1823,    1855,    1887,    1919,
  1951, 1983,  2015,   2047,   2079,   2111,    2143,    2175,    2207,    2239,
  2271, 2303,  2335,   2367,   2399,   2431,    2463,    2495,    2527,    2559,
  2591, 2623,  2655,   2687,   2719,   2751,    2783,    2815,    2847,    2879,
  2911, 2943,  2975,   3007,   3039,   3071,    3103,    3135,    3167,    3199,
  3231, 3263,  3295,   3327,   3359,   3391,    3423,    3455,    3487,    3519,
  3551, 3583,  3615,   3647,   3679,   3711,    3743,    3775,    3807,    3839,
  3871, 3903,  3935,   3967,   3999,   4031,    4063,    4095
};

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

/// number of CIDAC values tested
static const unsigned short N_CIDAC_VALS           = sizeof(CIDAC_TEST_VALS)/sizeof(*CIDAC_TEST_VALS);
/// n pulses (events) per CIDAC val
static const unsigned short N_PULSES_PER_DAC       = 50;
/// n total pulsees per xtal (or column)
static const unsigned N_PULSES_PER_XTAL      = N_CIDAC_VALS * N_PULSES_PER_DAC;

IntNonlin::IntNonlin()
{
}

void IntNonlin::AlgData::initHists() {
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

void IntNonlin::processEvent(const DigiEvent &digiEvent) {
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

void IntNonlin::processHit(const CalDigi &cdig) {
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

void IntNonlin::readRootData(const string &rootFileName,
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

void IntNonlin::genSplinePts(CIDAC2ADC &adcMeans,
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

void IntNonlin::smoothSpline(const vector<float> &curADC,
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

