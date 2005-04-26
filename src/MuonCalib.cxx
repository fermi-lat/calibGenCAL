// LOCAL INCLUDES
#include "MuonCalib.h"
#include "CGCUtil.h"

// GLAST INCLUDES

// EXTLIB INCLUDES
#include "TF1.h"
#include "TCanvas.h"
#include "TH2F.h"

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
  RootFileAnalysis(vector<string>(0),  // no mc files
                   cfg.rootFileList,   // input digi files
                   vector<string>(0),  // no recon files
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
  m_roughPedHists.clear();
  m_pedHists.clear();
  m_asymProfsLL.clear();
  m_asymProfsLS.clear();
  m_asymProfsSL.clear();
  m_asymProfsSS.clear();
  m_dacL2SProfs.clear();
  m_dacLLHists.clear();
  m_asymDACHists.clear();
  m_logratHistsLL.clear();
  m_logratHistsLS.clear();
  m_logratHistsSL.clear();
  m_logratHistsSS.clear();
}

void MuonCalib::openHistFile(const string &filename) {
  if (m_histFile.get()) flushHists();

  m_histFilename = filename;

  m_histFile.reset(new TFile(m_histFilename.c_str(), 
                             "RECREATE", "MuonCalibHistograms",9));
}



void MuonCalib::freeChildren() {
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
    if(m_evtId%1000 == 0)
      m_ostrm << " event " << m_evtId << endl;
  }

  return nb;
}

void MuonCalib::initRoughPedHists() {
  // DEJA VU?
  if (m_roughPedHists.size() == 0) {
    m_roughPedHists.resize(tFaceIdx::N_VALS);

    for (tFaceIdx faceIdx; faceIdx.isValid(); faceIdx++) {
      ostringstream tmp;
      tmp << "roughpeds_" << faceIdx;

      m_roughPedHists[faceIdx] = new TH1F(tmp.str().c_str(),
                                          tmp.str().c_str(),
                                          500,0,1000);
    }
  }else // clear existing histsograms
    for (tFaceIdx faceIdx; faceIdx.isValid(); faceIdx++)
      m_roughPedHists[faceIdx]->Reset();
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

    const TObjArray* calDigiCol = m_digiEvt->getCalDigiCol();
    if (!calDigiCol) {
      cerr << "no calDigiCol found for event#" << iEvt << endl;
      continue;
    }

    CalDigi *pCalDigi = 0;

    //loop through each 'hit' in one event
    for( int cde_nb=0; (pCalDigi=(CalDigi*) calDigiCol->At(cde_nb)); cde_nb++ ){ 
      CalDigi &calDigi = *pCalDigi; // use reference to avoid -> syntax

      //-- XtalId --//
      CalXtalId id(calDigi.getPackedId()); // get interaction information
      // skip hits not for current tower.
      TwrNum twr = id.getTower();
      if (twr != m_cfg.twrBay) continue; 

      ColNum col = id.getColumn();
      LyrNum lyr = id.getLayer();

      // get LEX8 data
      const CalXtalReadout& readout=*calDigi.getXtalReadout(LEX8); 

      // check that we are in the expected readout mode
      RngNum rngP = readout.getRange(CalXtalId::POS);
      RngNum rngN = readout.getRange(CalXtalId::NEG);
      if (rngP != LEX8 || rngN != LEX8) {
        ostringstream tmp;
        tmp << __FILE__  << ":" << __LINE__ << " " 
            << "Event# " << m_evtId << 
          " 1st range shold be LEX8, unexpected trigger mode!";
        throw tmp.str();
      }
      

      float adcP = readout.getAdc(CalXtalId::POS);
      float adcN = readout.getAdc(CalXtalId::NEG);

      tFaceIdx faceIdx(lyr,col,POS_FACE);
      m_roughPedHists[faceIdx]->Fill(adcP);

      faceIdx.setFace(NEG_FACE);
      m_roughPedHists[faceIdx]->Fill(adcN);

    }
  }
}

void MuonCalib::fitRoughPedHists() {
  m_calRoughPed.resize(tFaceIdx::N_VALS);
  m_calRoughPedErr.resize(tFaceIdx::N_VALS);

  for (tFaceIdx faceIdx; faceIdx.isValid(); faceIdx++) {
    // select histogram from list
    TH1F &h= *m_roughPedHists[faceIdx];

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

void MuonCalib::writeRoughPedsTXT(const string &filename) {
  ofstream outfile(filename.c_str());
  if (!outfile.is_open())
    throw string("Unable to open " + filename);
  
  for (tFaceIdx faceIdx; faceIdx.isValid(); faceIdx++)
    outfile << " " << faceIdx.getLyr()
            << " " << faceIdx.getCol()
            << " " << faceIdx.getFace()
            << " " << m_calRoughPed[faceIdx]
            << " " << m_calRoughPedErr[faceIdx]
            << endl;
}

void MuonCalib::initPedHists() {
  // DEJA VU?
  if (m_pedHists.size() == 0) {
    m_pedHists.resize(tRngIdx::N_VALS);

    for (tRngIdx rngIdx; rngIdx.isValid(); rngIdx++) {
      ostringstream tmp;
      tmp << "peds_" << rngIdx;
      m_pedHists[rngIdx] = new TH1F(tmp.str().c_str(),
                                    tmp.str().c_str(),
                                    1000,0,1000);
    }
  }
  else // clear existing histsograms
    for (tRngIdx rngIdx; rngIdx.isValid(); rngIdx++)
      m_pedHists[rngIdx]->Reset();
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

    const TObjArray* calDigiCol = m_digiEvt->getCalDigiCol();
    if (!calDigiCol) {
      cerr << "no calDigiCol found for event#" << iEvt << endl;
      continue;
    }

    CalDigi *pCalDigi = 0;

    //loop through each 'hit' in single event
    for( int cde_nb=0; (pCalDigi=(CalDigi*)calDigiCol->At(cde_nb)); cde_nb++ ) { 
      CalDigi &calDigi = *pCalDigi; // use reference to avoid -> syntax

      //-- XtalId --//
      CalXtalId id(calDigi.getPackedId()); // get interaction information
      // skip hits not for current tower.
      TwrNum twr = id.getTower();
      if (twr != m_cfg.twrBay) continue; 

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
      const CalXtalReadout& readout=*calDigi.getXtalReadout(LEX8); 

      // check that we are in the expected readout mode
      RngNum rngP = readout.getRange(CalXtalId::POS);
      RngNum rngN = readout.getRange(CalXtalId::NEG);
      if (rngP != LEX8 || rngN != LEX8) {
        ostringstream tmp;
        tmp << __FILE__  << ":"     << __LINE__ << " " 
            << "Event# " << m_evtId 
            << " 1st range shold be LEX8, unexpected trigger mode!";
        throw tmp.str(); 
      }
      
      float adcP = readout.getAdc(CalXtalId::POS);
      float adcN = readout.getAdc(CalXtalId::NEG);

      // skip outliers (outside of 5 sigma.
      if (fabs(adcN - m_calRoughPed[tFaceIdx(xtalIdx,NEG_FACE)]) < 
          5*m_calRoughPedErr[tFaceIdx(xtalIdx,NEG_FACE)] &&
          fabs(adcP - m_calRoughPed[tFaceIdx(xtalIdx,POS_FACE)]) < 
          5*m_calRoughPedErr[tFaceIdx(xtalIdx,POS_FACE)]) {

        for (short n = 0; n < nRO; n++) {
          const CalXtalReadout &readout = *calDigi.getXtalReadout(n);

          // check that we are in the expected readout mode
          RngNum rngP = readout.getRange(CalXtalId::POS);
          RngNum rngN = readout.getRange(CalXtalId::NEG);
          if (rngP != n || rngN != n) {
            ostringstream tmp;
            tmp << __FILE__  << ":"     << __LINE__ << " " 
                << "Event# " << m_evtId 
                << " 4-Range readouts out of order.  Expecting 0,1,2,3";
            throw tmp.str();
          }
          RngNum rng = rngP;

          int adc = readout.getAdc(CalXtalId::POS);
          tRngIdx rngIdx(xtalIdx,POS_FACE, rng);
          m_pedHists[rngIdx]->Fill(adc);

          adc = readout.getAdc(CalXtalId::NEG);
          rngIdx.setFace(NEG_FACE);
          m_pedHists[rngIdx]->Fill(adc);
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
    TH1F &h= *m_pedHists[rngIdx];
    
    // trim outliers
    float av = h.GetMean();float err =h.GetRMS();
    for( int iter=0; iter<3;iter++ ) {
      h.SetAxisRange(av-3*err,av+3*err);
      av  = h.GetMean(); 
      err = h.GetRMS();
    }
    
    // Gaussian fit doesn't seem to do well w/ 
    // small # of  populated bins.  if RMS is 
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

void MuonCalib::writePedsTXT(const string &filename) {
  ofstream outfile(filename.c_str());
  if (!outfile.is_open())
    throw string("Unable to open " + filename);
  
  for (tRngIdx rngIdx; rngIdx.isValid(); rngIdx++) {
    outfile << " " << rngIdx.getLyr()
            << " " << rngIdx.getCol()
            << " " << rngIdx.getFace()
            << " " << rngIdx.getRng()
            << " " << m_calPed[rngIdx]
            << " " << m_calPedErr[rngIdx]
            << endl;
  }
}

void MuonCalib::readCalPeds(const string &filename) {
  m_calPed.resize(tRngIdx::N_VALS);
  m_calPedErr.resize(tRngIdx::N_VALS);

  ifstream infile(filename.c_str());
  if (!infile.is_open())
    throw string("Unable to open " + filename);

  unsigned nRead = 0;
  while(infile.good()) {
    float av,err;
    short lyr;
    short col;
    short face;
    short rng;
    
    infile >> lyr
           >> col
           >> face
           >> rng
           >> av
           >> err;
    if (infile.fail()) break; // quit once we can't read any more values
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
    nLine++; // only increment line# if getline() was successful.
    short lyr;
    short col;
    short face;
    short rng;

    float adc, dac;

    istringstream lineStream(lineStr);

    lineStream >> lyr
               >> col
               >> face
               >> rng
               >> dac
               >> adc; // first 4 columns in file id the xtal & adc range
    if (lineStream.fail())
      throw string("Inl file '" + filename + "' is incomplete.");

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
      delete dacs;
      delete adcs;

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

    if (m_cfg.verbose) {
      m_ostrm << "inlspline=" << diodeIdx.getInt()
              << " adc=";
      for (int i = 0; i < arraySize; i++)
        m_ostrm << adcs[i] << " ";
      m_ostrm << endl;
      m_ostrm << "inlspline=" << diodeIdx.getInt()
              << " dac=";
      for (int i = 0; i < arraySize; i++)
        m_ostrm << dacs[i] << " ";
      m_ostrm << endl;
    }

    // create spline object.
    TSpline3 *mySpline    = new TSpline3(name.str().c_str(), adcs, dacs, nADC);
    TSpline3 *mySplineInv = new TSpline3(name.str().c_str(), dacs, adcs, nADC);

    mySpline->SetName(name.str().c_str());
    m_inlSplines[diodeIdx] = mySpline;

    mySplineInv->SetName(name.str().c_str());
    m_inlSplinesInv[diodeIdx] = mySplineInv;
  }

  // cleanup
  delete dacs;
  delete adcs;
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

  const TObjArray* calDigiCol = m_digiEvt->getCalDigiCol();
  if (!calDigiCol) {
    cerr << "no calDigiCol found for event" << m_digiEvt << endl;
    return;
  }

  CalDigi *pCalDigi = 0;

  // PER HIT LOOP:
  for( int cde_nb=0; (pCalDigi=(CalDigi*) calDigiCol->At(cde_nb)); cde_nb++ ) {
    CalDigi &calDigi = *pCalDigi; // use reference to avoid -> syntax

    //-- XtalId --//
    CalXtalId id(calDigi.getPackedId()); // get interaction information
    // skip hits not for current tower.
    TwrNum twr = id.getTower();
    if (twr != m_cfg.twrBay) continue; 
    
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
      DiodeNum diode  = xDiode.getDiode();
      RngNum   rng    = diode.getX8Rng();  // we are only interested in x8 range adc vals for muon calib
      FaceNum  face   = xDiode.getFace();
      tRngIdx   rngIdx  (xtalIdx, face, rng);
      tDiodeIdx diodeIdx(xtalIdx, face, diode);

      const CalXtalReadout& readout = *calDigi.getXtalReadout(rng);
      RngNum tmpRng = readout.getRange((CalXtalId::XtalFace)(int)face);
      // check that we are in the proper readout mode
      if (tmpRng != rng) {
        ostringstream tmp;
        tmp << __FILE__ << ":" << __LINE__ << " " 
            << "Event# " << m_evtId 
            << " 4-Range readouts out of order.  Expecting 0,1,2,3";
        throw tmp.str();
      }

      float adc = readout.getAdc((CalXtalId::XtalFace)(int)face); // raw adc
      hs.adc_ped[diodeIdx] = adc - m_calPed[rngIdx];// subtract pedestals
      if (m_cfg.verbose)
        m_ostrm << "nDiode " << diodeIdx.getInt() 
                << " nRng " << rngIdx.getInt() 
                << " adc " << adc 
                << " ped " << m_calPed[rngIdx]
                << " adc_ped " << hs.adc_ped[diodeIdx] << endl;
    }
    
    // DO WE HAVE A HIT? (sum up both ends LEX8 i.e LARGE_DIODE)
    if (hs.adc_ped[tDiodeIdx(xtalIdx,POS_FACE,LARGE_DIODE)] +
        hs.adc_ped[tDiodeIdx(xtalIdx,NEG_FACE,LARGE_DIODE)] > m_cfg.hitThresh) {

      hs.count++; // increment total # hits

      //m_ostrm << "nHits " << hs.count;// << endl;

      // used to determine if xtal is x or y layer
      if (m_cfg.verbose)
        m_ostrm << " lyr " << lyr << " col " << col ;
      if (lyr.getDir() == X_DIR) { // X layer
        short xLyr = lyr.getXLyr();
        hs.perLyrX[xLyr]++;
        hs.perColX[col]++;
        hs.hitListX.push_back(xtalIdx);
        if (m_cfg.verbose)
          m_ostrm << " X" << " xLyrId " << xLyr
                  << " perLyrX " << hs.perLyrX[xLyr]
                  << " perCol " << hs.perColX[col] << endl;
      } else { // y layer
        short yLyr = lyr.getYLyr();
        hs.perLyrY[yLyr]++;
        hs.perColY[col]++;
        hs.hitListY.push_back(xtalIdx);
        if (m_cfg.verbose)
          m_ostrm << " Y" << " YLyrId " << yLyr 
                  << " perLyrY " << hs.perLyrY[yLyr] 
                  << " perCol " << hs.perColY[col] << endl;
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

    if (m_cfg.verbose) 
      m_ostrm << "mplx " << hs.maxPerLyrX << " y " << hs.maxPerLyrY 
              << " mpl " << hs.maxPerLyr
              << " nlx " << hs.nLyrsX << " y " << hs.nLyrsY << " ncx " 
              << hs.nColsX << " y " << hs.nColsY
              << " fcx " << hs.firstColX << " y " << hs.firstColY << endl;


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

void MuonCalib::initAsymHists(bool genOptHists) {
  // DEJA VU?
  if (m_asymProfsLL.size() == 0) {
    m_asymProfsLL.resize(tXtalIdx::N_VALS);
    m_asymProfsLS.resize(tXtalIdx::N_VALS);
    m_asymProfsSL.resize(tXtalIdx::N_VALS);
    m_asymProfsSS.resize(tXtalIdx::N_VALS);
    if (genOptHists) {
      m_asymDACHists.resize(tDiodeIdx::N_VALS);
      m_logratHistsLL.resize(tXtalIdx::N_VALS);
      m_logratHistsLS.resize(tXtalIdx::N_VALS);
      m_logratHistsSL.resize(tXtalIdx::N_VALS);
      m_logratHistsSS.resize(tXtalIdx::N_VALS);
    }

    // PER XTAL LOOP
    for (tXtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
      // LARGE-LARGE ASYM
      ostringstream tmpLL;
      tmpLL <<  "asymLL_" << xtalIdx;
      // columns are #'d 0-11, hist contains 1-10. 
      // .5 & 10.5 limit puts 1-10 at center of bins
      m_asymProfsLL[xtalIdx] = new TProfile(tmpLL.str().c_str(),
                                            tmpLL.str().c_str(),
                                            N_ASYM_PTS,.5,10.5);

      // LARGE-SMALL ASYM
      ostringstream tmpLS;
      tmpLS << "asymLS_" << xtalIdx;
      m_asymProfsLS[xtalIdx] = new TProfile(tmpLS.str().c_str(),
                                            tmpLS.str().c_str(),
                                            N_ASYM_PTS,.5,10.5);

      // SMALL-LARGE ASYM
      ostringstream tmpSL;
      tmpSL <<  "asymSL_" << xtalIdx;
      m_asymProfsSL[xtalIdx] = new TProfile(tmpSL.str().c_str(),
                                            tmpSL.str().c_str(),
                                            N_ASYM_PTS,.5,10.5);

      // SMALL-SMALL ASYM
      ostringstream tmpSS;
      tmpSS << "asymSS_" << xtalIdx;
      m_asymProfsSS[xtalIdx] = new TProfile(tmpSS.str().c_str(),
                                            tmpSS.str().c_str(),
                                            N_ASYM_PTS,.5,10.5);

      // Optional logratHists
      if (genOptHists) {
        // LARGE-LARGE LOGRAT
        ostringstream tmpLL;
        tmpLL << "logratLL_" << xtalIdx;
        // columns are #'d 0-11, hist contains 1-10. 
        // .5 & 10.5 limit puts 1-10 at center of bins
        m_logratHistsLL[xtalIdx] = new TH2F(tmpLL.str().c_str(),
                                            tmpLL.str().c_str(),
                                            N_ASYM_PTS,.5,10.5,
                                            100,-.1,.1);
        m_logratHistsLL[xtalIdx]->SetBit(TH1::kCanRebin);
        
        // LARGE-SMALL LOGRAT
        ostringstream tmpLS;
        tmpLS << "logratLS_" << xtalIdx;
        m_logratHistsLS[xtalIdx] = new TH2F(tmpLS.str().c_str(),
                                            tmpLS.str().c_str(),
                                            N_ASYM_PTS,.5,10.5,
                                            100,1.5,1.6);
        m_logratHistsLS[xtalIdx]->SetBit(TH1::kCanRebin);

        // SMALL-LARGE LOGRAT
        ostringstream tmpSL;
        tmpSL << "logratSL_" << xtalIdx;
        m_logratHistsSL[xtalIdx] = new TH2F(tmpSL.str().c_str(),
                                            tmpSL.str().c_str(),
                                            N_ASYM_PTS,.5,10.5,
                                            100,-2.0,-1.9);
        m_logratHistsSL[xtalIdx]->SetBit(TH1::kCanRebin);

        // SMALL-SMALL LOGRAT
        ostringstream tmpSS;
        tmpSS << "logratSS_" << xtalIdx;
        m_logratHistsSS[xtalIdx] = new TH2F(tmpSS.str().c_str(),
                                            tmpSS.str().c_str(),
                                            N_ASYM_PTS,.5,10.5,
                                            100,-.1,.1);
        m_logratHistsSS[xtalIdx]->SetBit(TH1::kCanRebin);
      }

      if (genOptHists)
        for (XtalDiode xDiode; xDiode.isValid(); xDiode++) {            
          tDiodeIdx diodeIdx(xtalIdx, xDiode);
          
          ostringstream tmp;
          tmp << "asymdac_" << diodeIdx;
          m_asymDACHists[diodeIdx] = new TH1F(tmp.str().c_str(),
                                              tmp.str().c_str(),
                                              100,0,0);
          
        }
    }
  } else // clear existing histsograms
    for (tXtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
      m_asymProfsLL[xtalIdx]->Reset();
      m_asymProfsLS[xtalIdx]->Reset();
      m_asymProfsSL[xtalIdx]->Reset();
      m_asymProfsSS[xtalIdx]->Reset();
      if (genOptHists) {
        m_logratHistsLL[xtalIdx]->Reset();
        m_logratHistsLS[xtalIdx]->Reset();
        m_logratHistsSL[xtalIdx]->Reset();
        m_logratHistsSS[xtalIdx]->Reset();
        for (XtalDiode xDiode; xDiode.isValid(); xDiode++)
          m_asymDACHists[tDiodeIdx(xtalIdx,xDiode)]->Reset();
      }
    }
}

/** 
    \note 'test direction' refers to the direction w/ 4 vertical hits.  
    In asymetry calibration this direction is used to get position info 
    for the 'orthogonal direction'

    \note 'orthogonal direction' refers to the direction with the hits that 
    are actually measured for asymetry.
*/

void MuonCalib::fillAsymHists(int nEvts, bool genOptHists) {
  initAsymHists(genOptHists);

  int lastEvt = chkForEvts(nEvts);
  HitSummary hs;

  int  nGoodDirs  = 0; // count total # of events used
  int  nXDirs     = 0;
  int  nYDirs     = 0;
  long nHits      = 0; // count total # of xtals measured
  int  nBadHits   = 0;
  int  nBadAsymSS = 0;
  int  nBadAsymLL = 0;
  int  nBadAsymSL = 0;
  int  nBadAsymLS = 0;
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
      
        // short hand for each xtal in diode.
        tDiodeIdx diodePL(xtalIdx,POS_FACE,LARGE_DIODE);
        tDiodeIdx diodeNL(xtalIdx,NEG_FACE,LARGE_DIODE);
        tDiodeIdx diodePS(xtalIdx,POS_FACE,SMALL_DIODE);
        tDiodeIdx diodeNS(xtalIdx,NEG_FACE,SMALL_DIODE);

        if (m_cfg.verbose) {
          m_ostrm << "diodePL=" << diodePL.getInt()
                  << " " << xtalIdx.getInt()
                  << " " << POS_FACE
                  << " " << LARGE_DIODE
                  << endl;

          m_ostrm << " evt=" << m_evtId << " adc_ped"
                  << " " << hs.adc_ped[diodePL]
                  << " " << hs.adc_ped[diodePS]
                  << " " << hs.adc_ped[diodeNL]
                  << " " << hs.adc_ped[diodeNS] 
                  << endl;
        }

        
        double dacPL = adc2dac(diodePL,hs.adc_ped[diodePL]);
        double dacPS = adc2dac(diodePS,hs.adc_ped[diodePS]);
        double dacNL = adc2dac(diodeNL,hs.adc_ped[diodeNL]);
        double dacNS = adc2dac(diodeNS,hs.adc_ped[diodeNS]);

        if (m_cfg.verbose)
          m_ostrm << " evt=" << m_evtId
                  << " dacPL=" << dacPL
                  << " PS=" << dacPS
                  << " NL=" << dacNL
                  << " NS=" << dacNS << endl;  

        // fill optional dacVal histograms
        if (genOptHists) {
          m_asymDACHists[diodePL]->Fill(dacPL);
          m_asymDACHists[diodePS]->Fill(dacPS);
          m_asymDACHists[diodeNL]->Fill(dacNL);
          m_asymDACHists[diodeNS]->Fill(dacNS);
        }
        
        if (dacPL <= 0 || dacPS <= 0 || dacNL <= 0 || dacNS <= 0) {
          nBadHits++;
          nBadDACs++;
          continue;
        }
        
        // calcuate the 4 log ratios = log(POS/NEG)
        double asymLL = log(dacPL/dacNL);
        double asymLS = log(dacPL/dacNS);
        double asymSL = log(dacPS/dacNL);
        double asymSS = log(dacPS/dacNS);
                  
        if (m_cfg.verbose)
          m_ostrm << " evt=" << m_evtId
                  << " asymLL=" << asymLL
                  << " SS=" << asymSS
                  << " SL=" << asymSL
                  << " LS=" << asymLS << endl;  
        
        if (genOptHists) {
          m_logratHistsLL[xtalIdx]->Fill(pos, asymLL);
          m_logratHistsLS[xtalIdx]->Fill(pos, asymLS);
          m_logratHistsSL[xtalIdx]->Fill(pos, asymSL);
          m_logratHistsSS[xtalIdx]->Fill(pos, asymSS);
        }
   
        // check for asym vals which are way out of range
        
        // test LL first since it is most likely
        if (asymLL < m_cfg.minAsymLL || asymLL > m_cfg.maxAsymLL) {
          if (m_cfg.verbose)
            m_ostrm << " **AsymLL out-of-range evt=" << m_evtId
                    << " nXtal=" << xtalIdx.getInt()
                    << " pos=" << pos
                    << " dacP=" << dacPL << " dacN=" << dacNL
                    << " asym=" << asymLL << endl;
          nBadHits++;
          nBadAsymLL++;
          continue;
        }
        // test SS 2nd since bad LL or bad SS should leave  only small #
        // of bad LS & SL's
        if (asymSS < m_cfg.minAsymSS || asymSS > m_cfg.maxAsymSS) {
          if (m_cfg.verbose)
            m_ostrm << " **AsymSS out-of-range evt=" << m_evtId
                    << " nXtal=" << xtalIdx.getInt()
                    << " pos=" << pos
                    << " dacP=" << dacPS << " dacN=" << dacNS
                    << " asym=" << asymSS << endl;
          nBadHits++;
          nBadAsymSS++;
          continue;
        }
        if (asymLS < m_cfg.minAsymLS || asymLS > m_cfg.maxAsymLS) {
          if (m_cfg.verbose)
            m_ostrm << " **AsymLS out-of-range evt=" << m_evtId
                    << " nXtal=" << xtalIdx.getInt()
                    << " pos=" << pos
                    << " dacP=" << dacPL << " dacN=" << dacNS
                    << " asym=" << asymLS << endl;
          nBadHits++;
          nBadAsymLS++;
          continue;
        }
        if (asymSL < m_cfg.minAsymSL || asymSL > m_cfg.maxAsymSL) {
          if (m_cfg.verbose)
            m_ostrm << " **AsymSL out-of-range evt=" << m_evtId
                    << " nXtal=" << xtalIdx.getInt()
                    << " pos=" << pos
                    << " dacP=" << dacPS << " dacN=" << dacNS
                    << " asym=" << asymSL << endl;
          nBadHits++;
          nBadAsymSL++;
          continue;
        }
      
        // pos - 5.5 value will range from -4.5 to +4.5 in xtal width units
        m_asymProfsLL[xtalIdx]->Fill(pos, asymLL);
        m_asymProfsLS[xtalIdx]->Fill(pos, asymLS);
        m_asymProfsSL[xtalIdx]->Fill(pos, asymSL);
        m_asymProfsSS[xtalIdx]->Fill(pos, asymSS);

        nHits++;
      } // per hit loop
    } // per direction loop
  } // per event loop
  
  m_ostrm << "Asymmetry histograms filled nEvents=" << nGoodDirs
          << " nXDirs="               << nXDirs
          << " nYDirs="               << nYDirs << endl;
  m_ostrm << " nHits measured="       << nHits
          << " Bad hits="             << nBadHits
          << " asym out-of-range LL=" << nBadAsymLL
          << " SS=" << nBadAsymSS
          << " SL=" << nBadAsymSL
          << " LS=" << nBadAsymLS
          << endl;
}

void MuonCalib::populateAsymArrays() {
  m_calAsymLL.resize(tXtalIdx::N_VALS);
  m_calAsymLS.resize(tXtalIdx::N_VALS);
  m_calAsymSL.resize(tXtalIdx::N_VALS);
  m_calAsymSS.resize(tXtalIdx::N_VALS);

  m_calAsymLLErr.resize(tXtalIdx::N_VALS);
  m_calAsymLSErr.resize(tXtalIdx::N_VALS);
  m_calAsymSLErr.resize(tXtalIdx::N_VALS);
  m_calAsymSSErr.resize(tXtalIdx::N_VALS);

  // PER XTAL LOOP
  for (tXtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {

    // retrieve profile objects for this xtal
    TProfile &profLL = *m_asymProfsLL[xtalIdx];
    TProfile &profLS = *m_asymProfsLS.at(xtalIdx);
    TProfile &profSL = *m_asymProfsSL[xtalIdx];
    TProfile &profSS = *m_asymProfsSS.at(xtalIdx);

    if (m_cfg.verbose) {
      LyrNum lyr = xtalIdx.getLyr(); ColNum col = xtalIdx.getCol();
      m_ostrm << "Asym entries nXtal=" << xtalIdx.getInt()
              << " LL=" << profLL.GetEntries() << endl;
      m_ostrm << " per bin ";
      for (int i = 0; i < N_ASYM_PTS; i++)
        m_ostrm << " " << i << "," << profLL.GetBinEntries(i+1);
      m_ostrm << endl;
    }

    // ensure proper vector size
    m_calAsymLL[xtalIdx].resize(N_ASYM_PTS);
    m_calAsymLS[xtalIdx].resize(N_ASYM_PTS);
    m_calAsymSL[xtalIdx].resize(N_ASYM_PTS);
    m_calAsymSS[xtalIdx].resize(N_ASYM_PTS);

    m_calAsymLLErr[xtalIdx].resize(N_ASYM_PTS);
    m_calAsymLSErr[xtalIdx].resize(N_ASYM_PTS);
    m_calAsymSLErr[xtalIdx].resize(N_ASYM_PTS);
    m_calAsymSSErr[xtalIdx].resize(N_ASYM_PTS);

    // loop through all N_ASYM_PTS bins in asymmetry profile
    for (int i = 0; i < N_ASYM_PTS; i++) {
      // HISTOGRAM BINS START AT 1 NOT ZERO! (hence 'i+1')
      m_calAsymLL[xtalIdx][i] = profLL.GetBinContent(i+1); 
      m_calAsymLS[xtalIdx][i] = profLS.GetBinContent(i+1);
      m_calAsymSL[xtalIdx][i] = profSL.GetBinContent(i+1);
      m_calAsymSS[xtalIdx][i] = profSS.GetBinContent(i+1);

      m_calAsymLLErr[xtalIdx][i] = profLL.GetBinError(i+1);
      m_calAsymLSErr[xtalIdx][i] = profLS.GetBinError(i+1);
      m_calAsymSLErr[xtalIdx][i] = profSL.GetBinError(i+1);
      m_calAsymSSErr[xtalIdx][i] = profSS.GetBinError(i+1);
    }
  }
}

double MuonCalib::adc2dac(tDiodeIdx diodeIdx, double adc) {
  if (m_cfg.verbose) 
    m_ostrm << "adc2dac idx=" << diodeIdx.getInt() << " " << adc << endl;
  return m_inlSplines[diodeIdx]->Eval(adc);
}

double MuonCalib::dac2adc(tDiodeIdx diodeIdx, double dac) {
  return m_inlSplinesInv[diodeIdx]->Eval(dac);
}

void MuonCalib::writeAsymTXT(const string &filenameLL,
                             const string &filenameLS,
                             const string &filenameSL,
                             const string &filenameSS) {

  ofstream outfileLL(filenameLL.c_str());
  ofstream outfileLS(filenameLS.c_str());
  ofstream outfileSL(filenameSL.c_str());
  ofstream outfileSS(filenameSS.c_str());

  if (!outfileLL.is_open())
    throw string("Unable to open " + filenameLL);
  if (!outfileLS.is_open())
    throw string("Unable to open " + filenameLS);
  if (!outfileSL.is_open())
    throw string("Unable to open " + filenameSL);
  if (!outfileSS.is_open())
    throw string("Unable to open " + filenameSS);

  // PER XTAL LOOP
  for (tXtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
    LyrNum lyr = xtalIdx.getLyr();
    ColNum col = xtalIdx.getCol();

    outfileLL << lyr << " " << col;
    outfileLS << lyr << " " << col;
    outfileSL << lyr << " " << col;
    outfileSS << lyr << " " << col;

    // loop through all N_ASYM_PTS points in each asymmetry curve
    for (int i = 0; i < N_ASYM_PTS; i++) {
      outfileLL << " " << m_calAsymLL[xtalIdx][i] 
                << " " << m_calAsymLLErr[xtalIdx][i];
      outfileLS << " " << m_calAsymLS[xtalIdx][i] 
                << " " << m_calAsymLSErr[xtalIdx][i];
      outfileSL << " " << m_calAsymSL[xtalIdx][i] 
                << " " << m_calAsymSLErr[xtalIdx][i];
      outfileSS << " " << m_calAsymSS[xtalIdx][i] 
                << " " << m_calAsymSSErr[xtalIdx][i];
    }

    outfileLL << endl;
    outfileLS << endl;
    outfileSL << endl;
    outfileSS << endl;
  }
}

void MuonCalib::readAsymTXT(const string &filenameLL,
                            const string &filenameLS,
                            const string &filenameSL,
                            const string &filenameSS) {

  m_calAsymLL.resize(tXtalIdx::N_VALS);
  m_calAsymLS.resize(tXtalIdx::N_VALS);
  m_calAsymSL.resize(tXtalIdx::N_VALS);
  m_calAsymSS.resize(tXtalIdx::N_VALS);

  m_calAsymLLErr.resize(tXtalIdx::N_VALS);
  m_calAsymLSErr.resize(tXtalIdx::N_VALS);
  m_calAsymSLErr.resize(tXtalIdx::N_VALS);
  m_calAsymSSErr.resize(tXtalIdx::N_VALS);

  // ASYM (4-TYPES) //

  // we go through the same exact sequence for each of the the 4 asym files.
  for (int nFile = 0; nFile < 4; nFile++) {
    short lyr, col;
    unsigned nRead = 0;
    CalVec<tXtalIdx,vector<float > > *calVec, *errVec;
    const string *pFilename;

    // select proper input file & destination vector
    switch (nFile) {
    case 0: 
      calVec = &m_calAsymLL; 
      errVec = &m_calAsymLLErr;
      pFilename = &filenameLL; 
      break;
    case 1: 
      calVec = &m_calAsymLS; 
      errVec = &m_calAsymLSErr;
      pFilename = &filenameLS; 
      break;
    case 2: 
      calVec = &m_calAsymSL; 
      errVec = &m_calAsymSLErr;
      pFilename = &filenameSL; 
      break;
    case 3: 
      calVec = &m_calAsymSS; 
      errVec = &m_calAsymSSErr;
      pFilename = &filenameSS; 
      break;
    default:
      throw string("Programmer error, bad switch in readAsymTXT.");
    } // switch (nFile)
    const string &filename = *pFilename; // use ref to avoid ->

    // open file
    ifstream infile(filename.c_str());
    if (!infile.is_open())
      throw string("Unable to open " + filename);

    // loop through each line in file
    while (infile.good()) {
      // get lyr, col (xtalId)
      infile >> lyr >> col;
      if (infile.fail()) continue; // bad get()
      tXtalIdx xtalIdx(lyr,col);

      // ensure proper vector size
      (*calVec)[xtalIdx].resize(N_ASYM_PTS);
      (*errVec)[xtalIdx].resize(N_ASYM_PTS);

      // i'm expecting 10 asymmetry values
      for (int i = 0; i < N_ASYM_PTS; i++) {
        infile >> (*calVec)[xtalIdx][i];  // read in value
        infile >> (*errVec)[xtalIdx][i];  // read in error val
      }
      if (infile.fail()) break; // quit once we can't read any more values

      nRead++;
    }

    // check that we got all the values
    if (nRead != (*calVec).size()) {
      ostringstream tmp;
      tmp << __FILE__ << ":" << __LINE__ << " " 
          << "File '" << filename << "' is incomplete: " << nRead
          << " vals read, "       << m_calAsymLL.size() << " vals expected.";
      throw tmp.str();
    }
  } //for (nFile 0 to 4)
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
    vector<float> &asymVec = m_calAsymLL[xtalIdx];
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

double MuonCalib::asym2pos(tXtalIdx xtalIdx, double asym) {
  return m_asym2PosSplines[xtalIdx]->Eval(asym);
}

void MuonCalib::initMPDHists() {
  // DEJA VU?
  if (m_dacLLHists.size() == 0) {
    m_dacLLHists.resize(tXtalIdx::N_VALS);
    m_dacL2SProfs.resize(tXtalIdx::N_VALS);

    for (tXtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {

      // LARGE-LARGE DAC
      ostringstream tmpLL;
      tmpLL << "dacLL_" << xtalIdx;
      m_dacLLHists[xtalIdx] = new TH1F(tmpLL.str().c_str(),
                                       tmpLL.str().c_str(),
                                       200,0,100);

      // DAC L2S RATIO
      ostringstream tmpLS;
      tmpLS << "dacL2S_" << xtalIdx;
      m_dacL2SProfs[xtalIdx] = new TProfile(tmpLS.str().c_str(),
                                            tmpLS.str().c_str(),
                                            N_L2S_PTS,10,60);
    }
  }
  else // clear existing histsograms
    for (tXtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
      m_dacLLHists[xtalIdx]->Reset();
      m_dacL2SProfs[xtalIdx]->Reset();
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
  TH2F viewHist("viewHist","viewHist",
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
  double slopeFactor = m_cfg.cellHorPitch/m_cfg.cellVertPitch;

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

      vector<float> dacsPL(hitList.size());
      vector<float> dacsNL(hitList.size());
      vector<float> dacsPS(hitList.size());
      vector<float> dacsNS(hitList.size());

      // now that we have eliminated events on the ends of xtals, we can use
      // asymmetry to get a higher precision slope
      // we'll keep the dac vals while we're at it since some are used
      // more than once
      graph.Set(hitList.size());
      for (unsigned i = 0; i < hitList.size(); i++) {
        tXtalIdx xtalIdx = hitList[i];
        LyrNum lyr = xtalIdx.getLyr();

        // short hand for each xtal in diode.
        tDiodeIdx diodePL(xtalIdx,POS_FACE,LARGE_DIODE);
        tDiodeIdx diodeNL(xtalIdx,NEG_FACE,LARGE_DIODE);
        tDiodeIdx diodePS(xtalIdx,POS_FACE,SMALL_DIODE);
        tDiodeIdx diodeNS(xtalIdx,NEG_FACE,SMALL_DIODE);

        // calculate DAC vals
        dacsPL[i] = adc2dac(diodePL, hs.adc_ped[diodePL]);
        dacsNL[i] = adc2dac(diodeNL, hs.adc_ped[diodeNL]);
        dacsPS[i] = adc2dac(diodePS, hs.adc_ped[diodePS]);
        dacsNS[i] = adc2dac(diodeNS, hs.adc_ped[diodeNS]);

        // calcuate the log ratio = log(POS/NEG)
        float asymLL = log(dacsPL[i]/dacsNL[i]);

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
        dacsPL[i] /= sec;
        dacsNL[i] /= sec;
        dacsPS[i] /= sec;
        dacsNS[i] /= sec;

        float meanDACLarge = sqrt(dacsPL[i]*dacsNL[i]);
        float meanDACSmall = sqrt(dacsPS[i]*dacsNS[i]);

        // Load meanDAC Histogram
        m_dacLLHists[xtalIdx]->Fill(meanDACLarge);

        // load dacL2S profile
        m_dacL2SProfs[xtalIdx]->Fill(meanDACLarge,meanDACSmall);

        xtalIdxs++;
      } // populate histograms
    } // direction loop
  } // event loop

  m_ostrm << "MPD histograms filled nXEvts=" << nXEvts 
          << " nYEvents=" << nYEvts 
          << " nXtals measured " << xtalIdxs << endl;
}

void MuonCalib::fitMPDHists() {
  m_calMPDLarge.resize(tXtalIdx::N_VALS);
  m_calMPDSmall.resize(tXtalIdx::N_VALS);
  m_calMPDLargeErr.resize(tXtalIdx::N_VALS);
  m_calMPDSmallErr.resize(tXtalIdx::N_VALS);
  m_adc2nrg.resize(tDiodeIdx::N_VALS);


  ////////////////////////////////////////////////////
  // INITIALIZE ROOT PLOTTING OBJECTS FOR LINE FITS //
  ////////////////////////////////////////////////////
  // viewHist is used to set scale before drawing TGraph
  TH2F viewHist("viewHist","viewHist",
                N_L2S_PTS, 10,60, // X-LIMITS LARGE
                N_L2S_PTS, 1,60); // Y-LIMITS SMALL
  TCanvas canvas("canvas","event display",800,600);
  viewHist.Draw();
  TGraph graph(N_L2S_PTS); graph.Draw("*");
  canvas.Update();
  TF1 lineFunc("line","pol1",0,8);

  // PER XTAL LOOP
  for (tXtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
    // retrieve Large diode DAC histogram
    TH1F& h = *m_dacLLHists[xtalIdx];

    ///////////////////////////////////
    //-- MeV Per Dac (Large Diode) --//
    ///////////////////////////////////
    
    // LANDAU fit for muon peak (limit outliers by n*err)
    float ave = h.GetMean();
    float err = h.GetRMS();
    h.Fit("landau", "Q", "", ave-2*err, ave+3*err);
    float mean  = (h.GetFunction("landau"))->GetParameter(1);
    float sigma = (h.GetFunction("landau"))->GetParameter(2);

    m_calMPDLarge[xtalIdx] = 11.2/mean;

    // keep sigma proportional to extrapolated val
    m_calMPDLargeErr[xtalIdx] = 
      m_calMPDLarge[xtalIdx] * sigma/mean; 

    
    ///////////////////////
    //-- (Small Diode) --//
    ///////////////////////
        
    // LARGE 2 SMALL Ratio
    TProfile& p = *m_dacL2SProfs[xtalIdx]; // get profile

    // Fill scatter graph w/ smallDAC vs largeDAC points
    int nPts = 0;
    graph.Set(nPts); // start w/ empty graph
    for (int i = 0; i < N_L2S_PTS; i++) {
      // only insert a bin if it has entries
      if (!(p.GetBinEntries(i+1) > 0)) continue; // bins #'d from 1
      nPts++;

      // retrieve small & large dac vals
      float smallDAC = p.GetBinContent(i+1);
      float largeDAC = p.GetBinCenter(i+1);

      // update graphsize & set point
      graph.Set(nPts);
      graph.SetPoint(nPts-1,largeDAC,smallDAC);
    }

    // bail if for some reason we didn't get any points
    if (!nPts) {
      ostringstream tmp;
      tmp << __FILE__  << ":"     << __LINE__ << " " 
          << "Event# " << m_evtId 
          << "Unable to find small diode MPD for xtal=" << xtalIdx
          << " due to empty histogram." << endl;
      throw tmp.str();
    }

    // fit straight line to get mean ratio
    graph.Fit(&lineFunc,"WQN");
    // get slope = mean ratio of smallDac/largeDac
    float small2large = lineFunc.GetParameter(1);
    
    //-- NOTES:
    // MPDLarge     = MeV/LargeDAC
    // small2large  = SmallDAC/LargeDAC
    // MPDSmall     = MeV/SmallDAC = (MeV/LargeDAC)*(LargeDAC/SmallDAC) 
    //              = MPDLarge/small2large
    
    m_calMPDSmall[xtalIdx] = m_calMPDLarge[xtalIdx]/small2large;
    
    //-- Propogate errors
    // in order to combine slope & MPD error for final error
    // I need the relative error for both values - so sayeth sasha
    float relLineErr = lineFunc.GetParError(1)/small2large;
    float relMPDErr  = m_calMPDLargeErr[xtalIdx]/m_calMPDLarge[xtalIdx];

    m_calMPDSmallErr[xtalIdx] = m_calMPDSmall[xtalIdx]*
      sqrt(relLineErr*relLineErr + relMPDErr*relMPDErr);
  }
}

void MuonCalib::writeMPDTXT(const string &filenameLrg, 
                            const string &filenameSm) {
  ofstream outfileL(filenameLrg.c_str());
  ofstream outfileS(filenameSm.c_str());
  if (!outfileL.is_open())
    throw string("Unable to open " + filenameLrg);
  if (!outfileS.is_open())
    throw string("Unable to open " + filenameSm);


  for (tXtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
    LyrNum lyr = xtalIdx.getLyr();
    ColNum col = xtalIdx.getCol();

    outfileL << " " << lyr
             << " " << col
             << " " << m_calMPDLarge[xtalIdx]
             << " " << m_calMPDLargeErr[xtalIdx]
             << endl;
    outfileS << " " << lyr
             << " " << col
             << " " << m_calMPDSmall[xtalIdx]
             << " " << m_calMPDSmallErr[xtalIdx]
             << endl;
  }
}

void MuonCalib::writePedsXML(const string &filename, const string &dtdPath) {
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
  outfile << " <dimension nRow=\"1\" nCol=\"1\" nLayer=\"" << LyrNum::N_VALS 
          << "\" nXtal=\""  << ColNum::N_VALS 
          << "\" nFace=\""   << FaceNum::N_VALS 
          << "\" nRange=\"" << RngNum::N_VALS 
          << "\"/>" << endl;
  
  // currently only loop through one tower.
  for (TwrNum twr=m_cfg.twrBay; twr == m_cfg.twrBay; twr++) {
    outfile << " <tower iRow=\"" << twr.getRow() 
            << "\" iCol=\"" << twr.getCol() << "\">" << endl;

    for (LyrNum lyr; lyr.isValid(); lyr++) {
      outfile << "  <layer iLayer=\"" << lyr << "\">" << endl;
      for (ColNum col; col.isValid(); col++) {
        outfile << "   <xtal iXtal=\"" << col << "\">" << endl;
        for (FaceNum face; face.isValid(); face++) {
          outfile << "    <face end=\"" << FaceNum::MNEM[face] << "\">" << endl;

          for (RngNum rng; rng.isValid(); rng++) {
            tRngIdx rngIdx(lyr,col,face,rng);

            float av = m_calPed[rngIdx];
            float err= m_calPedErr[rngIdx];

            outfile << "     <calPed avg=\"" << fixed << av
                    << "\" sig=\""           << fixed << err
                    << "\" range=\""         << RngNum::MNEM[rng] << "\"/>"
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

void MuonCalib::writeAsymXML(const string &filename, const string &dtdPath) {
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
  outfile << " <dimension nRow=\"1\" nCol=\"1\" nLayer=\"" << LyrNum::N_VALS 
          << "\" nXtal=\""  << ColNum::N_VALS 
          << "\" nFace=\""   << 1
          << "\" nRange=\"" << 1
          << "\" nXpos=\""  << 1
          << "\"/>" << endl;
  
  // -- GENERATE xpos VALUES -- //
  outfile << " <xpos values=\"";
  for (int i = 0; i < N_ASYM_PTS; i++) 
    outfile << xtalCenterPos(i+1) << " ";
  outfile << "\"/>" << endl;
  
  // -- OUTPUT ASYMETRY DATA -- //
  // currently only loop through one tower.
  for (TwrNum twr=m_cfg.twrBay; twr == m_cfg.twrBay; twr++) {
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
          outfile << " " << fixed << m_calAsymLL[xtalIdx][i];
        outfile << "\"" << endl;
        // ASYM LL Err
        outfile << "           bigSigs=\"";
        for (int i = 0; i < N_ASYM_PTS; i++)
          outfile << " " << fixed << m_calAsymLLErr[xtalIdx][i];
        outfile << "\"" << endl;

        // ASYM LS
        outfile << "           NsmallPbigVals=\"";
        for (int i = 0; i < N_ASYM_PTS; i++)
          outfile << " " << fixed << m_calAsymLS[xtalIdx][i];
        outfile << "\"" << endl;
        // ASYM LS Err
        outfile << "           NsmallPbigSigs=\"";
        for (int i = 0; i < N_ASYM_PTS; i++)
          outfile << " " << fixed << m_calAsymLSErr[xtalIdx][i];
        outfile << "\"" << endl;

        // ASYM SL
        outfile << "           PsmallNbigVals=\"";
        for (int i = 0; i < N_ASYM_PTS; i++)
          outfile << " " << fixed << m_calAsymSL[xtalIdx][i];
        outfile << "\"" << endl;
        // ASYM SL Err
        outfile << "           PsmallNbigSigs=\"";
        for (int i = 0; i < N_ASYM_PTS; i++)
          outfile << " " << fixed << m_calAsymSLErr[xtalIdx][i];
        outfile << "\"" << endl;

        // ASYM SS
        outfile << "           smallVals=\"";
        for (int i = 0; i < N_ASYM_PTS; i++)
          outfile << " " << fixed << m_calAsymSS[xtalIdx][i];
        outfile << "\"" << endl;
        // ASYM SS Err
        outfile << "           smallSigs=\"";
        for (int i = 0; i < N_ASYM_PTS; i++)
          outfile << " " << fixed << m_calAsymSSErr[xtalIdx][i];
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

void MuonCalib::writeMPDXML(const string &filename, const string &dtdPath) {
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
  outfile << " <dimension nRow=\"1\" nCol=\"1\" nLayer=\"" << LyrNum::N_VALS 
          << "\" nXtal=\""  << ColNum::N_VALS 
          << "\" nFace=\""  << 1 
          << "\" nRange=\"" << 1 
          << "\" nXpos=\""  << 1 
          << "\"/>" << endl;

  // currently only loop through one tower.
  for (TwrNum twr=m_cfg.twrBay; twr == m_cfg.twrBay; twr++) {
    outfile << " <tower iRow=\"" << twr.getRow() 
            << "\" iCol=\"" << twr.getCol() << "\">" << endl;

    for (LyrNum lyr; lyr.isValid(); lyr++) {
      outfile << "  <layer iLayer=\"" << lyr << "\">" << endl;
      for (ColNum col; col.isValid(); col++) {
        tXtalIdx xtalIdx(lyr,col);
        outfile << "   <xtal iXtal=\"" << col << "\">" << endl;
        outfile << "    <face end=\"" << "NA" << "\">" << endl;

        outfile << "     <mevPerDac bigVal=\"" << fixed 
                << m_calMPDLarge[xtalIdx] 
                << "\" bigSig=\""   << fixed << m_calMPDLargeErr[xtalIdx]
                << "\" smallVal=\"" << fixed << m_calMPDSmall[xtalIdx]
                << "\" smallSig=\"" << fixed << m_calMPDSmallErr[xtalIdx]
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
  for (tXtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
    float mpdL = m_calMPDLarge[xtalIdx];
    float mpdS = m_calMPDSmall[xtalIdx];
    int ic = N_ASYM_PTS/2;
    float asym_center_L = 0.25*(m_calAsymLL[xtalIdx][ic]+m_calAsymLL[xtalIdx][ic-1]);
    float asym_center_S = 0.25*(m_calAsymSS[xtalIdx][ic]+m_calAsymSS[xtalIdx][ic-1]);

    tDiodeIdx diodeIdxPS(xtalIdx, POS_FACE, SMALL_DIODE);
    tDiodeIdx diodeIdxPL(xtalIdx, POS_FACE, LARGE_DIODE);
    tDiodeIdx diodeIdxNS(xtalIdx, NEG_FACE, SMALL_DIODE);
    tDiodeIdx diodeIdxNL(xtalIdx, NEG_FACE, LARGE_DIODE);

    m_adc2nrg[diodeIdxPL] = enemu/dac2adc(diodeIdxPL,(enemu/mpdL)*exp(asym_center_L));
    m_adc2nrg[diodeIdxNL] = enemu/dac2adc(diodeIdxNL,(enemu/mpdL)*exp(-asym_center_L));
    m_adc2nrg[diodeIdxPS] = enemu/dac2adc(diodeIdxPS,(enemu/mpdS)*exp(asym_center_S));
    m_adc2nrg[diodeIdxNS] = enemu/dac2adc(diodeIdxNS,(enemu/mpdS)*exp(-asym_center_S));
  }

  TwrNum twr = 0;
  outfile << "<?xml version=\'1.0\' encoding=\'UTF-8\'?>" << endl;
  outfile << "<LATdoc name=\'\'>" << endl;
  outfile << "  <declarations>"   << endl;
  outfile << "    <options>"      << endl;
  outfile << "      <explicitBroadcastNodes>0</explicitBroadcastNodes>" << endl;
  outfile << "    </options>"     << endl;
  outfile << "  </declarations>"  << endl;
  outfile << "  <configuration hierarchy=\"[\'low_hi_nrg\', \'GCCC\', \'GCRC\', \'GCFE\', \'adc2nrg\']\" shape=\'(2, 8, 2, 12)\' version=\'NA\' type=\'d\' name=\'\'>" << endl;
  outfile << "    <GLAT>"         << endl;
  outfile << "      <GTEM ID=\'0\'>" << endl;
  for (int diode=0; diode<2; diode++){
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
