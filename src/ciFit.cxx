#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

//ROOT INCLUDES
#include "TF1.h"
#include "TGraph.h"
#include "TSpline.h"

#include "RootFileAnalysis.h"

#include "cfCfg.h"
#include "CalDefs.h"

using namespace std;

// CONSTANTS //
const double mu2ci_thr_rat = 1.7;

//////////////////////////////////////////////////////
// class cfData - represents one complete set of intNonlin data ////////
//////////////////////////////////////////////////////
class cfData : protected CalDefs {
  friend class RootCI;

public:

  cfData(cfCfg &cfg);
  ~cfData() {delete m_dacArr;}

  void FitData();
  void corr_FLE_Thresh(cfData& data_high_thresh);
  void WriteSplinesXML(const string &filename, const string &dtdFilename);
  void WriteSplinesTXT(const string &filename);
  void ReadSplinesTXT (const string &filename);

  const int getNumSplineADC(const int lyr, const int col, const int face, const int rng) const {return m_numSplineADC[lyr][col][face][rng];}
  const vector<int>& getSplineDAC(int rng) const {return m_splineDAC[rng];}
  const vector<float>& getSplineADC(const int lyr, const int col, const int face, const int rng) const {return m_splineADC[lyr][col][face][rng];}


private:
  TF1 splineFunc;

  vector<float>  m_adcSum[N_LYRS][N_COLS][N_FACES][N_RNGS];
  vector<int>    m_adcN[N_LYRS][N_COLS][N_FACES][N_RNGS];
  vector<float>  m_adcMean[N_LYRS][N_COLS][N_FACES][N_RNGS];

  vector<float>  m_splineADC[N_LYRS][N_COLS][N_FACES][N_RNGS];
  int            m_numSplineADC[N_LYRS][N_COLS][N_FACES][N_RNGS];
  vector<int>    m_splineDAC[N_RNGS];
  vector<int>    m_numSplineDAC;

  cfCfg &m_cfg;
  // int c-style array copy of cfCfg::m_dacSettings needed by some fit routines
  float *m_dacArr;
};

cfData::cfData(cfCfg &cfg) :
  splineFunc("spline_fitter","pol2",0,4095),
  m_cfg(cfg)
{

  // need a c-style array of floats for DAC values.
  m_dacArr = new float[m_cfg.nDACs];
  for (int i = 0; i < m_cfg.nDACs; i++)
    m_dacArr[i] = m_cfg.dacSettings[i]; 

  // init vector arrays. c++ doesn't auto construct multidim vectors
  for (int lyr = 0; lyr < N_LYRS; lyr++)
    for (int col = 0; col < N_COLS; col++)
      for (int face = 0; face < N_FACES; face++)
        for (int rng = 0; rng < N_RNGS; rng++) {
          m_adcSum[lyr][col][face][rng].resize(m_cfg.nDACs);
          m_adcN[lyr][col][face][rng].resize(m_cfg.nDACs);
          m_adcMean[lyr][col][face][rng].resize(m_cfg.nDACs);
          m_splineADC[lyr][col][face][rng].resize(m_cfg.nDACs);
        }
  for (int rng = 0; rng < N_RNGS; rng++)
    m_splineDAC[rng].resize(m_cfg.nDACs);
  
  m_numSplineDAC.resize(N_RNGS); 
}

// smooth test lines & print output.
void cfData::FitData() {
  // 2 dimensional poly line f() to use for spline fitting.
  float *tmpADC(new float[m_cfg.nDACs]);

  for (int rng = 0; rng < N_RNGS; rng++) {
    // following vals only change w/ rng, so i'm getting them outside the other loops.
    int grpWid  = m_cfg.splineGroupWidth[rng];
    //int splLen  = grpWid*2 + 1;
    int skpLo   = m_cfg.splineSkipLow[rng];
    int skpHi   = m_cfg.splineSkipHigh[rng];
    int nPtsMin = m_cfg.splineNPtsMin[rng];

    for (int lyr = 0; lyr < N_LYRS; lyr++)
      for (int col = 0; col < N_COLS; col++)
        for (int face = 0; face < N_FACES; face++) {
          vector<float> &curADC = m_adcMean[lyr][col][face][rng];
          // get pedestal
          float ped     = m_adcSum[lyr][col][face][rng][0] /
            m_adcN[lyr][col][face][rng][0];

          //calculate ped-subtracted means.
          for (int dac = 0; dac < m_cfg.nDACs; dac++)
            curADC[dac] =
              m_adcSum[lyr][col][face][rng][dac] /
              m_adcN[lyr][col][face][rng][dac] - ped;

          // get upper adc boundary
          float adc_max = curADC[m_cfg.nDACs-1];
          int last_idx = 0; // last idx will be 1st index that is > 0.99*adc_max, it is the last point we intend on using.
          while (curADC[last_idx] < 0.99*adc_max) {
             last_idx++;
          }
          
          adc_max = curADC[last_idx]; 

          // set up new graph object for fitting.
          for (unsigned i = 0; i < curADC.size(); i++) tmpADC[i] = curADC[i];
          TGraph myGraph(last_idx+1,
                         m_dacArr,
                         tmpADC);

          // copy SKPLO points directly from beginning of array.
          int spl_idx = 0;
          for (int i = 0; i < skpLo; i++,spl_idx++) {
            if (curADC[i] > adc_max) break; // quit if we have past the max_value (unlikely in this loop, but it's here
            m_splineDAC[rng][spl_idx] = (int)m_dacArr[i];
            m_splineADC[lyr][col][face][rng][spl_idx] = curADC[i];
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
            m_splineDAC[rng][spl_idx] = fitDAC;
            m_splineADC[lyr][col][face][rng][spl_idx] = fitADC;
          }

          // copy SKPHI points directly from face of array.
          for (int i = (nPtsMin-skpLo-skpHi)*grpWid + skpLo;
               i <= last_idx;
               i++,spl_idx++) {
            if (curADC[i] > adc_max) break; // quit if we have past the max_adc_value

            m_splineDAC[rng][spl_idx] = (int)m_dacArr[i];
            m_splineADC[lyr][col][face][rng][spl_idx] = curADC[i]; 
          }

          //
          // UPDATE COUNTS
          //
          m_numSplineADC[lyr][col][face][rng] = spl_idx;
          m_numSplineDAC[rng] = max(m_numSplineDAC[rng],spl_idx);  // ensure we have just enough DAC points for largest spline
        }
  }

  delete tmpADC;
}

void cfData::corr_FLE_Thresh(cfData& data_high_thresh){
  int rng = 0;  // we correct LEX8 rng only

  for (int lyr = 0; lyr < N_LYRS; lyr++)
    for (int col = 0; col < N_COLS; col++)
      for (int face = 0; face < N_FACES; face++) {

        int numSpline = m_numSplineADC[lyr][col][face][rng];
        double* arrADC = new double(numSpline);
        double* arrDAC = new double(numSpline);
        for (int i = 0; i<numSpline; i++){
          arrADC[i] = m_splineADC[lyr][col][face][rng][i];
          arrDAC[i] = m_splineDAC[rng][i];
        }
        int numSpline_hi_thr = data_high_thresh.getNumSplineADC(lyr,col,face,rng);
        double* arrADC_hi_thr = new double(numSpline_hi_thr);
        double* arrDAC_hi_thr = new double(numSpline_hi_thr);
        const vector<float>& splineADC = data_high_thresh.getSplineADC(lyr,col,face,rng);
        const vector<int>& splineDAC = data_high_thresh.getSplineDAC(rng);


        for (int i = 0; i<numSpline_hi_thr; i++){
          arrADC_hi_thr[i] = splineADC[i];
          arrDAC_hi_thr[i] = splineDAC[i];
        }

        TSpline3* spl = new TSpline3("spl",arrDAC,arrADC,numSpline); 
        TSpline3* spl_hi_thr = new TSpline3("spl_hi_thr",arrDAC_hi_thr,arrADC_hi_thr,numSpline_hi_thr); 

        for (int i = 0; i<numSpline;i++){
          float dac_corr = arrDAC[i]/mu2ci_thr_rat;
          m_splineADC[lyr][col][face][rng][i] = arrADC_hi_thr[i] + spl->Eval(dac_corr)-spl_hi_thr->Eval(dac_corr);
        }

        delete spl;
        delete spl_hi_thr;
        delete arrADC;
        delete arrDAC;
        delete arrADC_hi_thr;
        delete arrDAC_hi_thr;

      }
}

void cfData::WriteSplinesTXT(const string &filename) {
  ofstream outFile(filename.c_str());
  if (!outFile.is_open()) {
    ostringstream temp;
    temp << "ERROR! unable to open txtFile='" << filename << "'";
    throw temp.str();
  }
  outFile.precision(2);
  outFile.setf(ios_base::fixed);

  for (int lyr = 0; lyr < N_LYRS; lyr++) 
    for (int col = 0; col < N_COLS; col++)
      for (int face = 0; face < N_FACES; face++)
        for (int rng = 0; rng < N_RNGS; rng++)
          for (int n = 0; n < m_numSplineADC[lyr][col][face][rng]; n++)
            outFile << lyr << " "
                    << col  << " "
                    << face << " "
                    << rng  << " "
                    << m_splineDAC[rng][n] << " "
                    << m_splineADC[lyr][col][face][rng][n]
                    << endl;
}

void cfData::ReadSplinesTXT (const string &filename) {
  ifstream inFile(filename.c_str());
  if (!inFile.is_open()) {
    ostringstream temp;
    temp << "ERROR! unable to open txtFile='" << filename << "'";
    throw temp.str();
  }

  int lyr, col, face, rng;
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
 
    int cur_idx = m_numSplineADC[lyr][col][face][rng];
    m_splineADC[lyr][col][face][rng][cur_idx] = tmpADC;
    m_splineDAC[rng][cur_idx]                  = tmpDAC;
 
    // update counters
    m_numSplineADC[lyr][col][face][rng]++;
    m_numSplineDAC[rng] = max(m_numSplineDAC[rng],cur_idx+1);
  }
}

void cfData::WriteSplinesXML(const string &filename, const string &dtdFilename) {
  // setup output file
  ofstream xmlFile(filename.c_str());
  if (!xmlFile.is_open()) {
    ostringstream temp;
    temp << "ERROR! unable to open xmlFile='" << filename << "'";
    throw temp.str();
  }
  ifstream dtdFile(dtdFilename.c_str());
  if (!dtdFile.is_open()) {
    ostringstream temp;
    temp << "ERROR! unable to open dtdFile='" << dtdFilename << "'";
    throw temp.str();
  }

  //
  // XML file header
  //
  xmlFile << "<?xml version=\"1.0\" ?>" << endl;
  xmlFile << "<!-- $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/ciFit.cxx,v 1.12 2004/12/31 00:09:39 fewtrell Exp $  -->" << endl;
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
          << "\" timestamp=\"" << m_cfg.startTime << "\"" << endl;
  xmlFile << "           calibType=\"CAL_IntNonlin\" fmtVersion=\"v2r1\">" << endl;
  xmlFile << endl;
  xmlFile << "    <inputSample startTime=\"" << m_cfg.startTime
          << "\" stopTime=\"" << m_cfg.stopTime << "\"" << endl;
  xmlFile << "triggers=\"" << m_cfg.triggerMode
          << "\" mode=\"" << m_cfg.instrumentMode
          << "\" source=\"" << m_cfg.source << "\" >" << endl;
  xmlFile << endl;
  xmlFile << "Times are start and stop time of calibration run." << endl;
  xmlFile << "Other attributes are just made up for code testing." << endl;
  xmlFile << "    </inputSample>" << endl;
  xmlFile << "  </generic>" << endl;
  xmlFile << endl;
  xmlFile << "<!-- EM instrument: 8 layers, 12 columns -->" << endl;
  xmlFile << endl;
  xmlFile << "<!-- number of collections of dac settings should normally be" << endl;
  xmlFile << "     0 (the default), if dacs aren't used to acquire data, or " << endl;
  xmlFile << "     equal to nRange -->" << endl;
  xmlFile << " <dimension nRow=\"" << 1 
          << "\" nCol=\"" << 1 
          << "\" nLayer=\"" << N_LYRS 
          << "\" nXtal=\"" << N_COLS 
          << "\" nFace=\"" << N_FACES 
          << "\" nRange=\"" << N_RNGS << "\"" << endl;
  xmlFile << "           nDACCol=\"" << N_RNGS << "\" />" << endl;

  //
  // DAC values for rest of file.
  //
  xmlFile << endl;
  for (int rng = 0; rng < N_RNGS; rng++) {
    xmlFile << " <dac range=\"" << RNG_MNEM[rng] << "\"" << endl;
    xmlFile << "     values=\"";
    for (int i = 0; i < m_numSplineDAC[rng]; i++) 
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
  xmlFile << " <tower iRow=\"" << 0 << "\" iCol=\"" << 0 << "\">" << endl;
  // LAYER //
  for (int lyr = 0; lyr < N_LYRS; lyr++) {
    xmlFile << "  <layer iLayer=\"" << lyr << "\">" << endl;
    // COL //
    for (int col = 0; col < N_COLS; col++) {
      xmlFile << "   <xtal iXtal=\"" << col << "\">" << endl;
      // FACE //
      for (int face = 0; face < N_FACES; face++) {
        const string facestr = (face == CalXtalId::NEG) ? "NEG" : "POS";
        xmlFile << "    <face end=\"" << facestr << "\">" << endl;
        // RNG //
        for (int rng = 0; rng < N_RNGS; rng++) {
          xmlFile << "     <intNonlin range=\"" << RNG_MNEM[rng] << "\"" << endl;
          // ADC VALS //
          xmlFile << "             values=\"";
          for (int i = 0; i < m_numSplineADC[lyr][col][face][rng]; i++) {
            xmlFile << m_splineADC[lyr][col][face][rng][i] << " ";
          }
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
  xmlFile << "</calCalib>" << endl;
}

//////////////////////////////////////////////////////
// class RootCI - derived from RootFileAnalysis - represents all Root input //
// needed for populating the cfData class                                //
//////////////////////////////////////////////////////

class RootCI : public RootFileAnalysis, protected CalDefs {
public:
  // @enum Diode Specify LE, HE, BOTH_DIODES
  typedef enum Diode {
    LE,
    HE,
    BOTH_DIODES};

  // Standard ctor, where user provides the names of the input root files
  // and optionally the name of the output ROOT histogram file
  RootCI(vector<string> &digiFileNames,
         cfData  &data, cfCfg &cfg);

  // standard dtor
  ~RootCI();

  // processes one 'event', as a collection of interactions.
  // loads up histograms.
  void DigiCal();

  // loops through all events in file
  void Go(Int_t numEvents);

  void SetDiode(RootCI::Diode d) {m_curDiode = d;}

private:
  bool   isRngEnabled(CalXtalId::AdcRange rng);          // checks rng against m_curDiode setting
  Diode m_curDiode;

  cfData &m_cfData;
  cfCfg  &m_cfg;

  int m_evtId;
};


RootCI::RootCI(vector<string> &digiFileNames, cfData  &data, cfCfg &cfg) :
  RootFileAnalysis(vector<string>(0), digiFileNames, vector<string>(0)),
  m_curDiode(BOTH_DIODES),
  m_cfData(data),
  m_cfg(cfg) {}

// default dstor
RootCI::~RootCI() {
}


// checks rng against m_curDiode setting
bool  RootCI::isRngEnabled(enum CalXtalId::AdcRange rng) {
  if (m_curDiode == BOTH_DIODES) return true;
  if (m_curDiode == LE && (rng == CalXtalId::LEX8 || rng == CalXtalId::LEX1)) return true;
  if (m_curDiode == HE && (rng == CalXtalId::HEX8 || rng == CalXtalId::HEX1)) return true;
  return false;
}

// compiles stats for each test type.
void RootCI::DigiCal() {
  // Determine test config for this event (which xtal?, which dac?)
  int testCol   = m_evtId/m_cfg.nPulsesPerXtal;
  int testDAC   = (m_evtId%m_cfg.nPulsesPerXtal)/m_cfg.nPulsesPerDAC;

  const TObjArray* calDigiCol = m_evt->getCalDigiCol();
  if (!calDigiCol) {
    ostringstream temp;
    temp << "Empty calDigiCol event #" << m_evtId;
    throw temp.str();
  }
  TIter calDigiIter(calDigiCol);

  // Loop through each xtal interaction
  CalDigi *pdig = 0;
  while ((pdig = (CalDigi*)calDigiIter.Next())) {  //loop through each 'hit' in one event
    CalDigi cdig = *pdig; // use ref to reduce '->'

    CalXtalId id = cdig.getPackedId();  // get interaction information
    int col = id.getColumn();
    if (col != testCol) continue;

    int lyr = id.getLayer();

    // Loop through each readout on current xtal
    int numRo = cdig.getNumReadouts();
    for (int iRo=0; iRo<numRo; iRo++){
      const CalXtalReadout &acRo = *(cdig.getXtalReadout(iRo));

      // POS FACE
      CalXtalId::XtalFace face  = CalXtalId::POS;
      CalXtalId::AdcRange rng = (CalXtalId::AdcRange)acRo.getRange(face);
      int adc                 = acRo.getAdc(face);
      // only interested in current diode!
      if (!isRngEnabled(rng)) continue;
      // assign to table
      m_cfData.m_adcSum[lyr][col][face][rng][testDAC]   += adc;
      m_cfData.m_adcN[lyr][col][face][rng][testDAC]++;

      // NEG FACE
      face = CalXtalId::NEG;
      rng = (CalXtalId::AdcRange)acRo.getRange(face);
      adc = acRo.getAdc(face);
      // insanity check
      // assign to table
      m_cfData.m_adcSum[lyr][col][face][rng][testDAC]   += adc;
      m_cfData.m_adcN[lyr][col][face][rng][testDAC]++;

    } // foreach readout
  } // foreach xtal
}

void RootCI::Go(Int_t numEvents)
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
  Int_t nentries = getEntries();
  cout << "\nNum Events in File is: " << nentries << endl;
  Int_t curI;
  Int_t nMax = min(numEvents+m_startEvent,nentries);

  if (numEvents+m_startEvent >  nentries) {
    ostringstream temp;
    temp << " not enough entries in file to proceed, we need " << nentries;
    throw temp.str();
  }

  // BEGINNING OF EVENT LOOP
  for (Int_t ievent=m_startEvent; ievent<nMax; ievent++, curI=ievent) {
    if (m_evt) m_evt->Clear();

    getEvent(ievent);
    // Digi ONLY analysis
    if (m_evt) {
      m_evtId = m_evt->getEventId();
      if(m_evtId%1000 == 0)
        cout << " event " << m_evtId << endl;

      DigiCal();
    }
  }  // end analysis code in event loop

  m_startEvent = curI;
}

int main(int argc, char **argv) {
  // Load xml config file
  string cfgPath;
  if(argc > 1) cfgPath = argv[1];
  else cfgPath = "../src/ciFit_option.xml";

  try {

    cfCfg cfg;
    cfg.readCfgFile(cfgPath);
    cfData data(cfg);

    // LE PASS
    {
      vector<string> digiFileNames;
      digiFileNames.push_back(cfg.rootFileLE1);
      RootCI rd(digiFileNames,data,cfg);  
      // set HE/LE rng
      rd.SetDiode(RootCI::LE);
      rd.Go(cfg.nPulsesPerRun);
    }

    // HE PASS
    {
      vector<string> digiFileNames;
      digiFileNames.push_back(cfg.rootFileHE1);
      RootCI rd(digiFileNames, data,cfg);
      rd.SetDiode(RootCI::HE);
      rd.Go(cfg.nPulsesPerRun);
    }
#if 0 
    cfData data_high_thresh(&cfg);

    // LE PASS
    {
      vector<string> digiFileNames;
      digiFileNames.push_back(cfg.rootFileLE2);
      RootCI rd(digiFileNames,data_high_thresh,cfg);  
      // set HE/LE rng
      rd.SetDiode(RootCI::LE);
      rd.Go(cfg.nPulsesPerRun);
    }

    // HE PASS
    {
      vector<string> digiFileNames;
      digiFileNames.push_back(cfg.rootFileHE2);
      RootCI rd(digiFileNames, data_high_thresh, cfg);
      rd.SetDiode(RootCI::HE);
      rd.Go(cfg.nPulsesPerRun);
    }
#endif
    data.FitData();
    // data_high_thresh.FitData();
    // data.corr_FLE_Thresh(data_high_thresh);
    //data.ReadSplinesTXT("../output/ciSplines.txt");
    data.WriteSplinesTXT(cfg.outputTXTPath);
    data.WriteSplinesXML(cfg.outputXMLPath, cfg.dtdFile);
  } catch (string s) {
    // generic exception handler...  all my exceptions are simple C++ strings
    cout << "ciFit:  exception thrown: " << s << endl;
    return -1;
  }

  return 0;
}

