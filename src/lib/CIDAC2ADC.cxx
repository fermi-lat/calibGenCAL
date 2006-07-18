// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/CIDAC2ADC.cxx,v 1.3 2006/06/27 15:36:25 fewtrell Exp $
/** @file
    @author fewtrell
 */

// LOCAL INCLUDES
#include "RootFileAnalysis.h"
#include "CIDAC2ADC.h"

// GLAST INCLUDES

// EXTLIB INCLUDES
#include "TH1S.h"
#include "TProfile.h"
#include "TGraph.h"
#include "TF1.h"
#include "TMultiGraph.h"

// STD INCLUDES
#include <sstream>
#include <ostream>

static const float CIDAC_TEST_VALS[] = 
  {0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,
   34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,64,
   80,96,112,128,144,160,176,192,208,224,240,256,272,288,
   304,320,336,352,368,384,400,416,432,448,464,480,496,512,
   543,575,607,639,671,703,735,767,799,831,863,895,927,959,
   991,1023,1055,1087,1119,1151,1183,1215,1247,1279,
   1311,1343,1375,1407,1439,1471,1503,1535,1567,1599,
   1631,1663,1695,1727,1759,1791,1823,1855,1887,1919,
   1951,1983,2015,2047,2079,2111,2143,2175,2207,2239,
   2271,2303,2335,2367,2399,2431,2463,2495,2527,2559,
   2591,2623,2655,2687,2719,2751,2783,2815,2847,2879,
   2911,2943,2975,3007,3039,3071,3103,3135,3167,3199,
   3231,3263,3295,3327,3359,3391,3423,3455,3487,3519,
   3551,3583,3615,3647,3679,3711,3743,3775,3807,3839,
   3871,3903,3935,3967,3999,4031,4063,4095};

static const unsigned short SPLINE_GRP_WIDTH[] = {3,4,3,4};
static const unsigned short SPLINE_SKIP_LO[]   = {3,1,3,1};
static const unsigned short SPLINE_SKIP_HI[]   = {6,10,6,10};
static const unsigned short SPLINE_ENABLE_GRP[]       = {1,1,1,1};

static const unsigned short N_CIDAC_VALS     = sizeof(CIDAC_TEST_VALS)/sizeof(*CIDAC_TEST_VALS);
static const unsigned short N_PULSES_PER_DAC = 50;
static const unsigned N_PULSES_PER_XTAL      = N_CIDAC_VALS * N_PULSES_PER_DAC;

CIDAC2ADC::CIDAC2ADC(ostream &ostrm) :
  m_splinePtsADC(RngIdx::N_VALS),
  m_splinePtsDAC(RngIdx::N_VALS),
  m_ostrm(ostrm)
{
}

void CIDAC2ADC::writeTXT(const string &filename) const {
  ofstream outFile(filename.c_str());
  if (!outFile.is_open()) {
    ostringstream tmp;
    tmp << __FILE__ << ":" << __LINE__ << " " 
        << "ERROR! unable to open txtFile='" << filename << "'";
    throw runtime_error(tmp.str());
  }
  outFile.precision(2);
  outFile.setf(ios_base::fixed);

  for (RngIdx rngIdx; rngIdx.isValid(); rngIdx++)
    for (unsigned n = 0; n < m_splinePtsADC[rngIdx].size(); n++) {
      RngNum rng = rngIdx.getRng();
      outFile << rngIdx.getTwr()  << " "        
              << rngIdx.getLyr()  << " "
              << rngIdx.getCol()  << " "
              << rngIdx.getFace().val() << " "
              << rng.val() << " "
              << m_splinePtsDAC[rngIdx][n] << " "
              << m_splinePtsADC[rngIdx][n]
              << endl;
    }
}

  
void CIDAC2ADC::readTXT(const string &filename) {
  ifstream inFile(filename.c_str());
  if (!inFile.is_open()) {
    ostringstream tmp;
    tmp << __FILE__ << ":" << __LINE__ << " " 
        << "ERROR! unable to open txtFile='" << filename << "'";
    throw runtime_error(tmp.str());
  }

  unsigned short twr;
  unsigned short lyr;
  unsigned short col;
  unsigned short face;
  unsigned short rng;
  float tmpDAC;
  float tmpADC;
  while (inFile.good()) {
    // load in one spline val w/ coords
    inFile >> twr
           >> lyr 
           >> col
           >> face
           >> rng
           >> tmpDAC
           >> tmpADC;
    if (inFile.fail()) break; // quit once we can't read any more values
    
    RngIdx rngIdx(twr,lyr,col,face,rng);

    m_splinePtsADC[rngIdx].push_back(tmpADC);
    m_splinePtsDAC[rngIdx].push_back(tmpDAC);
  }
}

void CIDAC2ADC::readRootData(const string &rootFileName,
                             DiodeNum diode,
                             bool bcastMode) {
  m_adcMean.resize(RngIdx::N_VALS, 
                   vector<float>(N_CIDAC_VALS, INVALID_ADC));

  // create one temporary histogram per adc channel.
  // this histogram will be reused for each new CIDAC
  // level
  //
  // these aren't worth saving since they're overwritten
  // for each cidac value, so we create them 
  // before we open the optional root ouptut file
  TObjArray adcHists(RngIdx::N_VALS);
  // delete histograms w/ TObjArray, do not save in file...
  adcHists.SetOwner(1);
  for (RngIdx rngIdx; rngIdx.isValid(); rngIdx++) {
    // only build histograms for current diode
    if (rngIdx.getRng().getDiode() != diode)
      continue;

    ostringstream tmp;
    tmp << "adc" << rngIdx.val();
    adcHists[rngIdx.val()] = new TH1S(tmp.str().c_str(),
                                      tmp.str().c_str(),
                                      4096,-0.5,4095.5);
  }
  
  // open input root file.
  vector<string> rootFileList;
  rootFileList.push_back(rootFileName);
  RootFileAnalysis rootFile(0,&rootFileList,0);

  // enable only needed branches in root file
  rootFile.getDigiChain()->SetBranchStatus("*",0);
  rootFile.getDigiChain()->SetBranchStatus("m_calDigiCloneCol");

  // profiles owned by current ROOT directory/m_histFile.
  CalVec<RngIdx, TProfile*> profiles(RngIdx::N_VALS,0);
  for (RngIdx rngIdx; rngIdx.isValid(); rngIdx++) {
    // only build histograms for current diode
    if (rngIdx.getRng().getDiode() != diode)
      continue;

    ostringstream tmp;
    tmp << "ciRaw_" << rngIdx.val();
    profiles[rngIdx] = new TProfile(tmp.str().c_str(), 
                                    tmp.str().c_str(),
                                    N_CIDAC_VALS, 
                                    -0.5, 
                                    N_CIDAC_VALS-0.5);
  }

  unsigned nEvents = rootFile.getEntries();
  m_ostrm << __FILE__ << ": Processing: " << nEvents << " events." << endl;

  int iGoodEvt = 0;
  // BEGINNING OF EVENT LOOP
  for (unsigned evtNum = 0 ; evtNum < nEvents; evtNum++) {
    if(evtNum%1000 == 0) {
      m_ostrm << " event " << evtNum << '\n';
      m_ostrm.flush();
    }

    rootFile.getEvent(evtNum);

    const TClonesArray* calDigiCol = rootFile.getDigiEvent()->getCalDigiCol();
    if (!calDigiCol) {
      ostringstream tmp;
      tmp << __FILE__ << ":" << __LINE__ << " " 
          << "Empty calDigiCol event #" << evtNum;
      throw runtime_error(tmp.str());
    }


    // -- Determine test config for this event -- //
    //    note: only count good events            //

    // which column are we testing?
    ColNum testCol;
    if (!bcastMode)
      testCol = iGoodEvt/N_PULSES_PER_XTAL;
    // which DAC setting are we on?
    int testDAC = (iGoodEvt%N_PULSES_PER_XTAL)/N_PULSES_PER_DAC;
    // how many samples for current DAC setting?
    int iSamp   = (iGoodEvt%N_PULSES_PER_XTAL)%N_PULSES_PER_DAC;
        

    int nDigis = calDigiCol->GetEntries();
    // event should have 1 hit for every xtal in each tower
    // we support any nTowers
    if(nDigis > 0 && nDigis%tXtalIdx::N_VALS == 0){
      iGoodEvt++;

      //loop through each 'hit' in one event
      TIter calDigiIter(calDigiCol);
      const CalDigi *pdig = 0;
      while ((pdig = (CalDigi*)calDigiIter.Next())) {  
        const CalDigi &cdig = *pdig; // shorthand.

        CalXtalId id = cdig.getPackedId();  // get interaction information

        // skip if not for current column
        ColNum col = id.getColumn();
        if (col != testCol && !bcastMode) 
          continue;
        
        TwrNum twr = id.getTower();
        LyrNum lyr = id.getLayer();
        
        // Loop through each readout on current xtal
        int numRo = cdig.getNumReadouts();
        for (int iRo=0; iRo<numRo; iRo++){
          const CalXtalReadout &acRo = *(cdig.getXtalReadout(iRo));

          for (FaceNum face; face.isValid(); face++) {
            RngNum rng = acRo.getRange((CalXtalId::XtalFace)face.val());

            // only interested in current diode!
            if (rng.getDiode() != diode) continue;

            // retrieve adc value
            int adc = acRo.getAdc((CalXtalId::XtalFace)face.val());

            // fill histogram
            RngIdx rngIdx(twr,lyr,col,face,rng);
            TH1S& h = *((TH1S*)adcHists[rngIdx.val()]);

            // reset histogram if we're starting a new DAC setting
            if(iSamp == 0){
              h.Reset();
              h.SetAxisRange(-0.5,4095.5);
            }

            // fill histogram
            h.Fill(adc);
            // fill optional profile
            profiles[rngIdx]->Fill(testDAC, adc);

            // save histogram data if we're on last sample for current
            // dac settigns
            if(iSamp == (N_PULSES_PER_DAC - 1)){
              float av,err;
              // trim outliers
              av  = h.GetMean();
              err = h.GetRMS();
              for( int iter=0; iter<3;iter++ ) {
                h.SetAxisRange(av-3*err,av+3*err);
                av  = h.GetMean(); 
                err = h.GetRMS();
              }
              // assign to table
              m_adcMean[rngIdx][testDAC] = av;
            }
          } // foreach face
        } // foreach readout
      } // foreach xtal
    }
    else {
      m_ostrm << " event " << evtNum << " contains " 
              << nDigis << " digis - skipped" << endl;
    }
  }  // end analysis code in event loop
}


void CIDAC2ADC::genSplinePts() {
  TF1 splineFunc("spline_fitter","pol2",0,4095);

  // 2 dimensional poly line f() to use for spline fitting.
  float *tmpADC(new float[N_CIDAC_VALS]);

  // Loop through all 4 energy ranges
  for (RngNum rng; rng.isValid(); rng++) {
    // following vals only change w/ rng, so i get them outside the other loops.
    int grpWid  = SPLINE_GRP_WIDTH[rng.val()];
    int skpLo   = SPLINE_SKIP_LO[rng.val()];
    int skpHi   = SPLINE_SKIP_HI[rng.val()];

    // loop through each xtal face.
    for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++) {
      RngIdx rngIdx(faceIdx,rng);

      // skip missing channels
      if (m_adcMean[rngIdx][0] == INVALID_ADC)
        continue;

      // point to current adc vector
      vector<float> &curADC = m_adcMean[rngIdx];
      // get pedestal
      float ped     = curADC[0];


      //calculate ped-subtracted means.
      for (int dac = 0; dac < N_CIDAC_VALS; dac++)
        curADC[dac] -= ped;

      // get upper adc boundary
      float adc_max = curADC[N_CIDAC_VALS-1];

      // last idx will be last index that is <= 0.99*adc_max
      // it is the last point we intend on using.
      int last_idx = 0; 
      while (curADC[last_idx] <= 0.99*adc_max) {
        last_idx++;
      }
      last_idx--;
      
      // set up new graph object for fitting.
      copy(curADC.begin(),
           curADC.end(),
           tmpADC);

      TGraph myGraph(last_idx+1,
                     CIDAC_TEST_VALS,
                     tmpADC);

      if (!SPLINE_ENABLE_GRP[rng.val()]) {
        ////////////////////////
        // GROUPING DISABLED  //
        ////////////////////////
        for (int i = 0; i <= last_idx; i++) {
          // put new ADC val on list
          m_splinePtsADC[rngIdx].push_back(curADC[i]);

          // put new DAC val on global output list
          m_splinePtsDAC[rngIdx].push_back(CIDAC_TEST_VALS[i]);
        }
      } else {

        //////////////////////
        // GROUPING ENABLED //
        //////////////////////

        // PART I: NO SMOOTHING/GROUPING FOR SKIPLO PTS ////////////////////////
        // copy SKPLO points directly from beginning of array.

        for (int i = 0; i < skpLo; i++) {
          // put new ADC val on list
          m_splinePtsADC[rngIdx].push_back(curADC[i]);

          // put new DAC val on global output list
          m_splinePtsDAC[rngIdx].push_back(CIDAC_TEST_VALS[i]);
        }

        // PART II: GROUP & SMOOTH MIDDLE RANGE  ///////////////////////////////
        // start one grp above skiplo & go as hi as you can w/out entering skpHi
        for (int ctrIdx = skpLo + grpWid - 1; 
             ctrIdx <= last_idx - max(skpHi,grpWid-1); // allow last group to overlap skpHi section.
             ctrIdx += grpWid) {

          int lp = ctrIdx - grpWid; // 1st point in group
          int hp  = ctrIdx + grpWid; // last point in group

          // fit curve to grouped points
          myGraph.Fit(&splineFunc,"QN","",
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
          m_splinePtsADC[rngIdx].push_back(fitADC);

          // put new DAC val on global output list
          m_splinePtsDAC[rngIdx].push_back(fitDAC);
        }

        // PART III: NO SMOOTHING/GROUPING FOR LAST SKPHI PTS //////////////////
        // copy SKPHI points directly from face of array.
        for (int i = last_idx+1 - skpHi;
             i <= last_idx;
             i++) {
          // put new ADC val on list
          m_splinePtsADC[rngIdx].push_back(curADC[i]); 

          // put new DAC val on global output list
          m_splinePtsDAC[rngIdx].push_back(CIDAC_TEST_VALS[i]);
        }

      } // grouping enabled

      //-- EXTRAPOLATE FINAL POINT --//
      int nPts    = m_splinePtsADC[rngIdx].size();
      float dac1  = m_splinePtsDAC[rngIdx][nPts-2];
      float dac2  = m_splinePtsDAC[rngIdx][nPts-1];
      float adc1  = m_splinePtsADC[rngIdx][nPts-2];
      float adc2  = m_splinePtsADC[rngIdx][nPts-1];
      float slope = (dac2 - dac1)/(adc2 - adc1);
      float dac_max = dac2 + (adc_max - adc2)*slope;

      // put final point on the list.
      m_splinePtsADC[rngIdx].push_back(adc_max);
      m_splinePtsDAC[rngIdx].push_back(dac_max);
    } // xtalFace lop
  } // range loop

  delete [] tmpADC;  
}

void CIDAC2ADC::makeGraphs(TFile &histFile) {
  // reuse to create each graphs, set to max possible size
  float *tmpADC = new float[N_CIDAC_VALS];
  float *tmpDAC = new float[N_CIDAC_VALS];

  for (RngIdx rngIdx; rngIdx.isValid(); rngIdx++) {
    // skip missing channels
    if (m_adcMean[rngIdx][0] == INVALID_ADC)
      continue;

    ostringstream mgName;
    mgName << "ciOverlay_" << rngIdx.val();
    TMultiGraph *mg = new TMultiGraph();
    mg->SetNameTitle(mgName.str().c_str(),
                     mgName.str().c_str());
    

    histFile.Add(mg);

    //-- FIRST GRAPH: ALL INPUT DATA POINTS, POST OUTLIER REJECTION --//
    // generate plot name
    ostringstream cleanName;
    cleanName << "ciCleanInput_" << rngIdx.val();

    // copy ADC points into temp array
    copy(m_adcMean[rngIdx].begin(),
         m_adcMean[rngIdx].end(),
         tmpADC);
    
    // no need to retain graph pointer as ROOT handles my memory for me
    TGraph *tmpGrClean = new TGraph(m_adcMean[rngIdx].size(),
                                    CIDAC_TEST_VALS, tmpADC);

    tmpGrClean->SetNameTitle(cleanName.str().c_str(),
                             cleanName.str().c_str());

    //-- SECOND GRAPH: OUTPUT SPLINE POINTS --//
    // generate plot name
    ostringstream splineName;
    splineName << "ciSpline_" << rngIdx.val();

    // copy ADC & DAC points into temp arrays
    copy(m_splinePtsADC[rngIdx].begin(), 
         m_splinePtsADC[rngIdx].end(), 
         tmpADC);
    copy(m_splinePtsDAC[rngIdx].begin(), 
         m_splinePtsDAC[rngIdx].end(), 
         tmpDAC);

    // no need to retain graph pointer as ROOT handles my memory for me
    TGraph *tmpGrSpline = new TGraph(m_splinePtsADC[rngIdx].size(),
                                     tmpDAC,
                                     tmpADC);
             
    tmpGrSpline->SetNameTitle(splineName.str().c_str(),
                              splineName.str().c_str());

             

    mg->Add(tmpGrClean);
    mg->Add(tmpGrSpline);
  }

  // clean up memory
  delete [] tmpADC;
  delete [] tmpDAC;
}


void CIDAC2ADC::genSplines() {
  m_splinesADC2DAC.resize(RngIdx::N_VALS, 0);
  m_splinesDAC2ADC.resize(RngIdx::N_VALS, 0);

  // ROOT functions take double arrays, not vectors 
  // so we need to copy each vector into an array
  // before loading it into a ROOT spline
  int arraySize = 100; // first guess for size, can resize later 
  double *dacs = new double[arraySize];
  double *adcs = new double[arraySize];

  for (RngIdx rngIdx; rngIdx.isValid(); rngIdx++) {
    vector<float> &adcVec = m_splinePtsADC[rngIdx];
    int nADC = adcVec.size();
    
    // Load up
    vector<float> &dacVec = m_splinePtsDAC[rngIdx];
    int nDAC = dacVec.size();
    
    // skip channel if we have no data.
    if (nADC == 0)
      continue;

    if (nADC != nDAC) {
      ostringstream tmp;
      tmp << __FILE__  << ":"     << __LINE__ << " " 
          << "nDAC != nADC for rngIdx = " << rngIdx.val();
      throw runtime_error(tmp.str());
    }

    // expand arrays if necessary
    if (nDAC > arraySize) {
      delete [] dacs;
      delete [] adcs;

      arraySize = nDAC;
      dacs = new double[arraySize];
      adcs = new double[arraySize];
    }

    // copy vector into array
    copy(dacVec.begin(),dacVec.end(),dacs);
    copy(adcVec.begin(),adcVec.end(),adcs);

    // generate splinename
    ostringstream name;
    name << "intNonlin_" << rngIdx.val();
    ostringstream nameInv;
    nameInv << "intNonlinInv_" << rngIdx.val();

    // create spline object.
    TSpline3 *mySpline    = new TSpline3(name.str().c_str(), adcs, dacs, nADC);
    TSpline3 *mySplineInv = new TSpline3(nameInv.str().c_str(), dacs, adcs, nADC);

    mySpline->SetName(name.str().c_str());
    m_splinesADC2DAC[rngIdx] = mySpline;

    mySplineInv->SetName(name.str().c_str());
    m_splinesDAC2ADC[rngIdx] = mySplineInv;
  }

  // cleanup
  delete [] dacs;
  delete [] adcs;
}


void CIDAC2ADC::writeADCMeans(const string &filename) const {
  ofstream outfile(filename.c_str());
  if (!outfile.is_open())
    throw runtime_error(string("Unable to open " + filename));

  for (RngIdx rngIdx; rngIdx.isValid(); rngIdx++) {
    TwrNum twr   = rngIdx.getTwr();
    LyrNum lyr   = rngIdx.getLyr();
    ColNum col   = rngIdx.getCol();
    FaceNum face = rngIdx.getFace();
    RngNum rng   = rngIdx.getRng();

    for (unsigned dacIdx = 0;
         dacIdx < N_CIDAC_VALS;
         dacIdx++) {

      float adcMean = m_adcMean[rngIdx][dacIdx];

      if (adcMean == INVALID_ADC)
        continue;

      outfile << " " << twr
              << " " << lyr
              << " " << col
              << " " << face.val()
              << " " << rng.val()
              << " " << dacIdx
              << " " << CIDAC_TEST_VALS[dacIdx]
              << " " << m_adcMean[rngIdx][dacIdx]
              << endl;
    }
  }
}

void CIDAC2ADC::readADCMeans(const string &filename) {
  m_adcMean.resize(RngIdx::N_VALS, 
                   vector<float>(N_CIDAC_VALS, INVALID_ADC));

  unsigned short twr, lyr, col, face, rng, dacIdx;
  float dac, adc;

  // open file
  ifstream infile(filename.c_str());
  if (!infile.is_open())
    throw runtime_error(string("Unable to open " + filename));

  // loop through each line in file
  while (infile.good()) {
    // get lyr, col (xtalId)
    infile >> twr >> lyr >> col >> face >> rng >> dacIdx
           >> dac >> adc;
    
    if (infile.fail()) break; // bad get()

    RngIdx rngIdx(twr, lyr, col, face, rng);

    m_adcMean[rngIdx][dacIdx] = adc;
  }
}
