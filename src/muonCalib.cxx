#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <functional>

//ROOT INCLUDES
#include "TF1.h"
#include "TCanvas.h"
#include "TH2F.h"

#include "muonCalib.h"

#include "CGCUtil.h"

using namespace std;
using namespace CGCUtil;

//////////////////////////GENERAL UTILITIES /////////////////////////////////////

/// functional class deletes a pointer
/// fun to use w/ for_each template
///
/// I got it from here - Z.F.
/// Newsgroups: comp.lang.c++.moderated
/// From: Didier Trosset <didier-dot-tros...@acqiris.com> - Find messages by this author
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

////////////////////////////////////////////////////////////////////////////////////

void muonCalib::flushHists() {
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

void muonCalib::openHistFile(const string &filename) {
  if (m_histFile.get()) flushHists();

  m_histFilename = filename;

  m_histFile.reset(new TFile(m_histFilename.c_str(), "RECREATE","muonCalibHistograms",9));
}

muonCalib::muonCalib(const vector<string> &digiFilenames,
                     const string &instrument,
                     const vector<int> &towerList,
                     ostream &ostr,
                     const string &timestamp,
                     double adcThresh,
                     double cellHorPitch,
                     double cellVertPitch,
                     double maxAsymLL,
                     double maxAsymLS,
                     double maxAsymSL,
                     double maxAsymSS,
                     double minAsymLL,
                     double minAsymLS,
                     double minAsymSL,
                     double minAsymSS)
  :
  // initialize all member objects
  RootFileAnalysis(vector<string>(0),digiFilenames,vector<string>(0),ostr),
  m_timestamp(timestamp),
  m_instrument(instrument),
  m_towerList(towerList),
  m_digiFilenames(digiFilenames),
  m_adcThresh(adcThresh),
  m_cellHorPitch(cellHorPitch),
  m_cellVertPitch(cellVertPitch),
  m_maxAsymLL(maxAsymLL),
  m_maxAsymLS(maxAsymLS),
  m_maxAsymSL(maxAsymSL),
  m_maxAsymSS(maxAsymSS),
  m_minAsymLL(minAsymLL),
  m_minAsymLS(minAsymLS),
  m_minAsymSL(minAsymSL),
  m_minAsymSS(minAsymSS)
{
  //open up histogram file if desired
  if (m_histFilename.length()) {
    m_histFile.reset(new TFile(m_histFilename.c_str(), "RECREATE","muonCalibHistograms",9));
  }

  // configure ROOT Tree - enable only branches we are going to use.
  if (m_mcEnabled) m_mcChain.SetBranchStatus("*", 0); // disable all branches

  if (m_digiEnabled) {
    m_digiChain.SetBranchStatus("*",0); // disable all branches

    // activate desired brances
    m_digiChain.SetBranchStatus("m_cal*",1);
    m_digiChain.SetBranchStatus("m_eventId", 1);
  }

  if (m_recEnabled) m_recChain.SetBranchStatus("*",0); // disable all branches

}

void muonCalib::freeChildren() {
}

int muonCalib::chkForEvts(int nEvts) {
  Int_t nEntries = getEntries();
  m_ostr << "\nTotal num Events in File is: " << nEntries << endl;

  if (nEntries <= 0 || m_startEvt == nEntries) // have already reached end of chain.
    throw string("No more events available for processing");

  int lastRequested = nEvts + m_startEvt;

  // CASE A: we have enough events
  if (lastRequested <= nEntries) return lastRequested;

  // CASE B: we don't have enough
  int evtsLeft = nEntries - m_startEvt;
  m_ostr << " EOF before " << nEvts << ". Will process remaining " <<
    evtsLeft<< " events." << endl;
  return evtsLeft;
}

UInt_t muonCalib::getEvent(UInt_t ievt) {
  if (m_digiEvt) m_digiEvt->Clear();

  int nb = RootFileAnalysis::getEvent(ievt);
  if (m_digiEvt && nb) { //make sure that m_digiEvt is valid b/c we will assume so after this
    m_evtId = m_digiEvt->getEventId();
    if(m_evtId%1000 == 0)
      m_ostr << " event " << m_evtId << endl;
  }

  return nb;
}

void muonCalib::initRoughPedHists() {
  // DEJA VU?
  if (m_roughPedHists.size() == 0) {
    m_roughPedHists.resize(MAX_FACE_IDX);

    for (int nFace = 0; nFace < MAX_FACE_IDX; nFace++) {
      string tmp("roughpeds_");
      appendFaceStr(nFace,tmp); // append face# to string

      m_roughPedHists[nFace] = new TH1F(tmp.c_str(),
                                        tmp.c_str(),
                                        500,0,1000);
    }
  }else // clear existing histsograms
    for (int nFace = 0; nFace < MAX_FACE_IDX; nFace++)
      m_roughPedHists[nFace]->Reset();
}

void muonCalib::fillRoughPedHists(int nEvts) {
  initRoughPedHists();

  int lastEvt = chkForEvts(nEvts);

  // Basic digi-event loop
  for (Int_t iEvt = m_startEvt; iEvt < lastEvt; iEvt++) {
    if (!getEvent(iEvt)) {
      m_ostr << "Warning, event " << iEvt << " not read." << endl;
      continue;
    }

    const TObjArray* calDigiCol = m_digiEvt->getCalDigiCol();
    if (!calDigiCol) {
      cerr << "no calDigiCol found for event#" << iEvt << endl;
      continue;
    }

    CalDigi *pCalDigi = 0;

    for( int cde_nb=0; (pCalDigi=(CalDigi*) calDigiCol->At(cde_nb)); cde_nb++ ) { //loop through each 'hit' in one event
      CalDigi &calDigi = *pCalDigi; // use reference to avoid -> syntax
      const CalXtalReadout& cRo=*calDigi.getXtalReadout(LEX8); // get LEX8 data

      CalXtalId id(calDigi.getPackedId()); // get interaction information
      int lyr = id.getLayer();
      //int tower = id.getTower();
      int col = id.getColumn();

      float adcP = cRo.getAdc(POS_FACE);
      float adcN = cRo.getAdc(NEG_FACE);

      int histId = getNFace(lyr,col,POS_FACE);
      m_roughPedHists[histId]->Fill(adcP);

      histId = getNFace(lyr,col,NEG_FACE);
      m_roughPedHists[histId]->Fill(adcN);

    }
  }
}

void muonCalib::fitRoughPedHists() {
  m_calRoughPed.resize(MAX_FACE_IDX);
  m_calRoughPedErr.resize(MAX_FACE_IDX);

  for (int nFace = 0; nFace < MAX_FACE_IDX; nFace++) {
    // select histogram from list
    TH1F &h= *m_roughPedHists[nFace];

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
    m_calRoughPed[nFace] =
      ((TF1&)*h.GetFunction("gaus")).GetParameter(1);
    m_calRoughPedErr[nFace] =
      ((TF1&)*h.GetFunction("gaus")).GetParameter(2);
  }
}

void muonCalib::writeRoughPedsTXT(const string &filename) {
  ofstream outfile(filename.c_str());
  if (!outfile.is_open())
    throw string("Unable to open " + filename);

  for(int lyr=0;lyr < N_LYRS; lyr++)
    for(int col=0;col < N_COLS; col++)
      for(int face =0;face < N_FACES; face++) {
        int nFace = getNFace(lyr,col, face);

        outfile << " "<< lyr
                <<" " << col
                <<" " << face
                <<" " << m_calRoughPed[nFace]
                <<" " << m_calRoughPedErr[nFace]
                << endl;

      }
}

void muonCalib::initPedHists() {
  // DEJA VU?
  if (m_pedHists.size() == 0) {
    m_pedHists.resize(MAX_RNG_IDX);

    for (int nRng = 0; nRng < MAX_RNG_IDX; nRng++) {
      string tmp("peds_");
      appendRngStr(nRng,tmp); // append rng# to string

      m_pedHists[nRng] = new TH1F(tmp.c_str(),
                                  tmp.c_str(),
                                  1000,0,1000);
    }
  }
  else // clear existing histsograms
    for (int nRng = 0; nRng < MAX_RNG_IDX; nRng++)
      m_pedHists[nRng]->Reset();
}

void muonCalib::fillPedHists(int nEvts) {
  initPedHists();

  int lastEvt = chkForEvts(nEvts);

  // Basic digi-event loop
  for (Int_t iEvt = m_startEvt; iEvt < lastEvt; iEvt++) {
    if (!getEvent(iEvt)) {
      m_ostr << "Warning, event " << iEvt << " not read." << endl;
      continue;
    }

    const TObjArray* calDigiCol = m_digiEvt->getCalDigiCol();
    if (!calDigiCol) {
      cerr << "no calDigiCol found for event#" << iEvt << endl;
      continue;
    }

    CalDigi *pCalDigi = 0;

    for( int cde_nb=0; (pCalDigi=(CalDigi*)calDigiCol->At(cde_nb)); cde_nb++ ) { //loop through each 'hit' in one event
      CalDigi &calDigi = *pCalDigi; // use reference to avoid -> syntax

      if (calDigi.getMode() != CalXtalId::ALLRANGE)
        throw string("muon calibration requires ALLRANGE Cal data");

      const CalXtalReadout& cRo=*calDigi.getXtalReadout(LEX8); // 1st look at LEX8 vals

      CalXtalId id(calDigi.getPackedId()); // get interaction information
      int lyr = id.getLayer();
      int col = id.getColumn();
      int nXtal = getNXtal(lyr,col);
      int nReadouts = calDigi.getNumReadouts();

      float adcP = cRo.getAdc(POS_FACE);
      float adcN = cRo.getAdc(NEG_FACE);

      // skip outliers (outside of 5 sigma.
      if (fabs(adcN - m_calRoughPed[getNFace(nXtal,NEG_FACE)]) < 5*m_calRoughPedErr[getNFace(nXtal,NEG_FACE)] &&
          fabs(adcP - m_calRoughPed[getNFace(nXtal,POS_FACE)]) < 5*m_calRoughPedErr[getNFace(nXtal,POS_FACE)]) {

        for (int rng = 0; rng < nReadouts; rng++) {
          const CalXtalReadout &readout = *calDigi.getXtalReadout(rng);
          int adc = readout.getAdc(POS_FACE);

          int histId = getNRng(nXtal,POS_FACE, rng);
          m_pedHists[histId]->Fill(adc);

          adc = readout.getAdc(NEG_FACE);
          histId = getNRng(nXtal,NEG_FACE, rng);
          m_pedHists[histId]->Fill(adc);
        }
      }
    }
  }
}

void muonCalib::fitPedHists() {
  m_calPed.resize(MAX_RNG_IDX);
  m_calPedErr.resize(MAX_RNG_IDX);

  TF1 mygaus("mygaus","gaus",0,1000);
  //mygaus.SetParLimits(2,0.5,1000);  // put limit on sigma parameter
  
  for (int nRng = 0; nRng < MAX_RNG_IDX; nRng++) {
    //select histogram from list
    TH1F &h= *m_pedHists[nRng];
    
    // trim outliers
    float av = h.GetMean();float err =h.GetRMS();
    for( int iter=0; iter<3;iter++ ) {
      h.SetAxisRange(av-3*err,av+3*err);
      av = h.GetMean(); err= h.GetRMS();
    }
    
    // reset initial f1 parameters
    mygaus.SetParameter(0,h.GetEntries()/err); // ballpark for height.
    mygaus.SetParameter(1,av);
    mygaus.SetParameter(2,err);

    // run gaussian fit
    float fitWidth = max(3.0*err,5.0); 
    h.Fit("mygaus", "QBI","", av-fitWidth, av+fitWidth);
    h.SetAxisRange(av-150,av+150);

    //assign values to permanet arrays
    m_calPed[nRng] =
      mygaus.GetParameter(1);
    m_calPedErr[nRng] =
      mygaus.GetParameter(2);
  }
}

void muonCalib::writePedsTXT(const string &filename) {
  ofstream outfile(filename.c_str());
  if (!outfile.is_open())
    throw string("Unable to open " + filename);

  for(int lyr=0;lyr <N_LYRS;lyr++)
    for(int col=0;col<N_COLS;col++)
      for(int face =0;face <N_FACES;face++)
        for(int rng= 0;rng < N_RNGS; rng++) {
          int nRng = getNRng(lyr,col,face,rng);

          outfile << " "<< lyr
                  << " " << col
                  << " " << face
                  << " " << rng
                  << " " << m_calPed[nRng]
                  << " " << m_calPedErr[nRng]
                  << endl;
        }
}

void muonCalib::readCalPeds(const string &filename) {
  m_calPed.resize(MAX_RNG_IDX);
  m_calPedErr.resize(MAX_RNG_IDX);

  ifstream infile(filename.c_str());
  if (!infile.is_open())
    throw string("Unable to open " + filename);

  unsigned nRead = 0;
  while(infile.good()) {
    float av,err;int lyr,col,face, rng;
    infile >> lyr
           >> col
           >> face
           >> rng
           >> av
           >> err;
    if (infile.fail()) break; // quit once we can't read any more values
    nRead++;

    int nRng = getNRng(lyr,col,face,rng);
    m_calPed[nRng]= av;
    m_calPedErr[nRng]= err;
  }

  if (nRead != m_calPed.size()) {
    ostringstream temp;
    temp << "CalPed file '" << filename << "' is incomplete: " << nRead
           << " pedestal values read, " << m_calPed.size() << " vals required.";
    throw temp.str();
  }
}

void muonCalib::readIntNonlin(const string &filename) {
  m_calInlADC.resize(MAX_DIODE_IDX);
  m_calInlDAC.resize(MAX_DIODE_IDX);

  ifstream infile(filename.c_str());
  if (!infile.is_open())
    throw string("Unable to open " + filename);

  string lineStr;
  int nLine = 0;

  getline(infile, lineStr);
  while(infile.good()) { //loop on each line.
    nLine++; // only increment line# if getline() was successful.
    int lyr, col, face, rng;
    float adc, dac;

    istringstream lineStream(lineStr);

    lineStream >> lyr
               >> col
               >> face
               >> rng
               >> adc
               >> dac; // first 4 columns in file id the xtal & adc range
    if (lineStream.fail())
      throw string("Inl file '" + filename + "' is incomplete.");

    // we are only interested in X8 splines for muonCalibration
    if (rng == LEX8 || rng == HEX8) {

      int nDiode = getNDiode(lyr,col,face,rng2diode(rng));

      // load values into vectors
      m_calInlADC[nDiode].push_back(adc);
      m_calInlDAC[nDiode].push_back(dac);
    }

    getline(infile, lineStr);
  }

  loadInlSplines();
}

void muonCalib::loadInlSplines() {
  m_inlSplines.resize(MAX_DIODE_IDX);

  // ROOT functions take double arrays, not vectors so we need to copy each vector into an array
  // before loading it into a ROOT spline
  int arraySize = 100; // 100 is good first guess for array size, resize later if needbe
  double *dacs = new double[arraySize];
  double *adcs = new double[arraySize];

  for (int nDiode = 0; nDiode < MAX_DIODE_IDX; nDiode++) {

    vector<float> &adcVec = m_calInlADC[nDiode];
    int nADC = adcVec.size();

    // Load up
    vector<float> &dacVec = m_calInlDAC[nDiode];
    int nDAC = dacVec.size();

    if (nADC == 0 || nDAC == 0)
      throw string("Zero elements for nDiode = " + nDiode);

    if (nADC != nDAC)
      throw string("nDAC != nADC for nDiode = " + nDiode);

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
    string name("intNonlin_");
    appendDiodeStr(nDiode,name);

    // create spline object.
    TSpline3 *mySpline= new TSpline3(name.c_str(), dacs, adcs, nADC);

    mySpline->SetName(name.c_str());
    m_inlSplines[nDiode] = mySpline;
  }

  // cleanup
  delete dacs;
  delete adcs;
}

void muonCalib::hitSummary::clear() {
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
  count = 0;
  nLyrsX = 0;
  nLyrsY = 0;
  nColsX = 0;
  nColsY = 0;
  maxPerLyr = 0;
  maxPerLyrX = 0;
  maxPerLyrY = 0;
  firstColX = 0;
  firstColY = 0;

  goodXTrack = false;
  goodYTrack = false;
  status = false;
}

void muonCalib::summarizeHits(hitSummary &hs) {
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
    // get geometry id for hit.
    CalXtalId id(calDigi.getPackedId());
    int lyr = id.getLayer();
    int col = id.getColumn();
    int nXtal = getNXtal(lyr,col);

    // check that we are in 4-range readout mode
    int nReadouts = calDigi.getNumReadouts();
    if (nReadouts != 4)
      throw string("Not in 4-range readout mode");

    // load up all adc values for each xtal diode
    // also ped subtraced adc values.
    for (int diode = 0; diode < N_DIODES; diode++) { // loop over 2 diodes
      int rng = diode2X8rng(diode);
      const CalXtalReadout& readout = *calDigi.getXtalReadout(rng);

      for (int face = 0; face < N_FACES; face++) {
        int nDiode = getNDiode(nXtal,face,diode);
        int nRng = getNRng(nXtal,face,rng);
        float adc = readout.getAdc((CalXtalId::XtalFace)face); // raw adc
        hs.adc_ped[nDiode] = adc - m_calPed[nRng];// subtract pedestals
        //m_ostr << "nDiode " << nDiode << " nRng " << nRng << " adc " << hs.adc[nDiode] << " ped " << m_calPed[nRng]
        // << " adc_ped " << hs.adc_ped[nDiode] << " calCorr " << m_calCorr[nDiode] << " adc_corr " << hs.adc_corr[nDiode] << endl;
      }
    }

    // DO WE HAVE A HIT? (sum up both ends LEX8)
    if (hs.adc_ped[getNDiode(nXtal,POS_FACE,rng2diode(LEX8))] +
        hs.adc_ped[getNDiode(nXtal,NEG_FACE,rng2diode(LEX8))] > m_adcThresh) {

      hs.count++; // increment total # hits

      //m_ostr << "nHits " << hs.count;// << endl;

      // used to determine if xtal is x or y layer
      //m_ostr << " lyr " << lyr << " col " << col ;//<< " xtal " << nXtal << endl;
      if (isXlyr(lyr)) { // X layer
        hs.perLyrX[lyr2Xlyr(lyr)]++;
        hs.perColX[col]++;
        hs.hitListX.push_back(nXtal);
        //m_ostr << " X" << " xLyrId " << lyr2Xlyr(lyr) << " perLyrX " << hs.perLyrX[lyr2Xlyr(lyr)] << " perCol " << hs.perColX[col] << endl;
      } else { // y layer
        hs.perLyrY[lyr2Ylyr(lyr)]++;
        hs.perColY[col]++;
        hs.hitListY.push_back(nXtal);
        //m_ostr << " Y" << " YLyrId " << lyr2Ylyr(lyr) << " perLyrY " << hs.perLyrY[lyr2Ylyr(lyr)] << " perCol " << hs.perColY[col] << endl;
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

    // find 1st col hit in each direction (will be only col hit if event is good)
    // find_if returns iterator to 1st true element, subtract begin() iterator to get distance or index
    hs.firstColX = find_if(hs.perColX.begin(), hs.perColX.end(), bind2nd(greater<short>(), 0)) - hs.perColX.begin();
    hs.firstColY = find_if(hs.perColY.begin(), hs.perColY.end(), bind2nd(greater<short>(), 0)) - hs.perColY.begin();

    // m_ostr << "mplx " << hs.maxPerLyrX << " y " << hs.maxPerLyrY << " mpl " << hs.maxPerLyr
    // << " nlx " << hs.nLyrsX << " y " << hs.nLyrsY << " ncx " << hs.nColsX << " y " << hs.nColsY
    // << " fcx " << hs.firstColX << " y " << hs.firstColY << endl;


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

void muonCalib::initAsymHists(bool genOptHists) {
  // DEJA VU?
  if (m_asymProfsLL.size() == 0) {
    m_asymProfsLL.resize(MAX_XTAL_IDX);
    m_asymProfsLS.resize(MAX_XTAL_IDX);
    m_asymProfsSL.resize(MAX_XTAL_IDX);
    m_asymProfsSS.resize(MAX_XTAL_IDX);
    if (genOptHists) {
      m_asymDACHists.resize(MAX_DIODE_IDX);
      m_logratHistsLL.resize(MAX_XTAL_IDX);
      m_logratHistsLS.resize(MAX_XTAL_IDX);
      m_logratHistsSL.resize(MAX_XTAL_IDX);
      m_logratHistsSS.resize(MAX_XTAL_IDX);
    }

    // PER XTAL LOOP
    for (int nXtal = 0; nXtal < MAX_XTAL_IDX; nXtal++) {
      string tmp;

      // LARGE-LARGE ASYM
      tmp = "asymLL_";
      appendXtalStr(nXtal,tmp);// append xtal# to string
      // columns are #'d 0-11, hist contains 1-10. .5 & 10.5 limit put 1-10 at center of bins
      m_asymProfsLL[nXtal] = new TProfile(tmp.c_str(),
                                          tmp.c_str(),
                                          N_ASYM_PTS,.5,10.5);

      // LARGE-SMALL ASYM
      tmp = "asymLS_";
      appendXtalStr(nXtal,tmp); // append xtal# to string
      m_asymProfsLS[nXtal] = new TProfile(tmp.c_str(),
                                          tmp.c_str(),
                                          N_ASYM_PTS,.5,10.5);

      // SMALL-LARGE ASYM
      tmp = "asymSL_";
      appendXtalStr(nXtal,tmp); // append xtal# to string
      m_asymProfsSL[nXtal] = new TProfile(tmp.c_str(),
                                          tmp.c_str(),
                                          N_ASYM_PTS,.5,10.5);

      // SMALL-SMALL ASYM
      tmp = "asymSS_";
      appendXtalStr(nXtal,tmp); // append xtal# to string
      m_asymProfsSS[nXtal] = new TProfile(tmp.c_str(),
                                          tmp.c_str(),
                                          N_ASYM_PTS,.5,10.5);

      // Optional logratHists
      if (genOptHists) {
        // LARGE-LARGE LOGRAT
        tmp = "logratLL_";
        appendXtalStr(nXtal,tmp);// append xtal# to string
        // columns are #'d 0-11, hist contains 1-10. .5 & 10.5 limit put 1-10 at center of bins
        m_logratHistsLL[nXtal] = new TH2F(tmp.c_str(),
                                          tmp.c_str(),
                                          N_ASYM_PTS,.5,10.5,
                                          100,-.1,.1);
        m_logratHistsLL[nXtal]->SetBit(TH1::kCanRebin);
        
        // LARGE-SMALL LOGRAT
        tmp = "logratLS_";
        appendXtalStr(nXtal,tmp); // append xtal# to string
        m_logratHistsLS[nXtal] = new TH2F(tmp.c_str(),
                                          tmp.c_str(),
                                          N_ASYM_PTS,.5,10.5,
                                          100,1.5,1.6);
        m_logratHistsLS[nXtal]->SetBit(TH1::kCanRebin);

        // SMALL-LARGE LOGRAT
        tmp = "logratSL_";
        appendXtalStr(nXtal,tmp); // append xtal# to string
        m_logratHistsSL[nXtal] = new TH2F(tmp.c_str(),
                                          tmp.c_str(),
                                          N_ASYM_PTS,.5,10.5,
                                          100,-2.0,-1.9);
        m_logratHistsSL[nXtal]->SetBit(TH1::kCanRebin);

        // SMALL-SMALL LOGRAT
        tmp = "logratSS_";
        appendXtalStr(nXtal,tmp); // append xtal# to string
        m_logratHistsSS[nXtal] = new TH2F(tmp.c_str(),
                                          tmp.c_str(),
                                          N_ASYM_PTS,.5,10.5,
                                          100,-.1,.1);
        m_logratHistsSS[nXtal]->SetBit(TH1::kCanRebin);
      }

      if (genOptHists)
        for (int face = 0; face < N_FACES; face++) {
          for (int diode = 0; diode < N_DIODES; diode++) {
            int nDiode = getNDiode(nXtal, face, diode);

            tmp = "asymdac_";
            appendDiodeStr(nDiode,tmp); // append diode# to string
            m_asymDACHists[nDiode] = new TH1F(tmp.c_str(),
                                              tmp.c_str(),
                                              100,0,0);

          }
        }
    }
  } else // clear existing histsograms
    for (int nXtal = 0; nXtal < MAX_XTAL_IDX; nXtal++) {
      m_asymProfsLL[nXtal]->Reset();
      m_asymProfsLS[nXtal]->Reset();
      m_asymProfsSL[nXtal]->Reset();
      m_asymProfsSS[nXtal]->Reset();
      if (genOptHists) {
        m_logratHistsLL[nXtal]->Reset();
        m_logratHistsLS[nXtal]->Reset();
        m_logratHistsSL[nXtal]->Reset();
        m_logratHistsSS[nXtal]->Reset();
        for (int face = 0; face < N_FACES; face++) {
          for (int diode = 0; diode < N_DIODES; diode++)
            m_asymDACHists[getNDiode(nXtal,face,diode)]->Reset();
        }
      }
    }
}

/// \note 'test direction' refers to the direction w/ 4 vertical hits.  In asymetry calibration this direction is used to get position info for the 'longitudinal direction'
/// \note 'longitudinal direction' refers to the direction with the hits that are actually measured for asymetry.

void muonCalib::fillAsymHists(int nEvts, bool genOptHists) {
  initAsymHists(genOptHists);

  int lastEvt = chkForEvts(nEvts);
  hitSummary hs;

  int nGoodDirs = 0; // count total # of events used
  int nXDirs = 0;
  int nYDirs = 0;
  long nHits = 0; // count total # of xtals measured
  int nBadHits = 0;
  int nBadAsymSS = 0;
  int nBadAsymLL = 0;
  int nBadAsymSL = 0;
  int nBadAsymLS = 0;
  int nBadDACs = 0;

  // Basic digi-event loop
  for (Int_t iEvt = m_startEvt; iEvt < lastEvt; iEvt++) {
    if (!getEvent(iEvt)) {
      m_ostr << "Warning, event " << iEvt << " not read." << endl;
      continue;
    }

    summarizeHits(hs);
    
    for (int dir = 0; dir < N_DIRS; dir++) {
      int pos;
      vector<int> *pHitList, *pHitListLong;

      // DIRECTION SPECIFIC SETUP //
      if (dir == X_DIR) {
        if (!hs.goodXTrack) continue;  // skip this direction if track is bad
        pos = hs.firstColX;
        pHitList = &hs.hitListX; // hit list in test direction 
        pHitListLong = &hs.hitListY; // long direction
      } else { // Y_DIR
        if (!hs.goodYTrack) continue; // skip this direction if track is bad
        pos = hs.firstColY;
        pHitList = &hs.hitListY; // hit list in test direction 
        pHitListLong = &hs.hitListX; // long direction
      }

      // skip extreme ends of xtal, as variance is high.
      if (pos == 0 || pos == 11) continue;
      nGoodDirs++;

      // use references to avoid -> notation
      vector<int> &hitListLong = *pHitListLong; 

      // loop through each longitudinal hit
      for (unsigned i = 0; i < hitListLong.size(); i++) {
        int nXtal = hitListLong[i];

        // calculate the 4 dac vals
        float dacPosLarge = adc2dac(nXtal, hs.adc_ped[getNDiode(nXtal,POS_FACE,LARGE_DIODE)]);
        float dacPosSmall = adc2dac(nXtal, hs.adc_ped[getNDiode(nXtal,POS_FACE,SMALL_DIODE)]);
        float dacNegLarge = adc2dac(nXtal, hs.adc_ped[getNDiode(nXtal,NEG_FACE,LARGE_DIODE)]);
        float dacNegSmall = adc2dac(nXtal, hs.adc_ped[getNDiode(nXtal,NEG_FACE,SMALL_DIODE)]);

        // fill optional dacVal histograms
        if (genOptHists) {
          m_asymDACHists[getNDiode(nXtal,POS_FACE,LARGE_DIODE)]->Fill(dacPosLarge);
          m_asymDACHists[getNDiode(nXtal,POS_FACE,SMALL_DIODE)]->Fill(dacPosSmall);
          m_asymDACHists[getNDiode(nXtal,NEG_FACE,LARGE_DIODE)]->Fill(dacNegLarge);
          m_asymDACHists[getNDiode(nXtal,NEG_FACE,SMALL_DIODE)]->Fill(dacNegSmall);
        }

        // CHECK INVALID VALUES //
        if (dacPosSmall <= 0 || dacPosLarge <= 0 ||
            dacNegSmall <= 0 || dacNegLarge <= 0) {
          nBadHits++;
          nBadDACs++;
          continue;
        }

        // calcuate the 4 log ratios = log(POS/NEG)
        float asymLL = log(dacPosLarge/dacNegLarge);
        float asymLS = log(dacPosLarge/dacNegSmall);
        float asymSL = log(dacPosSmall/dacNegLarge);
        float asymSS = log(dacPosSmall/dacNegSmall);
      
        if (genOptHists) {
          m_logratHistsLL[nXtal]->Fill(pos, asymLL);
          m_logratHistsLS[nXtal]->Fill(pos, asymLS);
          m_logratHistsSL[nXtal]->Fill(pos, asymSL);
          m_logratHistsSS[nXtal]->Fill(pos, asymSS);
        }
   
        // check for asym vals which are way out of range
        
        // test LL first since it is most likely
        if (asymLL < m_minAsymLL || asymLL > m_maxAsymLL) {
          // m_ostr << " **AsymLL out-of-range evt=" << m_evtId
          // << " lyr=" << lyr << " col=" << col
          // << " pos=" << pos
          // << " dacP=" << dacPosLarge << " dacN=" << dacNegLarge
          // << " asym=" << asymLL << endl;
          nBadHits++;
          nBadAsymLL++;
          continue;
        }
        // test SS 2nd since bad LL or bad SS should leave  only small #
        // of bad LS & SL's
        if (asymSS < m_minAsymSS || asymSS > m_maxAsymSS) {
          /*m_ostr << " **AsymSL out-of-range evt=" << m_evtId
            << " lyr=" << lyr << " col=" << col
            << " pos=" << pos
            << " dacP=" << dacPosSmall << " dacN=" << dacNegLarge
            << " asym=" << asymSL << endl;*/
          nBadHits++;
          nBadAsymSL++;
          continue;
        }
        if (asymLS < m_minAsymLS || asymLS > m_maxAsymLS) {
          // m_ostr << " **AsymSS out-of-range evt=" << m_evtId
          // << " lyr=" << lyr << " col=" << col
          // << " pos=" << pos
          // << " dacP=" << dacPosSmall << " dacN=" << dacNegSmall
          // << " asym=" << asymSS << endl;
          nBadHits++;
          nBadAsymSS++;
          continue;
        }
        if (asymSL < m_minAsymSL || asymSL > m_maxAsymSL) {
          /*m_ostr << " **AsymLS out-of-range evt=" << m_evtId
            << " lyr=" << lyr << " col=" << col
            << " pos=" << pos
            << " dacP=" << dacPosLarge << " dacN=" << dacNegSmall
            << " asym=" << asymLS << endl;*/
          nBadHits++;
          nBadAsymLS++;
          continue;
        }
      
        // pos - 5.5 value will range from -4.5 to +4.5 in xtal width units
        m_asymProfsLL[nXtal]->Fill(pos, asymLL);
        m_asymProfsLS[nXtal]->Fill(pos, asymLS);
        m_asymProfsSL[nXtal]->Fill(pos, asymSL);
        m_asymProfsSS[nXtal]->Fill(pos, asymSS);

        nHits++;
      } // per hit loop
    } // per direction loop
  } // per event loop

  m_ostr << "Asymmetry histograms filled nEvents=" << nGoodDirs
         << " nXDirs=" << nXDirs
         << " nYDirs=" << nYDirs << endl;
  m_ostr << " nHits measured=" << nHits
         << " Bad hits=" << nBadHits
         << " asym out-of-range LL=" << nBadAsymLL
         << " SS=" << nBadAsymSS
         << " SL=" << nBadAsymSL
         << " LS=" << nBadAsymLS
         << endl;
}

void muonCalib::populateAsymArrays() {
  m_calAsymLL.resize(MAX_XTAL_IDX);
  m_calAsymLS.resize(MAX_XTAL_IDX);
  m_calAsymSL.resize(MAX_XTAL_IDX);
  m_calAsymSS.resize(MAX_XTAL_IDX);

  m_calAsymLLErr.resize(MAX_XTAL_IDX);
  m_calAsymLSErr.resize(MAX_XTAL_IDX);
  m_calAsymSLErr.resize(MAX_XTAL_IDX);
  m_calAsymSSErr.resize(MAX_XTAL_IDX);

  // PER XTAL LOOP
  for (int nXtal = 0; nXtal < MAX_XTAL_IDX; nXtal++) {

    // retrieve profile objects for this xtal
    TProfile &profLL = *m_asymProfsLL[nXtal];
    TProfile &profLS = *m_asymProfsLS.at(nXtal);
    TProfile &profSL = *m_asymProfsSL[nXtal];
    TProfile &profSS = *m_asymProfsSS.at(nXtal);

    // int lyr = nXtal2lyr(nXtal); int col = nXtal2col(nXtal);
    // m_ostr << "Asym entries lyr=" << lyr << " col=" << col
    // << " LL=" << profLL->GetEntries() << endl;
    // m_ostr << " per bin ";
    // for (int i = 0; i < N_ASYM_PTS; i++)
    // m_ostr << " " << i << "," << profLL.GetBinEntries(i+1);
    // m_ostr << endl;

    // ensure proper vector size
    m_calAsymLL[nXtal].resize(N_ASYM_PTS);
    m_calAsymLS[nXtal].resize(N_ASYM_PTS);
    m_calAsymSL[nXtal].resize(N_ASYM_PTS);
    m_calAsymSS[nXtal].resize(N_ASYM_PTS);

    m_calAsymLLErr[nXtal].resize(N_ASYM_PTS);
    m_calAsymLSErr[nXtal].resize(N_ASYM_PTS);
    m_calAsymSLErr[nXtal].resize(N_ASYM_PTS);
    m_calAsymSSErr[nXtal].resize(N_ASYM_PTS);

    // loop through all N_ASYM_PTS bins in asymmetry profile
    for (int i = 0; i < N_ASYM_PTS; i++) {
      m_calAsymLL[nXtal][i] = profLL.GetBinContent(i+1); // HISTOGRAM BINS START AT 1 NOT ZERO!
      m_calAsymLS[nXtal][i] = profLS.GetBinContent(i+1);
      m_calAsymSL[nXtal][i] = profSL.GetBinContent(i+1);
      m_calAsymSS[nXtal][i] = profSS.GetBinContent(i+1);

      m_calAsymLLErr[nXtal][i] = profLL.GetBinError(i+1);
      m_calAsymLSErr[nXtal][i] = profLS.GetBinError(i+1);
      m_calAsymSLErr[nXtal][i] = profSL.GetBinError(i+1);
      m_calAsymSSErr[nXtal][i] = profSS.GetBinError(i+1);
    }
  }
}

double muonCalib::adc2dac(int nDiode, double adc) {
  return m_inlSplines[nDiode]->Eval(adc);
}

void muonCalib::writeAsymTXT(const string &filenameLL,
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
  for (int lyr = 0; lyr < N_LYRS; lyr++)
    for (int col = 0; col < N_COLS; col++) {
      int nXtal = getNXtal(lyr,col);

      outfileLL << lyr << " " << col;
      outfileLS << lyr << " " << col;
      outfileSL << lyr << " " << col;
      outfileSS << lyr << " " << col;

      // loop through all N_ASYM_PTS points in each asymmetry curve
      for (int i = 0; i < N_ASYM_PTS; i++) {
        outfileLL << " " << m_calAsymLL[nXtal][i] << " " << m_calAsymLLErr[nXtal][i];
        outfileLS << " " << m_calAsymLS[nXtal][i] << " " << m_calAsymLSErr[nXtal][i];
        outfileSL << " " << m_calAsymSL[nXtal][i] << " " << m_calAsymSLErr[nXtal][i];
        outfileSS << " " << m_calAsymSS[nXtal][i] << " " << m_calAsymSSErr[nXtal][i];
      }

      outfileLL << endl;
      outfileLS << endl;
      outfileSL << endl;
      outfileSS << endl;
    }
}

void muonCalib::readAsymTXT(const string &filenameLL,
                            const string &filenameLS,
                            const string &filenameSL,
                            const string &filenameSS) {

  m_calAsymLL.resize(MAX_XTAL_IDX);
  m_calAsymLS.resize(MAX_XTAL_IDX);
  m_calAsymSL.resize(MAX_XTAL_IDX);
  m_calAsymSS.resize(MAX_XTAL_IDX);

  m_calAsymLLErr.resize(MAX_XTAL_IDX);
  m_calAsymLSErr.resize(MAX_XTAL_IDX);
  m_calAsymSLErr.resize(MAX_XTAL_IDX);
  m_calAsymSSErr.resize(MAX_XTAL_IDX);

  // ASYM (4-TYPES) //

  // we go through the same exact sequence for each of the the 4 asym files.
  for (int nFile = 0; nFile < 4; nFile++) {
    int lyr, col;
    int nXtal;
    unsigned nRead = 0;
    vector<vector<float > > *calVec, *errVec;
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
      nXtal = getNXtal(lyr,col);

      // ensure proper vector size
      (*calVec)[nXtal].resize(N_ASYM_PTS);
      (*errVec)[nXtal].resize(N_ASYM_PTS);

      // i'm expecting 10 asymmetry values
      for (int i = 0; i < N_ASYM_PTS; i++) {
        infile >> (*calVec)[nXtal][i];  // read in value
        infile >> (*errVec)[nXtal][i];  // read in error val
      }
      if (infile.fail()) break; // quit once we can't read any more values

      nRead++;
    }

    // check that we got all the values
    if (nRead != (*calVec).size()) {
      ostringstream temp;
      temp << "File '" << filename << "' is incomplete: " << nRead
             << " vals read, " << m_calAsymLL.size() << " vals expected.";
      throw temp.str();
    }
  } //for (nFile 0 to 4)
}

void muonCalib::loadA2PSplines() {
  m_asym2PosSplines.resize(MAX_XTAL_IDX);

  // create position (Y-axis) array
  double pos[N_ASYM_PTS+2]; // linearly interpolate for 1st and last points (+2 points)
  for (int i = 0; i < N_ASYM_PTS+2; i++) pos[i] = i + 0.5; // (center of the column)
  double asym[N_ASYM_PTS+2];

  // PER XTAL LOOP
  for (int nXtal = 0; nXtal < MAX_XTAL_IDX; nXtal++) {
    // copy asym vector into middle of array
    vector<float> &asymVec = m_calAsymLL[nXtal];
    copy(asymVec.begin(),asymVec.end(),asym+1);

    // interpolate 1st & last points
    asym[0] = 2*asym[1]-asym[2];
    asym[N_ASYM_PTS+1] = 2*asym[N_ASYM_PTS]-asym[N_ASYM_PTS-1];

    //generate splinename
    string name("asym2pos_");
    appendXtalStr(nXtal,name);

    // create spline object
    TSpline3 *mySpline= new TSpline3(name.c_str(), asym, pos, N_ASYM_PTS+2);
    mySpline->SetName(name.c_str());
    m_asym2PosSplines[nXtal] = mySpline;
  }
}

double muonCalib::asym2pos(int nXtal, double asym) {
  return m_asym2PosSplines[nXtal]->Eval(asym);
}

void muonCalib::initMPDHists() {
  // DEJA VU?
  if (m_dacLLHists.size() == 0) {
    m_dacLLHists.resize(MAX_XTAL_IDX);
    m_dacL2SProfs.resize(MAX_XTAL_IDX);

    for (int nXtal = 0; nXtal < MAX_XTAL_IDX; nXtal++) {

      // LARGE-LARGE DAC
      string tmp("dacLL_");
      appendXtalStr(nXtal,tmp); // append xtal# to string
      m_dacLLHists[nXtal] = new TH1F(tmp.c_str(),
                                     tmp.c_str(),
                                     200,0,100);

      // DAC L2S RATIO
      tmp = "dacL2S_";
      appendXtalStr(nXtal,tmp); // append xtal# to string
      m_dacL2SProfs[nXtal] = new TProfile(tmp.c_str(),
                                          tmp.c_str(),
                                          N_L2S_PTS,10,60);
    }
  }
  else // clear existing histsograms
    for (int nXtal = 0; nXtal < MAX_XTAL_IDX; nXtal++) {
      m_dacLLHists[nXtal]->Reset();
      m_dacL2SProfs[nXtal]->Reset();
    }

}

void muonCalib::fillMPDHists(int nEvts) {
  initMPDHists();

  int lastEvt = chkForEvts(nEvts);
  hitSummary hs;

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
  int nXEvts = 0; // count total # of X events used
  int nYEvts = 0; // count total # of Y events used
  long nXtals = 0; // count total # of xtals measured

  // NUMERIC CONSTANTS
  // converts between lyr/col units & mm
  // real trig is needed for pathlength calculation
  double slopeFactor = m_cellHorPitch/m_cellVertPitch;

  // Basic digi-event loop
  for (Int_t iEvt = m_startEvt; iEvt < lastEvt; iEvt++) {
    if (!getEvent(iEvt)) {
      m_ostr << "Warning, event " << iEvt << " not read." << endl;
      continue;
    }

    summarizeHits(hs);

    // CHECK BOTH DIRECTIONS FOR USABLE EVENT
    for (int dir = 0; dir < N_DIRS; dir++) {

      // skip if we don't have a good track
      if (dir == X_DIR && !hs.goodXTrack) continue;
      else if (dir == Y_DIR && !hs.goodYTrack) continue;

      if (dir == X_DIR) nXEvts++;
      else nYEvts++;

      // set up proper hitLists for processing based on direction
      vector<int> &hitList = // hit list in current direction for gain calib
        (dir == X_DIR) ? hs.hitListX : hs.hitListY;
      vector<int> &hitListLong = // hit list in longitudinal direction
        (dir == X_DIR) ? hs.hitListY : hs.hitListX;

      // need at least 2 points to get a longitudinal track
      if (hitListLong.size() < 2) continue;

      graph.Set(hitListLong.size());

      // fill in each point val
      for (unsigned i = 0; i < hitListLong.size(); i++) {
        int nXtal = hitListLong[i];
        int lyr = nXtal2lyr(nXtal);
        int col = nXtal2col(nXtal);
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
        int nXtal = hitList[i];
        int lyr = nXtal2lyr(nXtal);

        float hitPos = lineFunc.Eval(lyr); // find column for given lyr

        //throw out event if energy centroid is in column 0 or 11 (3cm from end)
        if (hitPos < 1 || hitPos > 10) {
          hitList.erase(hitList.begin()+i);
          i--;
        }
      }

      // occasionally there will be no good hits!
      if (hitList.size() < 1) continue;

      vector<float> dacsPosLarge(hitList.size());
      vector<float> dacsNegLarge(hitList.size());
      vector<float> dacsPosSmall(hitList.size());
      vector<float> dacsNegSmall(hitList.size());

      // now that we have eliminated events on the ends of xtals, we can use
      // asymmetry to get a higher precision slope
      // we'll keep the dac vals while we're at it since some are used
      // more than once
      graph.Set(hitList.size());
      for (unsigned i = 0; i < hitList.size(); i++) {
        int nXtal = hitList[i];
        int lyr = nXtal2lyr(nXtal);

        // calculate DAC vals
        dacsPosLarge[i] = adc2dac(nXtal, hs.adc_ped[getNDiode(nXtal,POS_FACE,LARGE_DIODE)]);
        dacsNegLarge[i] = adc2dac(nXtal, hs.adc_ped[getNDiode(nXtal,NEG_FACE,LARGE_DIODE)]);
        dacsPosSmall[i] = adc2dac(nXtal, hs.adc_ped[getNDiode(nXtal,POS_FACE,SMALL_DIODE)]);
        dacsNegSmall[i] = adc2dac(nXtal, hs.adc_ped[getNDiode(nXtal,NEG_FACE,SMALL_DIODE)]);

        // calcuate the log ratio = log(POS/NEG)
        float asymLL = log(dacsPosLarge[i]/dacsNegLarge[i]);

        // get new position from asym
        float hitPos = asym2pos(nXtal,asymLL);

        graph.SetPoint(i,lyr,hitPos);
      }

      // fit straight line through graph
      graph.Fit(&lineFunc,"WQN");
      lineSlope = lineFunc.GetParameter(1);

      float tan = lineSlope*slopeFactor; //slope = rise/run = dy/dx = colPos/lyrNum
      float sec = sqrt(1 + tan*tan); //sec proportional to hyp which is pathlen.

      // poulate histograms & apply pathlen correction
      int nHits = hitList.size();
      for (int i = 0; i < nHits; i++) {
        // calculate dacs
        int nXtal = hitList[i];

        // apply pathlength correction
        dacsPosLarge[i] /= sec;
        dacsNegLarge[i] /= sec;
        dacsPosSmall[i] /= sec;
        dacsNegSmall[i] /= sec;

        float meanDACLarge = sqrt(dacsPosLarge[i]*dacsNegLarge[i]);
        float meanDACSmall = sqrt(dacsPosSmall[i]*dacsNegSmall[i]);

        // Load meanDAC Histogram
        m_dacLLHists[nXtal]->Fill(meanDACLarge);

        // load dacL2S profile
        m_dacL2SProfs[nXtal]->Fill(meanDACLarge,meanDACSmall);

        nXtals++;
      } // populate histograms
    } // direction loop
  } // event loop

  m_ostr << "MPD histograms filled nXEvts=" << nXEvts << " nYEvenvts=" << nYEvts << " nXtals measured " << nXtals << endl;
}

void muonCalib::fitMPDHists() {
  m_calMPDLarge.resize(MAX_XTAL_IDX);
  m_calMPDSmall.resize(MAX_XTAL_IDX);
  m_calMPDLargeErr.resize(MAX_XTAL_IDX);
  m_calMPDSmallErr.resize(MAX_XTAL_IDX);

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
  for (int nXtal = 0; nXtal < MAX_XTAL_IDX; nXtal++) {
    // retrieve Large diode DAC histogram
    TH1F& h = *m_dacLLHists[nXtal];

    // LANDAU fit for muon peak (limit outliers by n*err)
    float ave = h.GetMean();
    float err = h.GetRMS();
    h.Fit("landau", "Q", "", ave-2*err, ave+3*err);
    float mean = (h.GetFunction("landau"))->GetParameter(1);
    float sigma = (h.GetFunction("landau"))->GetParameter(2);

    m_calMPDLarge[nXtal] = 11.2/mean;
    m_calMPDLargeErr[nXtal] = m_calMPDLarge[nXtal] * sigma/mean; // keeps sigma proportional

    // LARGE 2 SMALL Ratio
    TProfile& p = *m_dacL2SProfs[nXtal]; // get profile

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
      ostringstream temp;
      temp << "Unable to find small diode MPD for xtal=" << nXtal
           << " due to empty histogram." << endl;
      throw temp.str();
    }

    // fit straight line
    graph.Fit(&lineFunc,"WQN");

    // get slope
    float large2small = lineFunc.GetParameter(1);
    
    // in order to combine slope & MPD error for final error
    // I need the relative error for both values - so sayeth sasha
    float relLineErr = lineFunc.GetParError(1)/large2small;
    float relMPDErr = m_calMPDLargeErr[nXtal]/m_calMPDLarge[nXtal];

    m_calMPDSmall[nXtal] = m_calMPDLarge[nXtal]*large2small;
    m_calMPDSmallErr[nXtal] = m_calMPDSmall[nXtal]*
		sqrt(relLineErr*relLineErr + relMPDErr*relMPDErr);
  }
}

void muonCalib::writeMPDTXT(const string &filenameL, const string &filenameS) {
  ofstream outfileL(filenameL.c_str());
  ofstream outfileS(filenameS.c_str());
  if (!outfileL.is_open())
    throw string("Unable to open " + filenameL);
  if (!outfileS.is_open())
    throw string("Unable to open " + filenameS);


  for(int lyr=0;lyr < N_LYRS; lyr++)
    for(int col=0;col < N_COLS; col++) {
      int nXtal = getNXtal(lyr,col);
      outfileL << " " << lyr
               << " " << col
               << " " << m_calMPDLarge[nXtal]
               << " " << m_calMPDLargeErr[nXtal]
               << endl;
      outfileS << " " << lyr
               << " " << col
               << " " << m_calMPDSmall[nXtal]
               << " " << m_calMPDSmallErr[nXtal]
               << endl;
    }
}


void muonCalib::writePedsXML(const string &filename, const string &dtdFilename) {
  ofstream outfile(filename.c_str());
  ifstream dtdFile(dtdFilename.c_str());
  if (!outfile.is_open())
    throw string("Unable to open " + filename);
  if (!dtdFile.is_open())
    throw string("Unable to open " + dtdFilename);

  outfile << "<?xml version=\"1.0\"?>" << endl;
  
  // INSERT ENTIRE .DTD FILE //
  outfile << "<!DOCTYPE calCalib [";
  string temp;
  while (dtdFile.good()) {
    getline(dtdFile, temp);
    if (dtdFile.fail()) continue; // bad get
    outfile << temp << endl;
  }
  outfile << "]>" << endl;

  outfile << "<calCalib>" << endl;
  outfile << " <generic instrument=\"" << m_instrument <<"\" timestamp=\""<< m_timestamp <<"\" calibType=\"CAL_Ped\" fmtVersion=\"v2r2\">" << endl;
  outfile << " </generic>" << endl;
  outfile << " <dimension nRow=\"1\" nCol=\"1\" nLayer=\"" << N_LYRS 
          << "\" nXtal=\"" << N_COLS 
          <<"\" nFace=\"" << N_FACES 
          << "\" nRange=\"" << N_RNGS 
          << "\"/>" << endl;
  outfile << " <tower iRow=\"0\" iCol=\"0\">"<< endl;

  for (int lyr=0; lyr < N_LYRS; lyr++) {
    outfile << "  <layer iLayer=\"" << lyr << "\">" << endl;
    for (int col=0; col < N_COLS; col++) {
      outfile << "   <xtal iXtal=\"" << col << "\">" << endl;
      for (int face =0; face < N_FACES; face++) {
        outfile << "    <face end=\"" << FACE_MNEM[face] << "\">" << endl;

        for (int rng=0; rng < N_RNGS; rng++) {
          int nRng = getNRng(lyr,col,face,rng);

          float av = m_calPed[nRng];
          float err= m_calPedErr[nRng];

          outfile <<"     <calPed avg=\"" << av
                  << "\" sig=\"" << err
                  << "\" range=\"" << RNG_MNEM[rng] << "\"/>"
                  << endl;
        }
        outfile << "    </face>" << endl;
      }
      outfile << "   </xtal>" << endl;
    }
    outfile<<"  </layer>" << endl;
  }
  outfile << " </tower>"<< endl;
  outfile << "</calCalib>" << endl;
}

void muonCalib::writeAsymXML(const string &filename, const string &dtdFilename) {
  ofstream outfile(filename.c_str());
  ifstream dtdFile(dtdFilename.c_str());
  if (!outfile.is_open())
    throw string("Unable to open " + filename);
  if (!dtdFile.is_open())
    throw string("Unable to open " + dtdFilename);
  outfile << "<?xml version=\"1.0\"?>" << endl;
  
  // INSERT ENTIRE .DTD FILE //
  outfile << "<!DOCTYPE calCalib [";
  string temp;
  while (dtdFile.good()) {
    getline(dtdFile, temp);
    if (dtdFile.fail()) continue; // bad get
    outfile << temp << endl;
  }
  outfile << "]>" << endl;

  outfile << "<calCalib>" << endl;
  outfile << " <generic instrument=\"" << m_instrument <<"\" timestamp=\""<< m_timestamp <<"\" calibType=\"CAL_Asym\" fmtVersion=\"v2r2\">" << endl;
  outfile << " </generic>" << endl;
  outfile << " <dimension nRow=\"1\" nCol=\"1\" nLayer=\"" << N_LYRS 
          << "\" nXtal=\"" << N_COLS 
          <<"\" nFace=\"" << 1
          << "\" nRange=\"" << 1
          << "\" nXpos=\"" << 1
          << "\"/>" << endl;
  
  // -- GENERATE xpos VALUES -- //
  outfile << " <xpos values=\"";
  for (int i = 0; i < N_ASYM_PTS; i++) outfile << i+1.5 << " ";
  outfile << "\"/>" << endl;
  
  // -- OUTPUT ASYMETRY DATA -- //
  outfile << " <tower iRow=\"0\" iCol=\"0\">"<< endl;

  for (int lyr=0; lyr < N_LYRS; lyr++) {
    outfile << "  <layer iLayer=\"" << lyr << "\">" << endl;
    for (int col=0; col < N_COLS; col++) {
      outfile << "   <xtal iXtal=\"" << col << "\">" << endl;
      outfile << "    <face end=\"NA\">" << endl;
       
      int nXtal = getNXtal(lyr,col);
      outfile << "     <asym " << endl;
      // ASYM LL
      outfile << "           bigVals=\"";
      for (int i = 0; i < N_ASYM_PTS; i++)
        outfile << " " << m_calAsymLL[nXtal][i];
      outfile << "\"" << endl;
      // ASYM LL Err
      outfile << "           bigSigs=\"";
      for (int i = 0; i < N_ASYM_PTS; i++)
        outfile << " " << m_calAsymLLErr[nXtal][i];
      outfile << "\"" << endl;

      // ASYM LS
      outfile << "           NsmallPbigVals=\"";
      for (int i = 0; i < N_ASYM_PTS; i++)
        outfile << " " << m_calAsymLS[nXtal][i];
      outfile << "\"" << endl;
      // ASYM LS Err
      outfile << "           NsmallPbigSigs=\"";
      for (int i = 0; i < N_ASYM_PTS; i++)
        outfile << " " << m_calAsymLSErr[nXtal][i];
      outfile << "\"" << endl;

      // ASYM SL
      outfile << "           PsmallNbigVals=\"";
      for (int i = 0; i < N_ASYM_PTS; i++)
        outfile << " " << m_calAsymSL[nXtal][i];
      outfile << "\"" << endl;
      // ASYM SL Err
      outfile << "           PsmallNbigSigs=\"";
      for (int i = 0; i < N_ASYM_PTS; i++)
        outfile << " " << m_calAsymSLErr[nXtal][i];
      outfile << "\"" << endl;

      // ASYM SS
      outfile << "           smallVals=\"";
      for (int i = 0; i < N_ASYM_PTS; i++)
        outfile << " " << m_calAsymSS[nXtal][i];
      outfile << "\"" << endl;
      // ASYM SS Err
      outfile << "           smallSigs=\"";
      for (int i = 0; i < N_ASYM_PTS; i++)
        outfile << " " << m_calAsymSSErr[nXtal][i];
      outfile << "\" />" << endl;

      outfile << "    </face>" << endl;
      outfile << "   </xtal>" << endl;
    }
    outfile<<"  </layer>" << endl;
  }
  outfile << " </tower>"<< endl;
  outfile << "</calCalib>" << endl;
}

void muonCalib::writeMPDXML(const string &filename, const string &dtdFilename) {
  ofstream outfile(filename.c_str());
  ifstream dtdFile(dtdFilename.c_str());
  if (!outfile.is_open())
    throw string("Unable to open " + filename);
  if (!dtdFile.is_open())
    throw string("Unable to open " + dtdFilename);

  outfile << "<?xml version=\"1.0\"?>" << endl;
  
  // INSERT ENTIRE .DTD FILE //
  outfile << "<!DOCTYPE calCalib [";
  string temp;
  while (dtdFile.good()) {
    getline(dtdFile, temp);
    if (dtdFile.fail()) continue; // bat get()
    outfile << temp << endl;;
  }
  outfile << "]>" << endl;

  outfile << "<calCalib>" << endl;
  outfile << " <generic instrument=\"" << m_instrument <<"\" timestamp=\""<< m_timestamp <<"\" calibType=\"CAL_MevPerDac\" fmtVersion=\"v2r2\">" << endl;
  outfile << " </generic>" << endl;
  outfile << " <dimension nRow=\"1\" nCol=\"1\" nLayer=\"" << N_LYRS 
          << "\" nXtal=\"" << N_COLS 
          <<"\" nFace=\"" << 1 
          << "\" nRange=\"" << 1 
          << "\" nXpos=\"" << 1 
          << "\"/>" << endl;
  outfile << " <tower iRow=\"0\" iCol=\"0\">"<< endl;

  for (int lyr=0; lyr < N_LYRS; lyr++) {
    outfile << "  <layer iLayer=\"" << lyr << "\">" << endl;
    for (int col=0; col < N_COLS; col++) {
      int nXtal = getNXtal(lyr,col);
      outfile << "   <xtal iXtal=\"" << col << "\">" << endl;
      outfile << "    <face end=\"" << "NA" << "\">" << endl;

      outfile << "     <mevPerDac bigVal=\"" << m_calMPDLarge[nXtal] 
              << "\" bigSig=\"" << m_calMPDLargeErr[nXtal]
              << "\" smallVal=\"" << m_calMPDSmall[nXtal]
              << "\" smallSig=\"" << m_calMPDSmallErr[nXtal]
              << "\">" << endl;
      outfile << "      <bigSmall end=\"POS\" bigSmallRatioVals=\"0\" bigSmallRatioSigs=\"0\"/>" << endl;
      outfile << "      <bigSmall end=\"NEG\" bigSmallRatioVals=\"0\" bigSmallRatioSigs=\"0\"/>" << endl;
      outfile << "     </mevPerDac>" << endl;

      outfile << "    </face>" << endl;
      outfile << "   </xtal>" << endl;
    }
    outfile<<"  </layer>" << endl;
  }
  outfile << " </tower>"<< endl;
  outfile << "</calCalib>" << endl;
}
