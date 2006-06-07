// LOCAL INCLUDES
#include "MuonCalib.h"
#include "CGCUtil.h"
#include "CalArray.h"

// GLAST INCLUDES

// EXTLIB INCLUDES
#include "TF1.h"
#include "TCanvas.h"
#include "TH2S.h"

// STD INCLUDES
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <functional>

using namespace std;
using namespace CGCUtil;

//////////////////////////GENERAL UTILITIES ////////////////////////////////////

/// functional class deletes a pointer
/// fun to use w/ for_each template
///
/// I got it from here - Z.F.
/// Newsgroups: comp.lang.c++.moderated
/// From: Didier Trosset <didier-dot-tros...@acqiris.com>
/// Date: 21 Oct 2004 15:39:18 -0400
/// Subject: Re: Standard way to delete container of pointers
struct delete_ptr
{
  template <class P>
  void operator() (P p)
  {
    delete p;
    p = 0;
  }
};

/// template function calls delete on all pointers stored in a STL container
template<class T> void del_all_ptrs(T &container) {
  for_each(container.begin(),container.end(),delete_ptr());
}

////////////////////////////////////////////////////////////////////////////////

MuonCalib::MuonCalib(McCfg &cfg) : 
  RootFileAnalysis(vector<string>(0),    // no mc files
                   cfg.rootFileList,   // input digi files
                   vector<string>(0),    // no recon files
                   cfg.ostrm),
  m_cfg(cfg)
{
  //open up histogram file if desired
  if (m_histFilename.length()) {
    m_histFile.reset(new TFile(m_histFilename.c_str(), 
                               "RECREATE","MuonCalibHistograms",9));
  }

  // configure ROOT Tree - enable only branches we are going to use.
  if (m_mcEnabled) 
    // disable all branches
    m_mcChain.SetBranchStatus("*", 0);


  if (m_digiEnabled) {
    // disable all branches - opt in
    m_digiChain.SetBranchStatus("*",0);

    // activate desired brances
    m_digiChain.SetBranchStatus("m_cal*",1);
    m_digiChain.SetBranchStatus("m_eventId", 1);
  }

  // disable all branches
  if (m_recEnabled) m_recChain.SetBranchStatus("*",0);
}

void MuonCalib::flushHists() {
  // write current histograms to file & close if we have an open file.
  if (m_histFile.get()) {
    m_histFile->Write();
    m_histFile->Close(); // all histograms deleted.
    delete m_histFile.release();
  }

  // clear pointer arrays
  // ped
  m_roughPedHists.clear();
  m_pedHists.clear();

  // asym
  m_asymHists.clear();


  // mpd
  m_dacL2SHists.clear();
  m_dacL2SSlopeProfs.clear();
  m_dacLLHists.clear();
}

void MuonCalib::openHistFile(const string &filename) {
  if (m_histFile.get()) flushHists();

  m_histFilename = filename;

  m_histFile.reset(new TFile(m_histFilename.c_str(), 
                             "RECREATE", "MuonCalibHistograms",9));
}

int MuonCalib::chkForEvts(int nEvts) {
  Int_t nEntries = getEntries();
  m_ostrm << "\nTotal num Events in File is: " << nEntries << endl;

  if (nEntries <= 0 || m_startEvt == nEntries) // already reached end of chain.
    throw string("No more events available for processing");

  int lastReq = nEvts + m_startEvt;

  // CASE A: we have enough events
  if (lastReq <= nEntries) return lastReq;

  // CASE B: we don't have enough
  int evtsLeft = nEntries - m_startEvt;
  m_ostrm << " EOF before " << nEvts << ". Will process remaining " <<
    evtsLeft << " events." << endl;
  return evtsLeft;
}

UInt_t MuonCalib::getEvent(UInt_t ievt) {
  if (m_digiEvt) m_digiEvt->Clear();

  int nb = RootFileAnalysis::getEvent(ievt);
  //make sure that m_digiEvt is valid b/c we will assume so after this
  if (m_digiEvt && nb) { 
    m_evtId = m_digiEvt->getEventId();
    if(m_evtId%1000 == 0) {
      m_ostrm << " event " << m_evtId << '\r';
      m_ostrm.flush();
    }
    
  }

  return nb;
}

void MuonCalib::initRoughPedHists() {
  m_roughPedHists.resize(tFaceIdx::N_VALS);
  
  for (tFaceIdx faceIdx; faceIdx.isValid(); faceIdx++) {
    ostringstream tmp;
    tmp << "roughpeds_" << faceIdx;

    m_roughPedHists[faceIdx] = new TH1S(tmp.str().c_str(),
                                        tmp.str().c_str(),
                                        500,0,1000);
  }
}

void MuonCalib::fillRoughPedHists(int nEvts) {
  initRoughPedHists();

  int lastEvt = chkForEvts(nEvts);

  // Basic digi-event loop
  for (Int_t iEvt = m_startEvt; iEvt < lastEvt; iEvt++) {
    if (!getEvent(iEvt)) {
      m_ostrm << "Warning, event " << iEvt << " not read." << endl;
      continue;
    }

    const TClonesArray* calDigiCol = m_digiEvt->getCalDigiCol();
    if (!calDigiCol) {
      cerr << "no calDigiCol found for event#" << iEvt << endl;
      continue;
    }

    TIter calDigiIter(calDigiCol);
    CalDigi *pCalDigi = 0;

    //loop through each 'hit' in one event
    while ((pCalDigi = (CalDigi*)calDigiIter.Next())) {
      CalDigi &calDigi = *pCalDigi; // use reference to avoid -> syntax

      //-- XtalId --//
      CalXtalId id(calDigi.getPackedId()); // get interaction information
      // skip hits not for current tower.
      TwrNum twr = id.getTower();
      if ((int)twr != m_cfg.twrBay) 
        continue; 

      ColNum col = id.getColumn();
      LyrNum lyr = id.getLayer();
      for (FaceNum face; face.isValid(); face++) {
        float adc = calDigi.getAdcSelectedRange(LEX8, (CalXtalId::XtalFace)(unsigned short)face);

        // check for missing readout
        bool badHit = false;
        if (adc < 0 ) {
          m_ostrm << "Couldn't get LEX8 readout for evt=" << m_evtId << endl;
          badHit = true;
          continue;
        }
        if (badHit) continue;

        tFaceIdx faceIdx(lyr,col,face);
        m_roughPedHists[faceIdx]->Fill(adc);
      }
    }
  }
}

void MuonCalib::fitRoughPedHists() {
  m_calRoughPed.resize(tFaceIdx::N_VALS);
  m_calRoughPedErr.resize(tFaceIdx::N_VALS);

  for (tFaceIdx faceIdx; faceIdx.isValid(); faceIdx++) {
    // select histogram from list
    TH1S &h= *m_roughPedHists[faceIdx];

    // trim outliers
    float av = h.GetMean();float err =h.GetRMS();
    for( int iter=0; iter<3;iter++ ) {
      h.SetAxisRange(av-3*err,av+3*err);
      av = h.GetMean(); err= h.GetRMS();
    }
    
    // gaussian fit
    h.Fit("gaus", "Q","", av-3*err, av+3*err );
    h.SetAxisRange(av-150,av+150);

    // assign values to permanent arrays
    m_calRoughPed[faceIdx] =
      ((TF1&)*h.GetFunction("gaus")).GetParameter(1);
    m_calRoughPedErr[faceIdx] =
      ((TF1&)*h.GetFunction("gaus")).GetParameter(2);
  }
}

void MuonCalib::writeRoughPedsTXT(const string &filename) const {
  ofstream outfile(filename.c_str());
  if (!outfile.is_open())
    throw string("Unable to open " + filename);
  
  for (tFaceIdx faceIdx; faceIdx.isValid(); faceIdx++)
    outfile << m_cfg.twrBay
            << " " << faceIdx.getLyr()
            << " " << faceIdx.getCol()
            << " " << faceIdx.getFace()
            << " " << m_calRoughPed[faceIdx]
            << " " << m_calRoughPedErr[faceIdx]
            << endl;
}

void MuonCalib::initPedHists() {
  m_pedHists.resize(tRngIdx::N_VALS);

  for (tRngIdx rngIdx; rngIdx.isValid(); rngIdx++) {
    ostringstream tmp;
    tmp << "peds_" << rngIdx;
    m_pedHists[rngIdx] = new TH1S(tmp.str().c_str(),
                                  tmp.str().c_str(),
                                  1000,0,1000);
  }
}

void MuonCalib::fillPedHists(int nEvts) {
  initPedHists();

  int lastEvt = chkForEvts(nEvts);

  // Basic digi-event loop
  for (Int_t iEvt = m_startEvt; iEvt < lastEvt; iEvt++) {
    if (!getEvent(iEvt)) {
      m_ostrm << "Warning, event " << iEvt << " not read." << endl;
      continue;
    }

    const TClonesArray* calDigiCol = m_digiEvt->getCalDigiCol();
    if (!calDigiCol) {
      cerr << "no calDigiCol found for event#" << iEvt << endl;
      continue;
    }

    TIter calDigiIter(calDigiCol);
    CalDigi *pCalDigi = 0;

    //loop through each 'hit' in single event
    while ((pCalDigi = (CalDigi*)calDigiIter.Next())) {
      CalDigi &calDigi = *pCalDigi; // use reference to avoid -> syntax

      //-- XtalId --//
      CalXtalId id(calDigi.getPackedId()); // get interaction information
      // skip hits not for current tower.
      TwrNum twr(id.getTower());
      if ((int)twr != m_cfg.twrBay) continue; 

      ColNum col = id.getColumn();
      LyrNum lyr = id.getLayer();

      tXtalIdx xtalIdx(lyr,col);
      int nRO = calDigi.getNumReadouts();

      if (nRO != 4) {
        ostringstream tmp;
        tmp << __FILE__  << ":"     << __LINE__ << " " 
            << "Event# " << m_evtId << " Invalid nReadouts, expecting 4";
        throw tmp.str();
      }
      
      // 1st look at LEX8 vals
      CalArray<FaceNum, float> adc;
      bool badHit = false;
      for (FaceNum face; face.isValid(); face++) {
        adc[face] = calDigi.getAdcSelectedRange(LEX8, (CalXtalId::XtalFace)(unsigned short)face);
        // check for missing readout
        if (adc[face] < 0) {
          m_ostrm << "Couldn't get LEX8 readout for evt=" << m_evtId << endl;
          badHit = true;
          continue;
        }
      }
      if (badHit) continue;

      // skip outliers (outside of 5 sigma on either side)
      if (fabs(adc[NEG_FACE] - m_calRoughPed[tFaceIdx(xtalIdx,NEG_FACE)]) < 
          5*m_calRoughPedErr[tFaceIdx(xtalIdx,NEG_FACE)] &&
          fabs(adc[POS_FACE] - m_calRoughPed[tFaceIdx(xtalIdx,POS_FACE)]) < 
          5*m_calRoughPedErr[tFaceIdx(xtalIdx,POS_FACE)]) {

        for (short n = 0; n < nRO; n++) {
          const CalXtalReadout &readout = *calDigi.getXtalReadout(n);
          
          for (FaceNum face; face.isValid(); face++) {
            // check that we are in the expected readout mode
            RngNum rng = readout.getRange((CalXtalId::XtalFace)(unsigned short)face);
            int adc = readout.getAdc((CalXtalId::XtalFace)(unsigned short)face);
            tRngIdx rngIdxP(xtalIdx, face, rng);
            m_pedHists[rngIdxP]->Fill(adc);
          }
        }
      }
    }
  }
}

void MuonCalib::fitPedHists() {
  m_calPed.resize(tRngIdx::N_VALS);
  m_calPedErr.resize(tRngIdx::N_VALS);

  TF1 mygaus("mygaus","gaus",0,1000);
  //mygaus.SetParLimits(2,0.5,1000);  // put limit on sigma parameter
  
  for (tRngIdx rngIdx; rngIdx.isValid(); rngIdx++) {
    //select histogram from list
    TH1S &h= *m_pedHists[rngIdx];
    
    // trim outliers
    float av = h.GetMean();float err =h.GetRMS();
    for( int iter=0; iter<3;iter++ ) {
      h.SetAxisRange(av-3*err,av+3*err);
      av  = h.GetMean(); 
      err = h.GetRMS();
    }
    
    // Gaussian fit doesn't seem to do well w/ 
    // sm # of  populated bins.  if RMS is 
    // low, it's good enough to return mean & RMS
    if (err < 0.7) {
      m_calPed[rngIdx]    = av;
      m_calPedErr[rngIdx] = err;
      continue;
    }

    // reset initial fit parameters
    mygaus.SetParameter(0,h.GetEntries()/err); // ballpark for height.
    mygaus.SetParameter(1,av);
    mygaus.SetParameter(2,err);

    // run gaussian fit
    float fitWidth = max(3.0*err,5.0); 
    h.Fit("mygaus", "QBI","", av-fitWidth, av+fitWidth);
    h.SetAxisRange(av-150,av+150);

    //assign values to permanet arrays
    m_calPed[rngIdx] =
      mygaus.GetParameter(1);
    m_calPedErr[rngIdx] =
      mygaus.GetParameter(2);
  }
}

void MuonCalib::writePedsTXT(const string &filename) const {
  ofstream outfile(filename.c_str());
  if (!outfile.is_open())
    throw string("Unable to open " + filename);
  
  for (tRngIdx rngIdx; rngIdx.isValid(); rngIdx++) {
    outfile << m_cfg.twrBay
            << " " << rngIdx.getLyr()
            << " " << rngIdx.getCol()
            << " " << rngIdx.getFace()
            << " " << rngIdx.getRng()
            << " " << m_calPed[rngIdx]
            << " " << m_calPedErr[rngIdx]
            << endl;
  }
}

void MuonCalib::readPedsTXT(const string &filename) {
  m_calPed.resize(tRngIdx::N_VALS);
  m_calPedErr.resize(tRngIdx::N_VALS);

  ifstream infile(filename.c_str());
  if (!infile.is_open())
    throw string("Unable to open " + filename);

  unsigned nRead = 0;
  while(infile.good()) {
    float av,err;
    short twr;
    short lyr;
    short col;
    short face;
    short rng;
    
    infile >> twr
           >> lyr
           >> col
           >> face
           >> rng
           >> av
           >> err;
    // quit once we can't read any more values
    if (infile.fail()) break; 

    // skip towers we're not using
    if (twr != m_cfg.twrBay) 
      continue;

    nRead++;

    tRngIdx rngIdx(lyr,col,face,rng);
    m_calPed[rngIdx]= av;
    m_calPedErr[rngIdx]= err;
  }

  if (nRead != m_calPed.size()) {
    ostringstream tmp;
    tmp << __FILE__ << ":" << __LINE__ << " " 
        << "CalPed file '" << filename << "' is incomplete: " << nRead
        << " pedestal values read, "   << m_calPed.size() << " vals required.";
    throw tmp.str();
  }
}

void MuonCalib::readIntNonlin(const string &filename) {
  m_calInlADC.resize(tDiodeIdx::N_VALS);
  m_calInlDAC.resize(tDiodeIdx::N_VALS);

  ifstream infile(filename.c_str());
  if (!infile.is_open())
    throw string("Unable to open " + filename);

  string lineStr;
  int nLine = 0;

  getline(infile, lineStr);
  while(infile.good()) { //loop on each line.
    nLine++;
    short twr;
    short lyr;
    short col;
    short face;
    short rng;

    float adc, dac;

    istringstream lineStream(lineStr);

    lineStream >> twr
               >> lyr
               >> col
               >> face
               >> rng
               >> dac
               >> adc; // first 4 columns in file id the xtal & adc range
    if (lineStream.fail()) {
      ostringstream nLineStr;
      nLineStr << nLine;
      throw string("Inl file '" + filename + "' is incomplete. (line "
                   + nLineStr.str() + ")");
    }
    // skip if not current tower
    if ((int)twr != m_cfg.twrBay) continue;

    // we are only interested in X8 splines for MuonCalibration
    if (rng == LEX8 || rng == HEX8) {
      tDiodeIdx diodeIdx(lyr,col,face,RngNum(rng).getDiode());

      // load values into vectors
      m_calInlADC[diodeIdx].push_back(adc);
      m_calInlDAC[diodeIdx].push_back(dac);
    }

    getline(infile, lineStr);
  }

  loadInlSplines();
}

void MuonCalib::loadInlSplines() {
  m_inlSplines.resize(tDiodeIdx::N_VALS);
  m_inlSplinesInv.resize(tDiodeIdx::N_VALS);

  // ROOT functions take double arrays, not vectors 
  // so we need to copy each vector into an array
  // before loading it into a ROOT spline
  int arraySize = 100; // first guess for size, can resize later 
  double *dacs = new double[arraySize];
  double *adcs = new double[arraySize];

  for (tDiodeIdx diodeIdx; diodeIdx.isValid(); diodeIdx++) {

    vector<float> &adcVec = m_calInlADC[diodeIdx];
    int nADC = adcVec.size();

    // Load up
    vector<float> &dacVec = m_calInlDAC[diodeIdx];
    int nDAC = dacVec.size();

    if (nADC == 0 || nDAC == 0) {
      ostringstream tmp;
      tmp << __FILE__  << ":"     << __LINE__ << " " 
          << "Event# " << m_evtId << "Zero elements for diodeIdx = " << diodeIdx;
      throw tmp.str();
    }

    if (nADC != nDAC) {
      ostringstream tmp;
      tmp << __FILE__  << ":"     << __LINE__ << " " 
          << "Event# " << m_evtId << "nDAC != nADC for diodeIdx = " << diodeIdx;
      throw tmp.str();
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
    name << "intNonlin_" << diodeIdx;
    ostringstream nameInv;
    nameInv << "intNonlinInv_" << diodeIdx;

    // create spline object.
    TSpline3 *mySpline    = new TSpline3(name.str().c_str(), adcs, dacs, nADC);
    TSpline3 *mySplineInv = new TSpline3(name.str().c_str(), dacs, adcs, nADC);

    mySpline->SetName(name.str().c_str());
    m_inlSplines[diodeIdx] = mySpline;

    mySplineInv->SetName(name.str().c_str());
    m_inlSplinesInv[diodeIdx] = mySplineInv;
  }

  // cleanup
  delete [] dacs;
  delete [] adcs;
}

void MuonCalib::HitSummary::clear() {
  // zero out all vectors
  fill_zero(adc_ped);
  fill_zero(perLyrX);
  fill_zero(perLyrY);
  fill_zero(perColX);
  fill_zero(perColY);

  // empty hit lists
  hitListX.clear();
  hitListY.clear();

  // zero out primitives
  count      = 0;
  nLyrsX     = 0;
  nLyrsY     = 0;
  nColsX     = 0;
  nColsY     = 0;
  maxPerLyr  = 0;
  maxPerLyrX = 0;
  maxPerLyrY = 0;
  firstColX  = 0;
  firstColY  = 0;

  goodXTrack = false;
  goodYTrack = false;
  status = false;
}

void MuonCalib::summarizeHits(HitSummary &hs) {
  hs.clear();

  const TClonesArray* calDigiCol = m_digiEvt->getCalDigiCol();
  if (!calDigiCol) {
    cerr << "no calDigiCol found for event" << m_digiEvt << endl;
    return;
  }

  TIter calDigiIter(calDigiCol);
  CalDigi *pCalDigi = 0;

  // PER HIT LOOP:
  while ((pCalDigi = (CalDigi*)calDigiIter.Next())) {
    CalDigi &calDigi = *pCalDigi; // use reference to avoid -> syntax

    //-- XtalId --//
    CalXtalId id(calDigi.getPackedId()); // get interaction information
    // skip hits not for current tower.
    TwrNum twr = id.getTower();
    if ((int)twr != m_cfg.twrBay) continue; 
    
    ColNum col = id.getColumn();
    LyrNum lyr = id.getLayer();

    tXtalIdx xtalIdx(lyr,col);

    // check that we are in 4-range readout mode
    int nRO = calDigi.getNumReadouts();
    if (nRO != 4) {
      ostringstream tmp;
      tmp << __FILE__  << ":"     << __LINE__ << " " 
          << "Event# " << m_evtId << "Not in 4-range readout mode";
      throw tmp.str();
    }

    // load up all adc values for each xtal diode
    // also ped subtraced adc values.
    for (XtalDiode xDiode; xDiode.isValid(); xDiode++) {
      DiodeNum  diode  = xDiode.getDiode();
      RngNum    rng    = diode.getX8Rng();  // we are only interested in x8 range adc vals for muon calib
      FaceNum   face   = xDiode.getFace();
      tRngIdx   rngIdx  (xtalIdx, face, rng);
      tDiodeIdx diodeIdx(xtalIdx, face, diode);

      float adc = calDigi.getAdcSelectedRange(rng, (CalXtalId::XtalFace)(unsigned short)face); // raw adc
      if (adc < 0) {
        m_ostrm << "Couldn't get adc val for face=" << face 
                << " rng=" << rng 
                << " evt=" << m_evtId << endl;
        continue;
      }
      hs.adc_ped[diodeIdx] = adc - m_calPed[rngIdx];// subtract pedestals
    }
    
    // DO WE HAVE A HIT? (sum up both ends LEX8 i.e LRG_DIODE)
    if (hs.adc_ped[tDiodeIdx(xtalIdx,POS_FACE,LRG_DIODE)] +
        hs.adc_ped[tDiodeIdx(xtalIdx,NEG_FACE,LRG_DIODE)] > m_cfg.hitThresh) {

      hs.count++; // increment total # hits

      //m_ostrm << "nHits " << hs.count;// << endl;

      // used to determine if xtal is x or y layer
      if (lyr.getDir() == X_DIR) { // X layer
        short xLyr = lyr.getXLyr();
        hs.perLyrX[xLyr]++;
        hs.perColX[col]++;
        hs.hitListX.push_back(xtalIdx);
      } else { // y layer
        short yLyr = lyr.getYLyr();
        hs.perLyrY[yLyr]++;
        hs.perColY[col]++;
        hs.hitListY.push_back(xtalIdx);
      }
    }
  }

  // we're done if there were no hits
  if (hs.count) {
    // POST-PROCESS: after we have registered all hits, summarize them all
    // find highest hits per layer
    hs.maxPerLyrX = *(max_element(hs.perLyrX.begin(),
                                  hs.perLyrX.end()));
    hs.maxPerLyrY = *(max_element(hs.perLyrY.begin(),
                                  hs.perLyrY.end()));
    hs.maxPerLyr = max(hs.maxPerLyrX, hs.maxPerLyrY);


    // count all layers w/ count > 0
    hs.nLyrsX = count_if(hs.perLyrX.begin(), hs.perLyrX.end(),
                         bind2nd(greater<short>(), 0));
    hs.nLyrsY = count_if(hs.perLyrY.begin(), hs.perLyrY.end(),
                         bind2nd(greater<short>(), 0));

    // count all cols w/ count > 0
    hs.nColsX = count_if(hs.perColX.begin(), hs.perColX.end(),
                         bind2nd(greater<short>(), 0));
    hs.nColsY = count_if(hs.perColY.begin(), hs.perColY.end(),
                         bind2nd(greater<short>(), 0));

    // find 1st col hit in each direction (will be only col hit if evt is good)
    // find_if returns iterator to 1st true element... 
    // ...subtract begin() iterator to get distance or index
    hs.firstColX = find_if(hs.perColX.begin(), hs.perColX.end(), 
                           bind2nd(greater<short>(), 0)) - hs.perColX.begin();
    hs.firstColY = find_if(hs.perColY.begin(), hs.perColY.end(), 
                           bind2nd(greater<short>(), 0)) - hs.perColY.begin();

    // EVENT SELECTION:

    // don't want ANY layer w/ >= 3 hits
    if (hs.maxPerLyr > 2) return;

    // X has Vertical Connect-4 && >0 Y hits
    if (hs.nLyrsX == 4 &&
        hs.nColsX == 1 &&
        hs.nLyrsY > 0)
      hs.goodXTrack = true;

    // Y has Vertical Connect-4 && >0 X hits
    if (hs.nLyrsY == 4 &&
        hs.nColsY == 1 &&
        hs.nLyrsX > 0)
      hs.goodYTrack = true;
  }

  // all summary data now filled in
  hs.status = true;
}

void MuonCalib::initAsymHists() {
  m_asymHists.resize(AsymType::N_VALS);

  // fill in min & max histogram levels.
  CalArray<AsymType, float> asymMin;
  CalArray<AsymType, float> asymMax;

  asymMin[ASYM_LL] = -2;
  asymMax[ASYM_LL] = 2;
  asymMin[ASYM_LS] = -1;
  asymMax[ASYM_LS] = 7;
  asymMin[ASYM_SL] = -7;
  asymMax[ASYM_SL] = 1;
  asymMin[ASYM_SS] = -2;
  asymMax[ASYM_SS] = 2;
    
  for (AsymType asymType; asymType.isValid(); asymType++)
    // PER XTAL LOOP
    for (tXtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
      ostringstream tmp;
      tmp <<  "asym" + asymType.getMnem() + "_" << xtalIdx;
      // columns are #'d 0-11, hist contains 1-10. 
      // .5 & 10.5 limit puts 1-10 at center of bins
      m_asymHists[asymType][xtalIdx] = new TH2S(tmp.str().c_str(),
                                                tmp.str().c_str(),
                                                N_ASYM_PTS, 
                                                .5, 
                                                10.5,
                                                (int)(100*(asymMax[asymType] -
                                                           asymMin[asymType])),
                                                asymMin[asymType],
                                                asymMax[asymType]);
      
    } 
}

/** 
    \note 'test direction' refers to the direction w/ 4 vertical hits.  
    In asymetry calibration this direction is used to get position info 
    for the 'orthogonal direction'

    \note 'orthogonal direction' refers to the direction with the hits that 
    are actually measured for asymetry.
*/

void MuonCalib::fillAsymHists(int nEvts) {
  initAsymHists();

  int lastEvt = chkForEvts(nEvts);
  HitSummary hs;

  int  nGoodDirs  = 0; // count total # of events used
  int  nXDirs     = 0;
  int  nYDirs     = 0;
  long nHits      = 0; // count total # of xtals measured
  int  nBadHits   = 0;
  int  nBadDACs   = 0;

  // Basic digi-event loop
  for (Int_t iEvt = m_startEvt; iEvt < lastEvt; iEvt++) {
    if (!getEvent(iEvt)) {
      m_ostrm << "Warning, event " << iEvt << " not read." << endl;
      continue;
    }

    summarizeHits(hs);
    
    for (DirNum dir; dir.isValid(); dir++) {
      int pos;
      vector<tXtalIdx> *pHitList, *pHitListOrtho;

      // DIRECTION SPECIFIC SETUP //
      if (dir == X_DIR) {
        if (!hs.goodXTrack) continue;  // skip this direction if track is bad
        pos = hs.firstColX;
        pHitList = &hs.hitListX; // hit list in test direction 
        pHitListOrtho = &hs.hitListY; // ortho direction
        nXDirs++;
      } else { // Y_DIR
        if (!hs.goodYTrack) continue; // skip this direction if track is bad
        pos = hs.firstColY;
        pHitList = &hs.hitListY; // hit list in test direction 
        pHitListOrtho = &hs.hitListX; // ortho direction
        nYDirs++;
      }

      // skip extreme ends of xtal, as variance is high.
      if (pos == 0 || pos == 11) continue;
      nGoodDirs++;

      // use references to avoid -> notation
      vector<tXtalIdx> &hitListOrtho = *pHitListOrtho; 
      
      // loop through each orthogonal hit
      for (unsigned i = 0; i < hitListOrtho.size(); i++) {
        tXtalIdx xtalIdx = hitListOrtho[i];
        
        CalArray<XtalDiode, float> dac;
        bool badDAC = false;
        for (XtalDiode xDiode; xDiode.isValid(); xDiode++) {
          tDiodeIdx diodeIdx(xtalIdx, xDiode);
          dac[xDiode] = adc2dac(diodeIdx, hs.adc_ped[diodeIdx]);

          // can't take log so we must quit here.
          if (dac[xDiode] <= 0) badDAC = true;
        }

        if (badDAC) {
          nBadHits++;
          nBadDACs++;
          continue;
        }
        
        // calcuate the 4 log ratios = log(POS/NEG)
        for (AsymType asymType; asymType.isValid(); asymType++) {
          float asym = log(dac[XtalDiode(POS_FACE, asymType.getDiode(POS_FACE))] /
                           dac[XtalDiode(NEG_FACE, asymType.getDiode(NEG_FACE))]);
          m_asymHists[asymType][xtalIdx]->Fill(pos, asym);
        }

        nHits++;
      } // per hit loop
    } // per direction loop
  } // per event loop
  
  m_ostrm << "Asymmetry histograms filled nEvents=" << nGoodDirs
          << " nXDirs="               << nXDirs
          << " nYDirs="               << nYDirs << endl;
  m_ostrm << " nHits measured="       << nHits
          << " Bad hits="             << nBadHits
          << endl;
}

void MuonCalib::fitAsymHists() {
  m_calAsym.resize(AsymType::N_VALS);
  m_calAsymErr.resize(AsymType::N_VALS);

  for (AsymType asymType; asymType.isValid(); asymType++) {
    
    // PER XTAL LOOP
    for (tXtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
      m_calAsym[asymType][xtalIdx].resize(N_ASYM_PTS);
      m_calAsymErr[asymType][xtalIdx].resize(N_ASYM_PTS);
      // loop through all N_ASYM_PTS bins in asymmetry profile
      for (int i = 0; i < N_ASYM_PTS; i++) {
        // get slice of 2D histogram for each X bin
        // HISTOGRAM BINS START AT 1 NOT ZERO! (hence 'i+1') 
        TH1D &slice = *(m_asymHists[asymType][xtalIdx]->ProjectionY("slice", i+1,i+1));

        // point local references to output values
        float &av = m_calAsym[asymType][xtalIdx][i];
      
        float &rms = m_calAsymErr[asymType][xtalIdx][i];

        // trim outliers - 3 times cut out anything outside 3 sigma
        for (short iter = 0; iter < 3; iter++) {
          // get current mean & RMS
          av  = slice.GetMean(); rms = slice.GetRMS();
        
          // trim new histogram limits
          slice.SetAxisRange(av - 3*rms, av + 3*rms);
        }

        // update new mean & sigma
        av = slice.GetMean(); rms = slice.GetRMS();

        // evidently ROOT doesn't like reusing the slice 
        // histograms as much as they claim they do.
        slice.Delete();
      }
    }
  }
}

float MuonCalib::adc2dac(tDiodeIdx diodeIdx, float adc) const {
  return m_inlSplines[diodeIdx]->Eval(adc);
}

float MuonCalib::dac2adc(tDiodeIdx diodeIdx, float dac) const {
  return m_inlSplinesInv[diodeIdx]->Eval(dac);
}

void MuonCalib::writeAsymTXT(const string &filename) const {
   
  ofstream outfile(filename.c_str());

  if (!outfile.is_open())
    throw string("Unable to open " + filename);

  // PER XTAL LOOP
  for (tXtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
    LyrNum lyr = xtalIdx.getLyr();
    ColNum col = xtalIdx.getCol();
    for (AsymType asymType; asymType.isValid(); asymType++) {
      // per point along curve
      for (int i = 0; i < N_ASYM_PTS; i++) {
        outfile << m_cfg.twrBay 
                << " " << lyr 
                << " " << col
                << " " << asymType.getDiode(POS_FACE)
                << " " << asymType.getDiode(NEG_FACE)
                << " " << m_calAsym[asymType][xtalIdx][i]
                << " " << m_calAsymErr[asymType][xtalIdx][i]
                << endl;
      }
    }
  }
}

void MuonCalib::readAsymTXT(const string &filename) {
  m_calAsym.clear();
  m_calAsym.clear();
  m_calAsym.resize(AsymType::N_VALS);
  m_calAsymErr.resize(AsymType::N_VALS);

  short twr, lyr, col, pdiode, ndiode;
  float asym, sig;
  unsigned nRead = 0;

  // open file
  ifstream infile(filename.c_str());
  if (!infile.is_open())
    throw string("Unable to open " + filename);

  // loop through each line in file
  while (infile.good()) {
    // get lyr, col (xtalId)
    infile >> twr >> lyr >> col >> pdiode >> ndiode >> asym >> sig;
    
    if (infile.fail()) break; // bad get()

    // skip if not current tower
    if (twr != m_cfg.twrBay) continue; 

    tXtalIdx xtalIdx(lyr,col);
    AsymType asymType(pdiode, ndiode);

    m_calAsym[asymType][xtalIdx].push_back(asym);
    m_calAsymErr[asymType][xtalIdx].push_back(sig);

    nRead++;
  }

  // make sure we read in correct @ of asym vals
  static const unsigned TOTAL_N_VALS = 
    AsymType::N_VALS*tXtalIdx::N_VALS*N_ASYM_PTS;

  if (nRead != TOTAL_N_VALS) {
    ostringstream tmp;
    tmp << __FILE__ << ":" << __LINE__ << " " 
        << "File '" << filename << "' is incomplete: " << nRead
        << " vals read, "       << TOTAL_N_VALS << " vals expected.";
    throw tmp.str();
  }
}

void MuonCalib::loadA2PSplines() {
  m_asym2PosSplines.resize(tXtalIdx::N_VALS);

  // create position (Y-axis) array
  // linearly extrapolate for 1st and last points (+2 points)
  double pos[N_ASYM_PTS+2];
  for (int i = 0; i < N_ASYM_PTS+2; i++) 
    pos[i] = i + 0.5; // (center of the column)
  double asym[N_ASYM_PTS+2];

  // PER XTAL LOOP
  for (tXtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
    // copy asym vector into middle of array
    vector<float> &asymVec = m_calAsym[ASYM_LL][xtalIdx];
    copy(asymVec.begin(), asymVec.end(), asym+1);

    // extrapolate 1st & last points
    asym[0] = 2*asym[1] - asym[2];
    asym[N_ASYM_PTS+1] = 2*asym[N_ASYM_PTS]-asym[N_ASYM_PTS-1];

    //generate splinename
    ostringstream name;
    name << "asym2pos_" << xtalIdx;

    // create spline object
    TSpline3 *mySpline= new TSpline3(name.str().c_str(), 
                                     asym, pos, N_ASYM_PTS+2);
    mySpline->SetName(name.str().c_str());
    m_asym2PosSplines[xtalIdx] = mySpline;
  }
}

float MuonCalib::asym2pos(tXtalIdx xtalIdx, float asym) const {
  return m_asym2PosSplines[xtalIdx]->Eval(asym);
}

void MuonCalib::initMPDHists() {
  m_dacLLHists.resize(tXtalIdx::N_VALS);
  m_dacL2SHists.resize(tXtalIdx::N_VALS);
  m_dacL2SSlopeProfs.resize(tXtalIdx::N_VALS);

  for (tXtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {

    {
      // LRG-LRG DAC
      ostringstream tmp;
      tmp << "dacLL_" << xtalIdx;
      m_dacLLHists[xtalIdx] = new TH1S(tmp.str().c_str(),
                                       tmp.str().c_str(),
                                       200,0,100);
    }

    {
      // DAC L2S RATIO
      ostringstream tmp;
      tmp << "dacL2S_" << xtalIdx;
      m_dacL2SHists[xtalIdx] = new TH1S(tmp.str().c_str(),
                                        tmp.str().c_str(),
                                        400, 0, .4);
    }

    {
      // DAC L2S SLOPE
      ostringstream tmp;
      tmp << "dacL2S_slope_" << xtalIdx;
      m_dacL2SSlopeProfs[xtalIdx] = new TProfile(tmp.str().c_str(),
                                                 tmp.str().c_str(),
                                                 N_L2S_PTS,
                                                 L2S_MIN_LEDAC,
                                                 L2S_MAX_LEDAC);
    }

  }
}

void MuonCalib::fillMPDHists(int nEvts) {
  initMPDHists();

  int lastEvt = chkForEvts(nEvts);
  HitSummary hs;

  ////////////////////////////////////////////////////
  // INITIALIZE ROOT PLOTTING OBJECTS FOR LINE FITS //
  ////////////////////////////////////////////////////
  // viewHist is used to set scale before drawing TGraph
  TH2S viewHist("viewHist","viewHist",
                8,-0.5,7.5, //X-limits lyr
                12,-0.5,11.5); //Y-limits col
  TCanvas canvas("canvas","event display",800,600);
  viewHist.Draw();
  TGraph graph(4); graph.Draw("*");
  canvas.Update();
  TF1 lineFunc("line","pol1",0,8);

  // GENERATE ASYM2POS SPLINES //
  loadA2PSplines();

  // SUMMARY COUNTERS //
  int nXEvts    = 0; // count total # of X events used
  int nYEvts    = 0; // count total # of Y events used
  long xtalIdxs = 0; // count total # of xtals measured

  // NUMERIC CONSTANTS
  // converts between lyr/col units & mm
  // real trig is needed for pathlength calculation
  float slopeFactor = m_cfg.cellHorPitch/m_cfg.cellVertPitch;

  // Basic digi-event loop
  for (Int_t iEvt = m_startEvt; iEvt < lastEvt; iEvt++) {
    if (!getEvent(iEvt)) {
      m_ostrm << "Warning, event " << iEvt << " not read." << endl;
      continue;
    }

    summarizeHits(hs);

    // CHECK BOTH DIRECTIONS FOR USABLE EVENT
    for (DirNum dir; dir.isValid(); dir++) {

      // skip if we don't have a good track
      if (dir == X_DIR && !hs.goodXTrack) continue;
      else if (dir == Y_DIR && !hs.goodYTrack) continue;

      if (dir == X_DIR) nXEvts++;
      else nYEvts++;

      // set up proper hitLists for processing based on direction
      vector<tXtalIdx> &hitList = // hit list in current direction for gain calib
        (dir == X_DIR) ? hs.hitListX : hs.hitListY;
      vector<tXtalIdx> &hitListOrtho = // hit list in orthogonal direction
        (dir == X_DIR) ? hs.hitListY : hs.hitListX;

      // need at least 2 points to get a orthogonal track
      if (hitListOrtho.size() < 2) continue;

      graph.Set(hitListOrtho.size());

      // fill in each point val
      for (unsigned i = 0; i < hitListOrtho.size(); i++) {
        tXtalIdx xtalIdx = hitListOrtho[i];
        LyrNum lyr = xtalIdx.getLyr();
        ColNum col = xtalIdx.getCol();
        graph.SetPoint(i,lyr,col);
      }

      // fit straight line through graph
      graph.Fit(&lineFunc,"WQN");

      // throw out events which are greater than about 30 deg from vertical
      float lineSlope = lineFunc.GetParameter(1);
      if (abs(lineSlope) > 0.5) continue;

      // loop through each hit in X direction, remove bad xtals
      // bad xtals have energy centroid at col 0 or 11 (-5 or +5 since center of xtal is 0)
      for (unsigned i = 0; i < hitList.size(); i++) {
        tXtalIdx xtalIdx = hitList[i];
        LyrNum lyr = xtalIdx.getLyr();

        float hitPos = lineFunc.Eval(lyr); // find column for given lyr

        //throw out event if energy centroid is in column 0 or 11 (3cm from end)
        if (hitPos < 1 || hitPos > 10) {
          hitList.erase(hitList.begin()+i);
          i--;
        }
      }

      // occasionally there will be no good hits!
      if (hitList.size() < 1) continue;

      vector<CalArray<XtalDiode,float> > dacs(hitList.size());

      // now that we have eliminated events on the ends of xtals, we can use
      // asymmetry to get a higher precision slope
      // we'll keep the dac vals while we're at it since some are used
      // more than once
      graph.Set(hitList.size());
      for (unsigned i = 0; i < hitList.size(); i++) {
        tXtalIdx xtalIdx = hitList[i];
        LyrNum lyr = xtalIdx.getLyr();

        for (XtalDiode xDiode; xDiode.isValid(); xDiode++) {
          tDiodeIdx diodeIdx(xtalIdx, xDiode);
          // calculate DAC vals
          dacs[i][xDiode] = adc2dac(diodeIdx, hs.adc_ped[diodeIdx]);
        }
        
        // calcuate the log ratio = log(POS/NEG)
        float asymLL = log(dacs[i][XtalDiode(POS_FACE,LRG_DIODE)]
                           / dacs[i][XtalDiode(NEG_FACE,LRG_DIODE)]);

        // get new position from asym
        float hitPos = asym2pos(xtalIdx,asymLL);

        graph.SetPoint(i,lyr,hitPos);
      }

      // fit straight line through graph
      graph.Fit(&lineFunc,"WQN");
      lineSlope = lineFunc.GetParameter(1);

      //slope = rise/run = dy/dx = colPos/lyrNum
      float tan = lineSlope*slopeFactor;
      float sec = sqrt(1 + tan*tan); //sec proportional to hyp which is pathlen.

      // poulate histograms & apply pathlen correction
      int nHits = hitList.size();
      for (int i = 0; i < nHits; i++) {
        // calculate dacs
        tXtalIdx xtalIdx = hitList[i];

        // apply pathlength correction
        for (XtalDiode xDiode; xDiode.isValid(); xDiode++)
          dacs[i][xDiode] /= sec;

        float meanDACLrg = sqrt(dacs[i][XtalDiode(POS_FACE,LRG_DIODE)]
                                * dacs[i][XtalDiode(NEG_FACE,LRG_DIODE)]);

        float meanDACSm = sqrt(dacs[i][XtalDiode(POS_FACE, SM_DIODE)]
                               * dacs[i][XtalDiode(NEG_FACE, SM_DIODE)]);
        
        // Load meanDAC Histogram
        m_dacLLHists[xtalIdx]->Fill(meanDACLrg);

        // load dacL2S profile
        m_dacL2SHists[xtalIdx]->Fill(meanDACSm/meanDACLrg);
        // load dacL2S profile
        m_dacL2SSlopeProfs[xtalIdx]->Fill(meanDACLrg,meanDACSm);


        xtalIdxs++;
      } // populate histograms
    } // direction loop
  } // event loop

  m_ostrm << "MPD histograms filled nXEvts=" << nXEvts 
          << " nYEvents=" << nYEvts 
          << " nXtals measured " << xtalIdxs << endl;
}

void MuonCalib::fitMPDHists() {
  m_calMPD.resize(DiodeNum::N_VALS);
  m_calMPDErr.resize(DiodeNum::N_VALS);
  m_adc2nrg.resize(tDiodeIdx::N_VALS);

  ////////////////////////////////////////////////////
  // INITIALIZE ROOT PLOTTING OBJECTS FOR LINE FITS //
  ////////////////////////////////////////////////////
  // viewHist is used to set scale before drawing TGraph
  TH2S viewHist("viewHist","viewHist",
                N_L2S_PTS, 0, L2S_MAX_LEDAC, // X-LIMITS LRG
                N_L2S_PTS, 0, L2S_MAX_LEDAC); // Y-LIMITS SM
  TCanvas canvas("canvas","event display",800,600);
  viewHist.Draw();
  TGraph graph(N_L2S_PTS); 
  graph.Draw("*");
  canvas.Update();
  TF1 lineFunc("line","pol1", 
               L2S_MIN_LEDAC, 
               L2S_MAX_LEDAC);

  // PER XTAL LOOP
  for (tXtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
    // retrieve Lrg diode DAC histogram
    TH1S& histLL = *m_dacLLHists[xtalIdx];

    ///////////////////////////////////
    //-- MeV Per Dac (Lrg Diode) --//
    ///////////////////////////////////
    
    // LANDAU fit for muon peak (limit outliers by n*err)
    float ave = histLL.GetMean();
    float err = histLL.GetRMS();
    histLL.Fit("landau", "Q", "", ave-2*err, ave+3*err);
    float mean  = (histLL.GetFunction("landau"))->GetParameter(1);
    float sigma = (histLL.GetFunction("landau"))->GetParameter(2);

    m_calMPD[LRG_DIODE][xtalIdx] = 11.2/mean;

    // keep sigma proportional to extrapolated val
    m_calMPDErr[LRG_DIODE][xtalIdx] = 
      m_calMPD[LRG_DIODE][xtalIdx] * sigma/mean; 

    
    ////////////////////
    //-- (Sm Diode) --//
    ////////////////////
        
    // LRG 2 SM Ratio
    TH1S &histL2S = *m_dacL2SHists[xtalIdx]; // get profile

    // trim outliers - 3 times cut out anything outside 3 sigma
    for (short iter = 0; iter < 3; iter++) {
      // get current mean & RMS
      float av  = histL2S.GetMean(); 
      float rms = histL2S.GetRMS();
      
      // trim new histogram limits
      histL2S.SetAxisRange(av - 3*rms, av + 3*rms);
    }

    // fit straight line to get mean ratio
    histL2S.Fit("gaus","Q");
    // mean ratio of smDac/lrgDac
    float sm2lrg = ((TF1&)*histL2S.GetFunction("gaus")).GetParameter(1);
    float s2lsig = ((TF1&)*histL2S.GetFunction("gaus")).GetParameter(2);
    
    //-- NOTES:
    // MPDLrg     = MeV/LrgDAC
    // sm2lrg  = SmDAC/LrgDAC
    // MPDSm     = MeV/SmDAC = (MeV/LrgDAC)*(LrgDAC/SmDAC) 
    //              = MPDLrg/sm2lrg
    
    m_calMPD[SM_DIODE][xtalIdx] = m_calMPD[LRG_DIODE][xtalIdx]/sm2lrg;
    
    //-- Propogate errors
    // in order to combine slope & MPD error for final error
    // I need the relative error for both values - so sayeth sasha
    float relLineErr = s2lsig/sm2lrg;
    float relMPDErr  = m_calMPDErr[LRG_DIODE][xtalIdx]/m_calMPD[LRG_DIODE][xtalIdx];

    m_calMPDErr[SM_DIODE][xtalIdx] = m_calMPD[SM_DIODE][xtalIdx]*
      sqrt(relLineErr*relLineErr + relMPDErr*relMPDErr);


    ////////////////////
    //-- L2S Slope  --//
    ////////////////////

    // LRG 2 SM Ratio
    TProfile& p = *m_dacL2SSlopeProfs[xtalIdx]; // get profile

    // Fill scatter graph w/ smDAC vs lrgDAC points
    int nPts = 0;
    graph.Set(nPts); // start w/ empty graph
    for (int i = 0; i < N_L2S_PTS; i++) {
      // only insert a bin if it has entries
      if (!(p.GetBinEntries(i+1) > 0)) continue; // bins #'d from 1
      nPts++;

      // retrieve sm & lrg dac vals
      float smDAC = p.GetBinContent(i+1);
      float lrgDAC = p.GetBinCenter(i+1);

      // update graphsize & set point
      graph.Set(nPts);
      graph.SetPoint(nPts-1,lrgDAC,smDAC);
    }

    // bail if for some reason we didn't get any points
    if (!nPts) {
      cout << __FILE__  << ":"     << __LINE__ << " "
           << "Event# " << m_evtId
           << "Unable to find sm diode MPD for xtal=" << xtalIdx
           << " due to empty histogram." << endl;
      continue;
    }

    // fit straight line to get mean ratio
    graph.Fit(&lineFunc,"WQN");
  }
}

void MuonCalib::writeMPDTXT(const string &filename) const {
  ofstream outfile(filename.c_str());
  if (!outfile.is_open())
    throw string("Unable to open " + filename);

  for (tXtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++)
    for (DiodeNum diode; diode.isValid(); diode++) {
      LyrNum lyr = xtalIdx.getLyr();
      ColNum col = xtalIdx.getCol();
      outfile << m_cfg.twrBay
              << " " << lyr
              << " " << col
              << " " << diode
              << " " << m_calMPD[diode][xtalIdx]
              << " " << m_calMPDErr[diode][xtalIdx]
              << endl;
    }
}

void MuonCalib::writePedsXML(const string &filename, const string &dtdPath) const {
  ofstream outfile(filename.c_str());
  ifstream dtdFile(dtdPath.c_str());

  if (!outfile.is_open())
    throw string("Unable to open " + filename);
  if (!dtdFile.is_open())
    throw string("Unable to open " + dtdPath);

  outfile << "<?xml version=\"1.0\"?>" << endl;
  
  // INSERT ENTIRE .DTD FILE //
  outfile << "<!DOCTYPE calCalib [";
  string tmp;
  while (dtdFile.good()) {
    getline(dtdFile, tmp);
    if (dtdFile.fail()) continue; // bad get
    outfile << tmp << endl;
  }
  outfile << "]>" << endl;

  outfile << "<calCalib>" << endl;
  outfile << " <generic instrument=\"" << m_cfg.instrument 
          << "\" timestamp=\"" << m_cfg.timestamp << "\"";
    
  outfile << " calibType=\"CAL_Ped\" fmtVersion=\"v2r2\" >" << endl;
  outfile << " </generic>" << endl;
  outfile << " <dimension nRow=\"" << TwrNum::N_ROWS 
          << "\" nCol=\""          << TwrNum::N_COLS 
          << "\" nLayer=\""        << LyrNum::N_VALS 
          << "\" nXtal=\""         << ColNum::N_VALS 
          << "\" nFace=\""         << FaceNum::N_VALS 
          << "\" nRange=\""        << RngNum::N_VALS 
          << "\"/>" << endl;
  
  // currently only loop through one tower.
  for (TwrNum twr=m_cfg.twrBay; (int)twr == m_cfg.twrBay; twr++) {
    outfile << " <tower iRow=\"" << twr.getRow() 
            << "\" iCol=\"" << twr.getCol() << "\">" << endl;

    for (LyrNum lyr; lyr.isValid(); lyr++) {
      outfile << "  <layer iLayer=\"" << lyr << "\">" << endl;
      for (ColNum col; col.isValid(); col++) {
        outfile << "   <xtal iXtal=\"" << col << "\">" << endl;
        for (FaceNum face; face.isValid(); face++) {
          outfile << "    <face end=\"" << face.getMnem() << "\">" << endl;

          for (RngNum rng; rng.isValid(); rng++) {
            tRngIdx rngIdx(lyr,col,face,rng);

            float av = m_calPed[rngIdx];
            float err= m_calPedErr[rngIdx];

            outfile << "     <calPed avg=\"" << fixed << av
                    << "\" sig=\""           << fixed << err
                    << "\" range=\""         << rng.getMnem() << "\"/>"
                    << endl;
          }
          outfile << "    </face>" << endl;
        }
        outfile << "   </xtal>" << endl;
      }
      outfile<< "  </layer>" << endl;
    }
    outfile << " </tower>" << endl;
  }
  outfile << "</calCalib>" << endl;
}

void MuonCalib::writeAsymXML(const string &filename, const string &dtdPath) const {
  ofstream outfile(filename.c_str());
  ifstream dtdFile(dtdPath.c_str());

  if (!outfile.is_open())
    throw string("Unable to open " + filename);
  if (!dtdFile.is_open())
    throw string("Unable to open " + dtdPath);
  outfile << "<?xml version=\"1.0\"?>" << endl;
  
  // INSERT ENTIRE .DTD FILE //
  outfile << "<!DOCTYPE calCalib [";
  string tmp;
  while (dtdFile.good()) {
    getline(dtdFile, tmp);
    if (dtdFile.fail()) continue; // bad get
    outfile << tmp << endl;
  }
  outfile << "]>" << endl;

  outfile << "<calCalib>" << endl;
  outfile << " <generic instrument=\"" << m_cfg.instrument 
          << "\" timestamp=\"" << m_cfg.timestamp << "\"";
  
  outfile << " calibType=\"CAL_Asym\" fmtVersion=\"v2r2\" >" << endl;
  outfile << " </generic>"  << endl;
  outfile << " <dimension nRow=\"" << TwrNum::N_ROWS
          << "\" nCol=\""          << TwrNum::N_COLS
          << "\" nLayer=\""        << LyrNum::N_VALS 
          << "\" nXtal=\""         << ColNum::N_VALS 
          << "\" nFace=\""         << 1
          << "\" nRange=\""        << 1
          << "\" nXpos=\""         << 1
          << "\"/>" << endl;
  
  // -- GENERATE xpos VALUES -- //
  outfile << " <xpos values=\"";
  for (int i = 0; i < N_ASYM_PTS; i++) 
    outfile << xtalCenterPos(i+1) << " ";
  outfile << "\"/>" << endl;
  
  // -- OUTPUT ASYMETRY DATA -- //
  // currently only loop through one tower.
  for (TwrNum twr=m_cfg.twrBay; (int)twr == m_cfg.twrBay; twr++) {
    outfile << " <tower iRow=\"" << twr.getRow() 
            << "\" iCol=\"" << twr.getCol() << "\">" << endl;

    for (LyrNum lyr; lyr.isValid(); lyr++) {
      outfile << "  <layer iLayer=\"" << lyr << "\">" << endl;
      for (ColNum col; col.isValid(); col++) {
        outfile << "   <xtal iXtal=\"" << col << "\">" << endl;
        outfile << "    <face end=\"NA\">" << endl;
       
        tXtalIdx xtalIdx(lyr,col);
        outfile << "     <asym " << endl;
        // ASYM LL
        outfile << "           bigVals=\"";
        for (int i = 0; i < N_ASYM_PTS; i++)
          outfile << " " << fixed << m_calAsym[ASYM_LL][xtalIdx][i];
        outfile << "\"" << endl;
        // ASYM LL Err
        outfile << "           bigSigs=\"";
        for (int i = 0; i < N_ASYM_PTS; i++)
          outfile << " " << fixed << m_calAsymErr[ASYM_LL][xtalIdx][i];
        outfile << "\"" << endl;

        // ASYM LS
        outfile << "           NsmallPbigVals=\"";
        for (int i = 0; i < N_ASYM_PTS; i++)
          outfile << " " << fixed << m_calAsym[ASYM_LS][xtalIdx][i];
        outfile << "\"" << endl;
        // ASYM LS Err
        outfile << "           NsmallPbigSigs=\"";
        for (int i = 0; i < N_ASYM_PTS; i++)
          outfile << " " << fixed << m_calAsymErr[ASYM_LS][xtalIdx][i];
        outfile << "\"" << endl;

        // ASYM SL
        outfile << "           PsmallNbigVals=\"";
        for (int i = 0; i < N_ASYM_PTS; i++)
          outfile << " " << fixed << m_calAsym[ASYM_SL][xtalIdx][i];
        outfile << "\"" << endl;
        // ASYM SL Err
        outfile << "           PsmallNbigSigs=\"";
        for (int i = 0; i < N_ASYM_PTS; i++)
          outfile << " " << fixed << m_calAsymErr[ASYM_SL][xtalIdx][i];
        outfile << "\"" << endl;

        // ASYM SS
        outfile << "           smallVals=\"";
        for (int i = 0; i < N_ASYM_PTS; i++)
          outfile << " " << fixed << m_calAsym[ASYM_SS][xtalIdx][i];
        outfile << "\"" << endl;
        // ASYM SS Err
        outfile << "           smallSigs=\"";
        for (int i = 0; i < N_ASYM_PTS; i++)
          outfile << " " << fixed << m_calAsymErr[ASYM_SS][xtalIdx][i];
        outfile << "\" />" << endl;

        outfile << "    </face>" << endl;
        outfile << "   </xtal>" << endl;
      }
      outfile<< "  </layer>" << endl;
    }
    outfile << " </tower>" << endl;
  }
  outfile << "</calCalib>" << endl;
}

void MuonCalib::writeMPDXML(const string &filename, const string &dtdPath) const {
  ofstream outfile(filename.c_str());
  ifstream dtdFile(dtdPath.c_str());

  if (!outfile.is_open())
    throw string("Unable to open " + filename);
  if (!dtdFile.is_open())
    throw string("Unable to open " + dtdPath);

  outfile << "<?xml version=\"1.0\"?>" << endl;
  
  // INSERT ENTIRE .DTD FILE //
  outfile << "<!DOCTYPE calCalib [";
  string tmp;
  while (dtdFile.good()) {
    getline(dtdFile, tmp);
    if (dtdFile.fail()) continue; // bat get()
    outfile << tmp << endl;;
  }
  outfile << "]>" << endl;

  outfile << "<calCalib>" << endl;
  outfile << " <generic instrument=\"" << m_cfg.instrument 
          << "\" timestamp=\"" << m_cfg.timestamp << "\"";
  outfile << " calibType=\"CAL_MevPerDac\" fmtVersion=\"v2r2\" >" << endl;
  outfile << " </generic>" << endl;
  outfile << " <dimension nRow=\"" << TwrNum::N_ROWS
          << "\" nCol=\""          << TwrNum::N_COLS
          << "\" nLayer=\""        << LyrNum::N_VALS 
          << "\" nXtal=\""  << ColNum::N_VALS 
          << "\" nFace=\""  << 1 
          << "\" nRange=\"" << 1 
          << "\" nXpos=\""  << 1 
          << "\"/>" << endl;

  // currently only loop through one tower.
  for (TwrNum twr=m_cfg.twrBay; (int)twr == m_cfg.twrBay; twr++) {
    outfile << " <tower iRow=\"" << twr.getRow() 
            << "\" iCol=\"" << twr.getCol() << "\">" << endl;

    for (LyrNum lyr; lyr.isValid(); lyr++) {
      outfile << "  <layer iLayer=\"" << lyr << "\">" << endl;
      for (ColNum col; col.isValid(); col++) {
        tXtalIdx xtalIdx(lyr,col);
        outfile << "   <xtal iXtal=\"" << col << "\">" << endl;
        outfile << "    <face end=\"" << "NA" << "\">" << endl;

        outfile << "     <mevPerDac bigVal=\"" << fixed 
                << m_calMPD[LRG_DIODE][xtalIdx] 
                << "\" bigSig=\""   << fixed << m_calMPDErr[LRG_DIODE][xtalIdx]
                << "\" smallVal=\"" << fixed << m_calMPD[SM_DIODE][xtalIdx]
                << "\" smallSig=\"" << fixed << m_calMPDErr[SM_DIODE][xtalIdx]
                << "\">" << endl;
        outfile << "      <bigSmall end=\"POS\" bigSmallRatioVals=\"0\" bigSmallRatioSigs=\"0\"/>" << endl;
        outfile << "      <bigSmall end=\"NEG\" bigSmallRatioVals=\"0\" bigSmallRatioSigs=\"0\"/>" << endl;
        outfile << "     </mevPerDac>" << endl;

        outfile << "    </face>" << endl;
        outfile << "   </xtal>" << endl;
      }
      outfile<< "  </layer>" << endl;
    }
    outfile << " </tower>" << endl;
  }
  outfile << "</calCalib>" << endl;
}

void MuonCalib::writeADC2NRGXML(const string &filename) {
  ofstream outfile(filename.c_str());

  const float enemu = (float)11.2;

  static const int ic = N_ASYM_PTS/2;
  for (tXtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
    for (DiodeNum diode; diode.isValid(); diode++) {
      float mpd = m_calMPD[diode][xtalIdx];

      // 0.25 would normally be 0.5, but it is applied equally to both sides
      // so we split it in half.
      float asym_ctr = 0.25*(m_calAsym[AsymType(diode,diode)][xtalIdx][ic]
                             + m_calAsym[AsymType(diode,diode)][xtalIdx][ic-1]);
      
      // POS FACE
      XtalDiode xDiodeP(POS_FACE,diode);
      tDiodeIdx diodeIdxP(xtalIdx, xDiodeP);
      m_adc2nrg[diodeIdxP] = enemu/dac2adc(diodeIdxP,(enemu/mpd)*exp(asym_ctr));

      // NEG FACE
      XtalDiode xDiodeN(NEG_FACE,diode);
      tDiodeIdx diodeIdxN(xtalIdx, xDiodeN);
      m_adc2nrg[diodeIdxN] = enemu/dac2adc(diodeIdxN,(enemu/mpd)*exp(-asym_ctr));
    }
  }

  TwrNum twr = m_cfg.twrBay;
  outfile << "<?xml version=\'1.0\' encoding=\'UTF-8\'?>" << endl;
  outfile << "<LATdoc name=\'\'>" << endl;
  outfile << "  <declarations>"   << endl;
  outfile << "    <options>"      << endl;
  outfile << "      <explicitBroadcastNodes>0</explicitBroadcastNodes>" << endl;
  outfile << "    </options>"     << endl;
  outfile << "  </declarations>"  << endl;
  outfile << "  <configuration hierarchy=\"[\'low_hi_nrg\', \'GCCC\', \'GCRC\', \'GCFE\', \'adc2nrg\']\" shape=\'(2, 8, 2, 12)\' version=\'NA\' type=\'d\' name=\'\'>" << endl;
  outfile << "    <GLAT>"         << endl;
  outfile << "      <GTEM ID=\'" << twr << "\'>" << endl;
  for (DiodeNum diode; diode.isValid(); diode++) {
    outfile << "        <low_hi_nrg ID=\'" << diode << "\'>" << endl;
    for (int xy = 0; xy<2; xy++){
      for (int side =0; side < 2; side++){
        //       int gccc = (1-side)+2*xy;  bug - correction in the next line - 8 march 2005
        int gccc = side*2+xy;
        outfile << "          <GCCC ID=\'" << gccc << "\'>" << endl;
        for (int gcrc = 0; gcrc <4; gcrc++){
          outfile << "            <GCRC ID=\'" << gcrc << "\'>" << endl;
          int layer = xy+gcrc*2;
          for (int gcfe = 0; gcfe<12; gcfe++){
            outfile << "              <GCFE ID=\'" << gcfe << "\'>" << endl;
            int col = gcfe;
            tDiodeIdx diodeIdx(layer,col,side,diode);
            float adc2nrg = m_adc2nrg[diodeIdx];
            outfile << "                <adc2nrg>" << fixed 
                    << adc2nrg << "</adc2nrg>" << endl;
            outfile << "              </GCFE>" << endl;
          }
          outfile << "            </GCRC>" << endl;
        }
        outfile << "          </GCCC>" << endl;
      }
    }
    outfile << "        </low_hi_nrg>" << endl;
  }
  outfile << "      </GTEM>" << endl;
  outfile << "    </GLAT>" << endl;
  outfile << "  </configuration>" << endl;
  outfile << "</LATdoc>" << endl;

}
