#include <fstream>
#include <iostream>
#include <string>
#include <vector>

//ROOT INCLUDES
#include "TF1.h"
#include "TGraph.h"
#include "TSpline.h"

#include "xml/IFile.h"
#include "idents/CalXtalId.h"

#include "RootFileAnalysis.h"
#include "ICfg.h"

using namespace std;

// CONSTANTS //
const int N_FACES  = 2;
const int N_LYRS = 8;
const int N_RNGS = 4;
const int N_COLS  = 12;
const int N_RNGS_PER_TOWER = N_FACES * N_LYRS * N_RNGS * N_COLS;
const double mu2ci_thr_rat = 1.7;

////////////////////////// GENERAL UTILITIES //////////////////////////////////////////

/// finds position of last directory delimeter ('/' || '\\')
/// in path, returns -1 if no delim is found
string::size_type path_find_last_delim(string &path) {
  // find last directory delimeter.
  string::size_type fwdslash_pos = path.find_last_of('/');
  string::size_type bckslash_pos = path.find_last_of('\\');

  // check for 'not found' case
  if (fwdslash_pos == path.npos) fwdslash_pos = path.npos;
  if (bckslash_pos == path.npos) bckslash_pos = path.npos;

  return max(fwdslash_pos,bckslash_pos);

}

string &path_remove_dir(string &path) {
  string::size_type slash_pos;
  
  // if there was no delimeter, return path unaltered
  if ((slash_pos = path_find_last_delim(path)) == path.npos) 
    return path;

  // else remove everything up to & including the delimeter
  path.erase(0,slash_pos+1);

  return path;
}

/// removes filename extension from end of path string.
string &path_remove_ext(string &path) {
  // return path unaltered if there is no '.'
  string::size_type dot_pos;
  if ((dot_pos = path.find_last_of('.')) == path.npos)
    return path;

  // find last delim (extension must be after this point)
  string::size_type slash_pos = path_find_last_delim(path);

  // if there is no '/' then just erase everything from '.' onward
  // or if slash is before the '.'
  if (slash_pos == path.npos || slash_pos < dot_pos)
    path.erase(dot_pos, path.size());

  // otherwise return the string as is
  return path;
}

///////////////////////////////////////////////////////////////////////////////////


class cfCfg : ICfg {
public:
  // basic ctor
  cfCfg() : valid(false) {};
  virtual ~cfCfg() {};

  // clear all values, delete all pointers
  void clear();

  // read in config data from config file, calculate dependent vars
  int readCfgFile(const string& path);

  // return data valid flag.
  bool isValid() {return valid;}

  // print summary to ostream
  void summarize();
public:  // i know, don't make members public, but it's just easier this way!
  // CONFIGURABLE PARAMETERS //

  // SECTION: TEST_INFO //
  string timestamp;
  string startTime;
  string stopTime; 

  string instrument;    
  vector<int> towerList;

  string triggerMode;   
  string instrumentMode;
  string source;

  vector<int> dacSettings;
  int nPulsesPerDac;

  // SECTION: PATHS //
  // current setup has us reading in 6 input files 2 rngs each and 3 different FLE settings
  string outputDir; ///< folder for autonamed output files
  string outputXMLPath;   
  string outputTXTPath;

  string dtdFile; ///< Data descriptoin file for .xml output

  string rootFileLE1;
  string rootFileHE1;
  string rootFileLE2;
  string rootFileHE2;
  string rootFileLE3;
  string rootFileHE3;
  
  // SECTION: SPLINE CONFIG //
  vector<int> splineGroupWidth;
  vector<int> splineSkipLow;
  vector<int> splineSkipHigh;
  vector<int> splineNPtsMin;

  // DERIVED FROM CONFIG PARMS //
  int nPulsesPerXtal;
  int nPulsesPerRun;
  int nDacs;

private:
  bool valid;   // set to false member data is incomplete/invalid.

  string baseFilename; ///< used as the base for auto-generating output filenames.  derived from input root filename.

  // Section decription strings
  static const string testInfo;
  static const string paths;
  static const string splineCfg;

};

const string cfCfg::testInfo("TEST_INFO");
const string cfCfg::paths("PATHS");
const string cfCfg::splineCfg("SPLINE_CFG");

int cfCfg::readCfgFile(const string& path) {
  clear();

  xml::IFile ifile(path.c_str());
  
  // TEST INFO
  timestamp = ifile.getString(testInfo.c_str(), "TIMESTAMP");
  startTime = ifile.getString(testInfo.c_str(), "STARTTIME");
  stopTime  = ifile.getString(testInfo.c_str(), "STOPTIME");

  instrument    = ifile.getString(testInfo.c_str(), "INSTRUMENT");
  towerList     = ifile.getIntVector(testInfo.c_str(), "TOWER_LIST");

  triggerMode    = ifile.getString(testInfo.c_str(), "TRIGGER_MODE");
  instrumentMode = ifile.getString(testInfo.c_str(), "INST_MODE");
  source         = ifile.getString(testInfo.c_str(), "TEST_SOURCE");
  
  dacSettings        = ifile.getIntVector(testInfo.c_str(), "DAC_SETTINGS");
  nPulsesPerDac      = ifile.getInt(testInfo.c_str(), "N_PULSES_PER_DAC");

  // PATHS
  outputDir = ifile.getString(paths.c_str(), "OUTPUT_FOLDER");

  dtdFile = ifile.getString(paths.c_str(), "DTD_FILE");

  outputXMLPath      = ifile.getString(paths.c_str(), "XMLPATH");
  outputTXTPath      = ifile.getString(paths.c_str(), "TXTPATH");
  
  rootFileLE1 = ifile.getString(paths.c_str(), "ROOTFILE_LE1");
  rootFileHE1 = ifile.getString(paths.c_str(), "ROOTFILE_HE1");
  rootFileLE2 = ifile.getString(paths.c_str(), "ROOTFILE_LE2");
  rootFileHE2 = ifile.getString(paths.c_str(), "ROOTFILE_HE2");
  rootFileLE3 = ifile.getString(paths.c_str(), "ROOTFILE_LE3");
  rootFileHE3 = ifile.getString(paths.c_str(), "ROOTFILE_HE3");

  // SPLINE CFG
  splineGroupWidth = ifile.getIntVector(splineCfg.c_str(), "GROUP_WIDTH");
  splineSkipLow    = ifile.getIntVector(splineCfg.c_str(), "SKIP_LOW"  );
  splineSkipHigh   = ifile.getIntVector(splineCfg.c_str(), "SKIP_HIGH" );
  splineNPtsMin    = ifile.getIntVector(splineCfg.c_str(), "N_PTS_MIN" );
  
  // Geneate derived config quantities.
  nDacs          = dacSettings.size();
  nPulsesPerXtal = nPulsesPerDac * nDacs;
  nPulsesPerRun  = N_COLS*nPulsesPerXtal;

  baseFilename = rootFileLE1;
  path_remove_dir(baseFilename);
  path_remove_ext(baseFilename);

  // Auto-generate output filenames
  if (outputXMLPath.length() == 0)
    outputXMLPath = outputDir + "ciFit." + baseFilename + ".xml";
  if (outputTXTPath.length() == 0)
    outputTXTPath = outputDir + "ciFit." + baseFilename + ".txt";

  return 0;
}

void cfCfg::clear() {  
}

void cfCfg::summarize() {
}


//////////////////////////////////////////////////////
// class cfData - represents one complete set of intNonlin data ////////
//////////////////////////////////////////////////////
class cfData {
  friend class RootCI;

public:

  cfData(cfCfg &cfg);
  ~cfData() {delete m_dacArr;}

  int FitData();
  int corr_FLE_Thresh(cfData& data_high_thresh);
  int WriteSplinesXML(const string &filename, const string &dtdFilename);
  int WriteSplinesTXT(const string &filename);
  int ReadSplinesTXT (const string &filename);

  const int getNumSplineADC(const int lyr, const int col, const int face, const int rng) const {return m_numSplineADC[lyr][col][face][rng];}
  const vector<int>& getSplineDAC(int rng) const {return m_splineDac[rng];}
  const vector<float>& getSplineADC(const int lyr, const int col, const int face, const int rng) const {return m_splineADC[lyr][col][face][rng];}

  static const string RNG_MNEM[];

private:
  TF1 splineFunc;

  vector<float>  m_adcSum[N_LYRS][N_COLS][N_FACES][N_RNGS];
  vector<int>    m_adcN[N_LYRS][N_COLS][N_FACES][N_RNGS];
  vector<float>  m_adcMean[N_LYRS][N_COLS][N_FACES][N_RNGS];

  vector<float>  m_splineADC[N_LYRS][N_COLS][N_FACES][N_RNGS];
  int                 m_numSplineADC[N_LYRS][N_COLS][N_FACES][N_RNGS];
  vector<int>    m_splineDac[N_RNGS];
  vector<int>    m_numSplineDac;

  cfCfg &m_cfg;
  // int c-style array copy of cfCfg::m_dacSettings needed by some fit routines
  float *m_dacArr;
};

const string cfData::RNG_MNEM[] = {"LEX8",
                                   "LEX1",
                                   "HEX8",
                                   "HEX1"};

cfData::cfData(cfCfg &cfg) :
  splineFunc("spline_fitter","pol2",0,4095),
  m_cfg(cfg)
{

  // need a c-style array of floats for DAC values.
  m_dacArr = new float[m_cfg.nDacs];
  for (int i = 0; i < m_cfg.nDacs; i++)
    m_dacArr[i] = m_cfg.dacSettings[i]; 

  // init vector arrays. c++ doesn't auto construct multidim vectors
  for (int lyr = 0; lyr < N_LYRS; lyr++)
    for (int col = 0; col < N_COLS; col++)
      for (int face = 0; face < N_FACES; face++)
        for (int rng = 0; rng < N_RNGS; rng++) {
          m_adcSum[lyr][col][face][rng] = vector<float>(m_cfg.nDacs);
          m_adcN[lyr][col][face][rng] = vector<int>(m_cfg.nDacs);
          m_adcMean[lyr][col][face][rng] = vector<float>(m_cfg.nDacs);
          m_splineADC[lyr][col][face][rng] = vector<float>(m_cfg.nDacs);
        }
  for (int rng = 0; rng < N_RNGS; rng++)
    m_splineDac[rng] = vector<int>(m_cfg.nDacs);
  
  m_numSplineDac = vector<int>(N_RNGS);
  
}

// smooth test lines & print output.
int cfData::FitData() {
  // 2 dimensional poly line f() to use for spline fitting.

  for (int rng = 0; rng < N_RNGS; rng++) {
    // following vals only change w/ rng, so i'm getting them outside the other loops.
    int grpWid  = m_cfg.splineGroupWidth[rng];
    //int splLen  = grpWid*2 + 1;
    int skpLo   = m_cfg.splineSkipLow[rng];
    int skpHi   = m_cfg.splineSkipHigh[rng];
    int nPtsMin = m_cfg.splineNPtsMin[rng];

    // configure output stream format for rest of function
    cout.setf(ios_base::fixed);
    cout.precision(2);

    for (int lyr = 0; lyr < N_LYRS; lyr++)
      for (int col = 0; col < N_COLS; col++)
        for (int face = 0; face < N_FACES; face++) {
          vector<float> &curADC = m_adcMean[lyr][col][face][rng];
          // get pedestal
          float ped     = m_adcSum[lyr][col][face][rng][0] /
            m_adcN[lyr][col][face][rng][0];

          //calculate ped-subtracted means.
          for (int dac = 0; dac < m_cfg.nDacs; dac++)
            curADC[dac] =
              m_adcSum[lyr][col][face][rng][dac] /
              m_adcN[lyr][col][face][rng][dac] - ped;

          // get upper adc boundary
          float adc_max = curADC[m_cfg.nDacs-1];
          int last_idx = 0; // last idx will be 1st index that is > .99*adc_max
          while (curADC[last_idx] < .99*adc_max)
            last_idx++;
          adc_max = curADC[last_idx];

          // set up new graph object for fitting.
          float *tmpADC = new float[curADC.size()];
          for (unsigned i = 0; i < curADC.size(); i++) tmpADC[i] = curADC[i];
          TGraph *myGraph = new TGraph(last_idx+1,
                                       m_dacArr,
                                       tmpADC);
          delete (tmpADC);

          // copy SKPLO points directly from beginning of array.
          int spl_idx = 0;
          for (int i = 0; i < skpLo; i++,spl_idx++) {
            m_splineDac[rng][spl_idx] = (int)m_dacArr[i];
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

            myGraph->Fit(&splineFunc,"QN","",m_dacArr[lp],m_dacArr[hp]);
            float myPar1 = splineFunc.GetParameter(0);
            float myPar2 = splineFunc.GetParameter(1);
            float myPar3 = splineFunc.GetParameter(2);

            int   fitDac = (int)m_dacArr[cp];
            float fitADC = myPar1 + fitDac*(myPar2 + fitDac*myPar3);

            // output result
            m_splineDac[rng][spl_idx] = fitDac;
            m_splineADC[lyr][col][face][rng][spl_idx] = fitADC;
          }

          delete myGraph;

          // copy SKPHI points directly from face of array.
          for (int i = (nPtsMin-skpLo-skpHi)*grpWid + skpLo;
               i <= last_idx;
               i++,spl_idx++) {
            m_splineDac[rng][spl_idx] = (int)m_dacArr[i];
            m_splineADC[lyr][col][face][rng][spl_idx] = curADC[i];
          }

          //
          // UPDATE COUNTS
          //
          m_numSplineADC[lyr][col][face][rng] = spl_idx;
          m_numSplineDac[rng] = TMath::Max(m_numSplineDac[rng],spl_idx);  // ensure we have just enough Dac points for largest spline
        }
  }

  return 0;
}

int cfData::corr_FLE_Thresh(cfData& data_high_thresh){

  int rng = 0;  // we correct LEX8 rng only

  for (int lyr = 0; lyr < N_LYRS; lyr++)
    for (int col = 0; col < N_COLS; col++)
      for (int face = 0; face < N_FACES; face++) {

        int numSpline = m_numSplineADC[lyr][col][face][rng];
        double* arrADC = new double(numSpline);
        double* arrDAC = new double(numSpline);
        for (int i = 0; i<numSpline; i++){
          arrADC[i] = m_splineADC[lyr][col][face][rng][i];
          arrDAC[i] = m_splineDac[rng][i];
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
  return 0;
}

int cfData::WriteSplinesTXT(const string &filename) {
  ofstream outFile(filename.c_str());
  if (!outFile.is_open()) {
    cout << "ERROR! unable to open txtFile='" << filename << "'" << endl;
    return -1;
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
                    << m_splineDac[rng][n] << " "
                    << m_splineADC[lyr][col][face][rng][n]
                    << endl;
  
  return 0;
}

int cfData::ReadSplinesTXT (const string &filename) {
  ifstream inFile(filename.c_str());
  if (!inFile.is_open()) {
    cout << "ERROR! unable to open txtFile='" << filename << "'" << endl;
    return -1;
  }

  int lyr, col, face, rng;
  int tmpDac;
  float tmpADC;
  while (inFile.good()) {
    // load in one spline val w/ coords
    inFile >> lyr 
           >> col
           >> face
           >> rng
           >> tmpDac
           >> tmpADC;
 
    int cur_idx = m_numSplineADC[lyr][col][face][rng];
    m_splineADC[lyr][col][face][rng][cur_idx] = tmpADC;
    m_splineDac[rng][cur_idx]                  = tmpDac;
 
    // update counters
    m_numSplineADC[lyr][col][face][rng]++;
    m_numSplineDac[rng] = TMath::Max(m_numSplineDac[rng],cur_idx+1);
  }

  return 0;
}

int cfData::WriteSplinesXML(const string &filename, const string &dtdFilename) {
  // setup output file
  ofstream xmlFile(filename.c_str());
  if (!xmlFile.is_open()) {
    cout << "ERROR! unable to open xmlFile='" << filename << "'" << endl;
    return -1;
  }
  ifstream dtdFile(dtdFilename.c_str());
  if (!dtdFile.is_open()) {
     cout << "ERROR! unable to open dtdFile='" << dtdFilename << "'" << endl;
     return -1;
  }

  //
  // XML file header
  //
  xmlFile << "<?xml version=\"1.0\" ?>" << endl;
  xmlFile << "<!-- $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/ciFit.cxx,v 1.10 2004/12/23 02:13:29 fewtrell Exp $  -->" << endl;
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
  xmlFile << "           calibType=\"CAL_IntNonlin\" fmtVersion=\"v2r2\">" << endl;
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
  xmlFile << "           nDacCol=\"" << N_RNGS << "\" />" << endl;

  //
  // Dac values for rest of file.
  //
  xmlFile << endl;
  for (int rng = 0; rng < N_RNGS; rng++) {
    xmlFile << " <dac range=\"" << RNG_MNEM[rng] << "\"" << endl;
    xmlFile << "     values=\"";
    for (int i = 0; i < m_numSplineDac[rng]; i++) 
      xmlFile << m_splineDac[rng][i] << " ";
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
  return 0;
}

//////////////////////////////////////////////////////
// class RootCI - derived from RootFileAnalysis - represents all Root input //
// needed for populating the cfData class                                //
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
  RootFileAnalysis(digiFileNames, vector<string>(0), vector<string>(0)),
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
  int testDac   = (m_evtId%m_cfg.nPulsesPerXtal)/m_cfg.nPulsesPerDac;

  const TObjArray* calDigiCol = m_evt->getCalDigiCol();
  if (!calDigiCol) return;
  TIter calDigiIter(calDigiCol);

  // Loop through each xtal interaction
  CalDigi *cdig = 0;
  while ((cdig = (CalDigi*)calDigiIter.Next())) {  //loop through each 'hit' in one event
    CalXtalId id = cdig->getPackedId();  // get interaction information
    int col = id.getColumn();
    if (col != testCol) continue;

    int lyr = id.getLayer();

    // Loop through each readout on current xtal
    int numRo = cdig->getNumReadouts();
    for (int iRo=0; iRo<numRo; iRo++){
      const CalXtalReadout &acRo = *(cdig->getXtalReadout(iRo));

      // POS FACE
      CalXtalId::XtalFace face  = CalXtalId::POS;
      CalXtalId::AdcRange rng = (CalXtalId::AdcRange)acRo.getRange(face);
      int adc                 = acRo.getAdc(face);
      // only interested in current diode!
      if (!isRngEnabled(rng)) continue;
      // assign to table
      m_cfData.m_adcSum[lyr][col][face][rng][testDac]   += adc;
      m_cfData.m_adcN[lyr][col][face][rng][testDac]++;

      // NEG FACE
      face = CalXtalId::NEG;
      rng = (CalXtalId::AdcRange)acRo.getRange(face);
      adc = acRo.getAdc(face);
      // insanity check
      // assign to table
      m_cfData.m_adcSum[lyr][col][face][rng][testDac]   += adc;
      m_cfData.m_adcN[lyr][col][face][rng][testDac]++;

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
    //digiChain->SetBranchStatus("m_timeStamp", 1);
  }

  //
  // DO WE HAVE ENOUGH EVENTS IN FILE?
  //
  Int_t nentries = getEntries();
  cout << "\nNum Events in File is: " << nentries << endl;
  Int_t curI;
  Int_t nMax = TMath::Min(numEvents+m_startEvent,nentries);

  if (numEvents+m_startEvent >  nentries) {
    cout << " not enough entries in file to proceed, we need " << nentries << endl;
    return;
  }

  // BEGINNING OF EVENT LOOP
  for (Int_t ievent=m_startEvent; ievent<nMax; ievent++, curI=ievent) {
    if (m_evt) m_evt->Clear();
    if (m_rec) m_rec->Clear();

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

  cfCfg cfg;
  if (cfg.readCfgFile(cfgPath) != 0) {
    cout << "Error reading config file: " << cfgPath << endl;
    return -1;
  }

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
  //data->ReadSplinesTXT("../output/ciSplines.txt");
  data.WriteSplinesTXT(cfg.outputTXTPath);
  data.WriteSplinesXML(cfg.outputXMLPath, cfg.dtdFile);

  return 0;
}

