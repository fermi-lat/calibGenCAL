// LOCAL INCLUDES
#include "RootFileAnalysis.h"
#include "CfCfg.h"
#include "CalDefs.h"

// GLAST INCLUDES

// EXTLIB INCLUDES
#include "TF1.h"
#include "TGraph.h"
#include "TSpline.h"

// STD INCLUDES
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

using namespace std;
using namespace CalDefs;

// CONSTANTS //
const double mu2ci_thr_rat = 1.7;

//////////////////////////////////////////////////////
// class CfData - one complete set of intlin data ////
//////////////////////////////////////////////////////
class CfData {
  friend class RootCI;

public:

  CfData(CfCfg &cfg);
  ~CfData() {delete m_dacArr;}

  void FitData();
  void corr_FLE_Thresh(CfData& data_high_thresh);
  void WriteSplinesXML(const string &filename, const string &dtdPath);
  void WriteSplinesTXT(const string &filename);
  void ReadSplinesTXT (const string &filename);

  const vector<int>& getSplineDAC(RngNum rng) const {return m_splineDAC[rng];}

  const vector<float>& getSplineADC(tRngIdx rngIdx) const {
    return m_splineADC[rngIdx];
  }

  int getNSplineADC(tRngIdx rngIdx) const {
    return m_splineADC[rngIdx].size();
  }

private:
  TF1 splineFunc;

  CalVec<tRngIdx, vector<float> >  m_adcSum;
  CalVec<tRngIdx, vector<float> >  m_adcN;
  CalVec<tRngIdx, vector<float> >  m_adcMean;

  CalVec<tRngIdx, vector<float> >  m_splineADC;
  CalVec<RngNum, vector<int> >    m_splineDAC;

  CfCfg &m_cfg;
  // int c-style array copy of CfCfg::dacVals needed by some fit routines
  float *m_dacArr;
};

CfData::CfData(CfCfg &cfg) :
  splineFunc("spline_fitter","pol2",0,4095),
  m_adcSum(tRngIdx::N_VALS, vector<float>(cfg.nDACs,0)),
  m_adcN(tRngIdx::N_VALS, vector<float>(cfg.nDACs,0)),
  m_adcMean(tRngIdx::N_VALS, vector<float>(cfg.nDACs,0)),
  m_splineADC(tRngIdx::N_VALS),
  m_splineDAC(RngNum::N_VALS),
  m_cfg(cfg),
  m_dacArr(0)
{

  // need a c-style array of floats for DAC values.
  m_dacArr = new float[m_cfg.nDACs];
  copy(m_cfg.dacVals.begin(), m_cfg.dacVals.begin()+m_cfg.nDACs, m_dacArr);
}

// smooth test lines & print output.
void CfData::FitData() {
  // 2 dimensional poly line f() to use for spline fitting.
  float *tmpADC(new float[m_cfg.nDACs]);

  for (RngNum rng; rng.isValid(); rng++) {
    // following vals only change w/ rng, so i get them outside the other loops.
    int grpWid  = m_cfg.splineGroupWidth[rng];
    //int splLen  = grpWid*2 + 1;
    int skpLo   = m_cfg.splineSkipLow[rng];
    int skpHi   = m_cfg.splineSkipHigh[rng];
    int nPtsMin = m_cfg.splineNPtsMin[rng];

    for (tFaceIdx faceIdx; faceIdx.isValid(); faceIdx++) {
      tRngIdx rngIdx(faceIdx,rng);

      vector<float> &curADC = m_adcMean[rngIdx];
      // get pedestal
      float ped     = curADC[0];

      //calculate ped-subtracted means.
      for (int dac = 0; dac < m_cfg.nDACs; dac++)
        curADC[dac] -= ped;

      // get upper adc boundary
      float adc_max = curADC[m_cfg.nDACs-1];

      // last idx will be last index that is <= 0.99*adc_max
      // it is the last point we intend on using.
      int last_idx = 0; 
      while (curADC[last_idx] <= 0.99*adc_max) {
        last_idx++;
      }
      last_idx--;
      
      adc_max = curADC[last_idx]; 

      // set up new graph object for fitting.
      for (unsigned i = 0; i < curADC.size(); i++) tmpADC[i] = curADC[i];
      TGraph myGraph(last_idx+1,
                     m_dacArr,
                     tmpADC);

      // copy SKPLO points directly from beginning of array.
      int spl_idx = 0;
      for (int i = 0; i < skpLo; i++,spl_idx++) {
        if (curADC[i] > adc_max) break; // quit if past the max_value (unlikely)
        m_splineADC[rngIdx].push_back(curADC[i]);
        if (m_splineADC[rngIdx].size() >  m_splineDAC[rng].size())
          m_splineDAC[rng].push_back((int)m_dacArr[i]);
      }

      //
      // RUN SPLINE FITS
      //
      // start one grp above skiplo & go as high as you can w/out entering skpHi
      for (int cp = skpLo + grpWid - 1; // cp = 'center point'
           cp < (nPtsMin-skpLo-skpHi)*grpWid + skpLo;
           cp += grpWid, spl_idx++) {
        int lp = cp - grpWid;
        int hp  = cp + grpWid;

        myGraph.Fit(&splineFunc,"QN","",m_dacArr[lp],m_dacArr[hp]);
        float myPar1 = splineFunc.GetParameter(0);
        float myPar2 = splineFunc.GetParameter(1);
        float myPar3 = splineFunc.GetParameter(2);

        int   fitDAC = (int)m_dacArr[cp];
        float fitADC = myPar1 + fitDAC*(myPar2 + fitDAC*myPar3);

        if (fitADC > adc_max) break; // quit if we have past the max_adc_value

        // output result
        m_splineADC[rngIdx].push_back(fitADC);
        if (m_splineADC[rngIdx].size() > m_splineDAC[rng].size())
          m_splineDAC[rng].push_back(fitDAC);
      }

      // copy SKPHI points directly from face of array.
      for (int i = (nPtsMin-skpLo-skpHi)*grpWid + skpLo;
           i <= last_idx;
           i++,spl_idx++) {
        if (curADC[i] > adc_max) break; // quit if past the max_adc_value

        m_splineADC[rngIdx].push_back(curADC[i]); 
        if (m_splineADC[rngIdx].size() >  m_splineDAC[rng].size())
          m_splineDAC[rng].push_back((int)m_dacArr[i]);
      }
    }
  }

  delete tmpADC;
}

void CfData::corr_FLE_Thresh(CfData& data_high_thresh){
  RngNum rng;  // we correct LEX8 rng only

  for (tFaceIdx faceIdx; faceIdx.isValid(); faceIdx++) {
    tRngIdx rngIdx(faceIdx,rng);

    int nSpline = m_splineADC[rngIdx].size();
    double* arrADC = new double(nSpline);
    double* arrDAC = new double(nSpline);
    for (int i = 0; i<nSpline; i++){
      arrADC[i] = m_splineADC[rngIdx][i];
      arrDAC[i] = m_splineDAC[rng][i];
    }
    int nSpline_hi_thr = data_high_thresh.getNSplineADC(rngIdx);
    double* arrADC_hi_thr = new double(nSpline_hi_thr);
    double* arrDAC_hi_thr = new double(nSpline_hi_thr);
    const vector<float>& splineADC = data_high_thresh.getSplineADC(rngIdx);
    const vector<int>& splineDAC = data_high_thresh.getSplineDAC(rng);


    for (int i = 0; i<nSpline_hi_thr; i++){
      arrADC_hi_thr[i] = splineADC[i];
      arrDAC_hi_thr[i] = splineDAC[i];
    }

    TSpline3* spl = new TSpline3("spl",arrDAC,arrADC,nSpline); 
    TSpline3* spl_hi_thr = new TSpline3("spl_hi_thr",arrDAC_hi_thr,
                                        arrADC_hi_thr,nSpline_hi_thr); 

    for (int i = 0; i<nSpline;i++){
      float dac_corr = arrDAC[i]/mu2ci_thr_rat;
      m_splineADC[rngIdx][i] = arrADC_hi_thr[i] + 
        spl->Eval(dac_corr)-spl_hi_thr->Eval(dac_corr);
    }

    delete spl;
    delete spl_hi_thr;
    delete arrADC;
    delete arrDAC;
    delete arrADC_hi_thr;
    delete arrDAC_hi_thr;

  }
}

void CfData::WriteSplinesTXT(const string &filename) {
  ofstream outFile(filename.c_str());
  if (!outFile.is_open()) {
    ostringstream tmp;
    tmp << __FILE__ << ":" << __LINE__ << " " 
        << "ERROR! unable to open txtFile='" << filename << "'";
    throw tmp.str();
  }
  outFile.precision(2);
  outFile.setf(ios_base::fixed);

  for (tRngIdx rngIdx; rngIdx.isValid(); rngIdx++)
    for (unsigned n = 0; n < m_splineADC[rngIdx].size(); n++) {
      RngNum rng = rngIdx.getRng();
      outFile << rngIdx.getLyr() << " "
              << rngIdx.getCol()  << " "
              << rngIdx.getFace() << " "
              << rng  << " "
              << m_splineDAC[rng][n] << " "
              << m_splineADC[rngIdx][n]
              << endl;
    }
}

void CfData::ReadSplinesTXT (const string &filename) {
  ifstream inFile(filename.c_str());
  if (!inFile.is_open()) {
    ostringstream tmp;
    tmp << __FILE__ << ":" << __LINE__ << " " 
        << "ERROR! unable to open txtFile='" << filename << "'";
    throw tmp.str();
  }

  short twr = m_cfg.twrBay;
  short lyr;
  short col;
  short face;
  short rng;
  int tmpDAC;
  float tmpADC;
  while (inFile.good()) {
    // load in one spline val w/ coords
    inFile >> lyr 
           >> col
           >> face
           >> rng
           >> tmpDAC
           >> tmpADC;
    
    tRngIdx rngIdx(lyr,col,face,rng);

    m_splineADC[rngIdx].push_back(tmpADC);
    if (m_splineADC[rngIdx].size() > m_splineDAC[rng].size()) 
      m_splineDAC[rng].push_back(tmpDAC);
  }
}

void CfData::WriteSplinesXML(const string &filename, const string &dtdPath) {
  // setup output file
  ofstream xmlFile(filename.c_str());
  if (!xmlFile.is_open()) {
    ostringstream tmp;
    tmp << __FILE__ << ":" << __LINE__ << " " 
        << "ERROR! unable to open xmlFile='" << filename << "'";
    throw tmp.str();
  }

  // input file
  ifstream dtdFile(dtdPath.c_str());
  if (!dtdFile.is_open()) {
    ostringstream tmp;
    tmp << __FILE__ << ":" << __LINE__ << " " 
        << "ERROR! unable to open dtdPath='" << dtdPath << "'";
    throw tmp.str();
  }

  //
  // XML file header
  //
  xmlFile << "<?xml version=\"1.0\" ?>" << endl;
  xmlFile << "<!-- $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/runCIFit.cxx,v 1.12 2005/04/18 15:53:41 heather Exp $  -->" << endl;
  xmlFile << "<!-- Made-up  intNonlin XML file for EM, according to calCalib_v2r1.dtd -->" << endl;
  xmlFile << endl;
  xmlFile << "<!DOCTYPE calCalib [" << endl;
  string tmpStr;
  while (dtdFile.good()) {
    getline(dtdFile, tmpStr);
    if (dtdFile.fail()) continue; // bat get()
    xmlFile << tmpStr << endl;;
  }
  xmlFile << "] >" << endl;
  xmlFile << endl;
  xmlFile << "<calCalib>" << endl;
  xmlFile << "  <generic instrument=\"" << m_cfg.instrument
          << "\" timestamp=\"" << m_cfg.timestamp << "\"" << endl;
  xmlFile << "           calibType=\"CAL_IntNonlin\" fmtVersion=\"v2r2\" >" << endl;

  xmlFile << endl;
  xmlFile << "  </generic>" << endl;
  xmlFile << endl;
  xmlFile << "<!-- EM instrument: 8 layers, 12 columns -->" << endl;
  xmlFile << endl;
  xmlFile << "<!-- number of collections of dac settings should normally be" << endl;
  xmlFile << "     0 (the default), if dacs aren't used to acquire data, or " << endl;
  xmlFile << "     equal to nRange -->" << endl;
  xmlFile << " <dimension nRow=\"" << 1 
          << "\" nCol=\"" << 1 
          << "\" nLayer=\"" << LyrNum::N_VALS 
          << "\" nXtal=\""  << ColNum::N_VALS 
          << "\" nFace=\""  << FaceNum::N_VALS 
          << "\" nRange=\"" << RngNum::N_VALS << "\"" << endl;
  xmlFile << "           nDacCol=\"" << RngNum::N_VALS << "\" />" << endl;

  //
  // DAC values for rest of file.
  //
  xmlFile << endl;
  for (RngNum rng; rng.isValid(); rng++) {
    xmlFile << " <dac range=\"" << RngNum::MNEM[rng] << "\"" << endl;
    xmlFile << "     values=\"";
    for (unsigned i = 0; i < m_splineDAC[rng].size(); i++) 
      xmlFile << m_splineDAC[rng][i] << " ";
    xmlFile << "\"" << endl;
    xmlFile << "     error=\"" << 0.1 << "\" />" << endl;
  }
  
  //
  // main data loop
  //
  
  xmlFile.setf(ios_base::fixed);
  xmlFile.precision(2);
  // TOWER // currently only using 1 tower.
  xmlFile << endl;
  // only using 1 tower right now
  for (TwrNum twr = m_cfg.twrBay; twr == m_cfg.twrBay; twr++) {
    xmlFile << " <tower iRow=\"" << twr.getRow() 
            << "\" iCol=\"" << twr.getCol() << "\">" << endl;
    // LAYER //
    for (LyrNum lyr; lyr.isValid(); lyr++) {
      xmlFile << "  <layer iLayer=\"" << lyr << "\">" << endl;
      // COL //
      for (ColNum col; col.isValid(); col++) {
        xmlFile << "   <xtal iXtal=\"" << col << "\">" << endl;
        // FACE //
        for (FaceNum face; face.isValid(); face++) {
          const string facestr = (face == NEG_FACE) ? "NEG" : "POS";
          xmlFile << "    <face end=\"" << facestr << "\">" << endl;
          // RNG //
          for (RngNum rng; rng.isValid(); rng++) {
            tRngIdx rngIdx(lyr,col,face,rng);

            xmlFile << "     <intNonlin range=\"" << RngNum::MNEM[rng] << "\"" << endl;
            // ADC VALS //
            xmlFile << "             values=\"";
            for (unsigned i = 0; i < m_splineADC[rngIdx].size(); i++)
              xmlFile << fixed << m_splineADC[rngIdx][i] << " ";

            xmlFile << "\"" << endl;
            xmlFile << "             error=\"" << 0.1 << "\" />" << endl;
          }
          xmlFile << "    </face>" << endl;
        }
        xmlFile << "   </xtal>" << endl;
      }
      xmlFile << "  </layer>" << endl;
    }
    xmlFile << " </tower>" << endl;
  }
  xmlFile << "</calCalib>" << endl;
}

//////////////////////////////////////////////////////
// class RootCI -                                   //
// needed for populating the CfData class           //
//////////////////////////////////////////////////////

class RootCI : public RootFileAnalysis {
public:
  // @enum Diode Specify LE, HE, BOTH_DIODES
  typedef enum Diode {
    LE,
    HE,
    BOTH_DIODES};

  // Standard ctor, where user provides the names of the input root files
  // and optionally the name of the output ROOT histogram file
  RootCI(vector<string> &digiFileNames,
         CfData  &data, CfCfg &cfg);

  // standard dtor
  ~RootCI();

  // processes one 'event', as a collection of interactions.
  // loads up histograms.
  void DigiCal();

  // loops through all events in file
  void Go(Int_t nEvtAsked);

  void SetDiode(RootCI::Diode d) {m_curDiode = d;}

  void initCiHists();

private:
  bool   isRngEnabled(RngNum rng);  // checks rng against m_curDiode setting
  Diode m_curDiode;

  CfData &m_CfData;
  CfCfg  &m_cfg;

  int m_evtId;
  int m_nMax;
  int m_iGoodEvt;

  CalVec<tRngIdx,  TH1F*> m_ciHists; 

};


RootCI::RootCI(vector<string> &digiFileNames, CfData  &data, CfCfg &cfg) :
  RootFileAnalysis(vector<string>(0), digiFileNames, vector<string>(0)),
  m_curDiode(BOTH_DIODES),
  m_CfData(data),
  m_cfg(cfg) {
  m_iGoodEvt = 0; 
}

// default dstor
RootCI::~RootCI() {
}


void RootCI::initCiHists() {
  // DEJA VU?
  if (m_ciHists.size() == 0) {
    m_ciHists.resize(tRngIdx::N_VALS);
        
    for (tRngIdx rngIdx; rngIdx.isValid(); rngIdx++) {
      ostringstream tmp;
      if( m_curDiode == RootCI::LE) 
        tmp << "ciLE_" << rngIdx;
      else tmp << "ciHE_" << rngIdx;       
      m_ciHists[rngIdx] = new TH1F (tmp.str().c_str(),tmp.str().c_str(),
                                    4100,0,4100);
    }
  }
  else // clear existing histsograms
    for (tRngIdx rngIdx; rngIdx.isValid(); rngIdx++)
      m_ciHists[rngIdx]->Reset();
}


// checks rng against m_curDiode setting
bool  RootCI::isRngEnabled(RngNum rng) {
  if (m_curDiode == BOTH_DIODES) return true;
  if (m_curDiode == LE && (rng == LEX8 || rng == LEX1)) return true;
  if (m_curDiode == HE && (rng == HEX8 || rng == HEX1)) return true;
  return false;
}

// compiles stats for each test type.
void RootCI::DigiCal() {
  // Determine test config for this event (which xtal?, which dac?)
  int ievt = m_iGoodEvt;
  int testCol   = ievt/m_cfg.nPulsesPerXtal;
  int testDAC   = (ievt%m_cfg.nPulsesPerXtal)/m_cfg.nPulsesPerDAC;
  int iSamp   = (ievt%m_cfg.nPulsesPerXtal)%m_cfg.nPulsesPerDAC;

  const TObjArray* calDigiCol = m_digiEvt->getCalDigiCol();
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
  if(nDigis == tXtalIdx::N_VALS){
    m_iGoodEvt++;

    //loop through each 'hit' in one event
    while ((pdig = (CalDigi*)calDigiIter.Next())) {  
      CalDigi &cdig = *pdig; // use ref to reduce '->'

      CalXtalId id = cdig.getPackedId();  // get interaction information

      // skip if not for current column
      ColNum col = id.getColumn();
      if (col != testCol) continue;

      // skip if not for current tower
      TwrNum twr = id.getTower();
      if (twr != m_cfg.twrBay) continue;

      LyrNum lyr = id.getLayer();

      // Loop through each readout on current xtal
      int numRo = cdig.getNumReadouts();
      for (int iRo=0; iRo<numRo; iRo++){
        const CalXtalReadout &acRo = *(cdig.getXtalReadout(iRo));
        for (FaceNum face; face.isValid(); face++) {
          RngNum rng = acRo.getRange((CalXtalId::XtalFace)(short)face);
          // only interested in current diode!
          if (!isRngEnabled(rng)) continue;

          int adc = acRo.getAdc((CalXtalId::XtalFace)(short)face);
          tRngIdx rngIdx(lyr,col,face,rng);
          TH1F& h = *m_ciHists[rngIdx];
          if(iSamp == 0){
            h.Reset();
            h.SetAxisRange(0,4100);
          }

          h.Fill(adc);
                

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
            m_CfData.m_adcMean[rngIdx][testDAC] = av;
            m_CfData.m_adcN[rngIdx][testDAC] = h.Integral();     

          }
        } // foreach face
      } // foreach readout
    } // foreach xtal
  }
  else {
    m_cfg.ostrm << " event " << m_evtId << " contains " 
                << nDigis << " digis - skipped" << endl;
    m_nMax++;
  }
}

void RootCI::Go(Int_t nEvtAsked)
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
  Int_t curI;
  m_nMax = min(nEvtAsked+m_startEvt,nEvts);

  if (nEvtAsked+m_startEvt >  nEvts) {
    ostringstream tmp;
    tmp << __FILE__ << ":" << __LINE__ << " " 
        << "not enough entries in file to proceed, we need " << nEvtAsked;
    throw tmp.str();
  }

  // BEGINNING OF EVENT LOOP
  for (Int_t iEvt=m_startEvt; iEvt<m_nMax; iEvt++, curI=iEvt) {
    if (m_digiEvt) m_digiEvt->Clear();

    getEvent(iEvt);
    // Digi ONLY analysis
    if (m_digiEvt) {
      m_evtId = m_digiEvt->getEventId();
      if(m_evtId%1000 == 0)
        m_cfg.ostrm << " event " << m_evtId << endl;

      DigiCal();
    }
  }  // end analysis code in event loop

  m_startEvt = curI;
}

int main(int argc, char **argv) {
  // Load xml config file
  string cfgPath;
  if(argc > 1) cfgPath = argv[1];
  else cfgPath = "../src/ciFit_option.xml";

  CfCfg cfg;
  try {
    cfg.readCfgFile(cfgPath);

    // insert quoted config file into log stream //
    { 
      string tmp;
      ifstream cfgFile(cfgPath.c_str());
      cfg.ostrm << "--- Begin cfg_file: " << cfgPath << " ---" << endl;
      while (cfgFile.good()) {
        getline(cfgFile, tmp);
        if (cfgFile.fail()) continue; // bad get
        cfg.ostrm << "> " << tmp << endl;
      }
      cfg.ostrm << "--- End " << cfgPath << " ---" << endl << endl;
    }

    cfg.ostrm << endl;
    output_env_banner(cfg.ostrm);
    cfg.ostrm << endl;
    
    
    CfData data(cfg);

    // LE PASS
    {
      vector<string> digiFileNames;
      digiFileNames.push_back(cfg.rootFileLE1);
      RootCI rd(digiFileNames,data,cfg);  
      // set HE/LE rng
      rd.SetDiode(RootCI::LE);
      rd.initCiHists();
      rd.Go(cfg.nPulsesPerRun);
    }

    // HE PASS
    {
      vector<string> digiFileNames;
      digiFileNames.push_back(cfg.rootFileHE1);
      RootCI rd(digiFileNames, data,cfg);
      rd.SetDiode(RootCI::HE);
      rd.initCiHists();
      rd.Go(cfg.nPulsesPerRun);
    }

    data.FitData();
    // data_high_thresh.FitData();
    // data.corr_FLE_Thresh(data_high_thresh);
    //data.ReadSplinesTXT("../output/ciSplines.txt");
    if (cfg.genTXT) data.WriteSplinesTXT(cfg.outputTXTPath);
    if (cfg.genXML) data.WriteSplinesXML(cfg.outputXMLPath, cfg.dtdPath);
  } catch (string s) {
    // generic exception handler...  all my exceptions are simple C++ strings
    cfg.ostrm << "ciFit:  exception thrown: " << s << endl;
    return -1;
  }

  return 0;
}

