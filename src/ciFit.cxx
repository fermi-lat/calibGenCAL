#include <fstream>
#include <iostream>
#include <string>
#include <string>
#include <vector>

#include "RootFileAnalysis.h"
#include "xml/IFile.h"

//ROOT INCLUDES
#include "TF1.h"
#include "TGraph.h"
#include "TSpline.h"

// CONSTANTS //
const int N_FACES  = 2;
const int N_LAYERS = 8;
const int N_RANGES = 4;
const int N_XTALS  = 12;
const int N_RANGES_PER_TOWER = N_FACES * N_LAYERS * N_RANGES * N_XTALS;
const double mu2ci_thr_rat = 1.7;

class cfCfg {
public:
  // basic ctor
  cfCfg() {m_isValid = false;}

  // clear all values, delete all pointers
  void clear();

  // read in config data from config file, calculate dependent vars
  int readCfgFile(const std::string& path);

  // return data valid flag.
  bool isValid() {return m_isValid;}

  // print summary to ostream
  void summarize(std::ostream &ostr);
public:  // i know, don't make members public, but it's just easier this way!
  // CONFIGURABLE PARAMETERS //

  // SECTION: TEST_INFO //
  std::string m_timestamp;
  std::string m_startTime;
  std::string m_stopTime; 

  std::string m_instrument;    
  std::vector<int> m_towerList;

  std::string m_triggerMode;   
  std::string m_instrumentMode;
  std::string m_source;

  std::vector<int> m_dacSettings;
  int m_nPulsesPerDac;

  // SECTION: PATHS //
  // current setup has us reading in 6 input files 2 ranges each and 3 different FLE settings
  std::string m_outputXMLPath;   
  std::string m_outputDTDPath;   
  std::string m_outputDTDVersion;
  std::string m_outputTXTPath;

  std::string m_infileType;

  std::string m_rootFileLE1;
  std::string m_rootFileHE1;
  std::string m_rootFileLE2;
  std::string m_rootFileHE2;
  std::string m_rootFileLE3;
  std::string m_rootFileHE3;

  std::string m_csvFileLE1;
  std::string m_csvFileHE1;
  std::string m_csvFileLE2;
  std::string m_csvFileHE2;
  std::string m_csvFileLE3;
  std::string m_csvFileHE3;
  
  // SECTION: SPLINE CONFIG //
  std::vector<int> m_splineGroupWidth;
  std::vector<int> m_splineSkipLow;
  std::vector<int> m_splineSkipHigh;
  std::vector<int> m_splineNPtsMin;

  // DERIVED FROM CONFIG PARMS //
  int m_nPulsesPerXtal;
  int m_nPulsesPerRun;
  int m_nDacs;

private:
  bool m_isValid;   // set to false member data is incomplete/invalid.

  // Section decription strings
  static const std::string m_testInfo;
  static const std::string m_paths;
  static const std::string m_splineCfg;

};

const std::string cfCfg::m_testInfo("TEST_INFO");
const std::string cfCfg::m_paths("PATHS");
const std::string cfCfg::m_splineCfg("SPLINE_CFG");

int cfCfg::readCfgFile(const std::string& path) {
  clear();

  xml::IFile ifile(path.c_str());
  
  // TEST INFO
  m_timestamp = ifile.getString(m_testInfo.c_str(), "TIMESTAMP");
  m_startTime = ifile.getString(m_testInfo.c_str(), "STARTTIME");
  m_stopTime  = ifile.getString(m_testInfo.c_str(), "STOPTIME");

  m_instrument    = ifile.getString(m_testInfo.c_str(), "INSTRUMENT");
  m_towerList     = ifile.getIntVector(m_testInfo.c_str(), "TOWER_LIST");

  m_triggerMode    = ifile.getString(m_testInfo.c_str(), "TRIGGER_MODE");
  m_instrumentMode = ifile.getString(m_testInfo.c_str(), "INST_MODE");
  m_source         = ifile.getString(m_testInfo.c_str(), "TEST_SOURCE");
  
  m_dacSettings        = ifile.getIntVector(m_testInfo.c_str(), "DAC_SETTINGS");
  m_nPulsesPerDac      = ifile.getInt(m_testInfo.c_str(), "N_PULSES_PER_DAC");

  // PATHS
  m_outputXMLPath      = ifile.getString(m_paths.c_str(), "XMLPATH");
  m_outputDTDPath      = ifile.getString(m_paths.c_str(), "DTDFILE");
  m_outputDTDVersion   = ifile.getString(m_paths.c_str(), "DTD_VERSION");
  m_outputTXTPath      = ifile.getString(m_paths.c_str(), "TXTPATH");

  m_infileType         = ifile.getString(m_paths.c_str(), "INFILE_TYPE");
  
  m_rootFileLE1 = ifile.getString(m_paths.c_str(), "ROOTFILE_LE1");
  m_rootFileHE1 = ifile.getString(m_paths.c_str(), "ROOTFILE_HE1");
  m_rootFileLE2 = ifile.getString(m_paths.c_str(), "ROOTFILE_LE2");
  m_rootFileHE2 = ifile.getString(m_paths.c_str(), "ROOTFILE_HE2");
  m_rootFileLE3 = ifile.getString(m_paths.c_str(), "ROOTFILE_LE3");
  m_rootFileHE3 = ifile.getString(m_paths.c_str(), "ROOTFILE_HE3");

  m_csvFileLE1 = ifile.getString(m_paths.c_str(), "CSVFILE_LE1");
  m_csvFileHE1 = ifile.getString(m_paths.c_str(), "CSVFILE_HE1");
  m_csvFileLE2 = ifile.getString(m_paths.c_str(), "CSVFILE_LE2");
  m_csvFileHE2 = ifile.getString(m_paths.c_str(), "CSVFILE_HE2");
  m_csvFileLE3 = ifile.getString(m_paths.c_str(), "CSVFILE_LE3");
  m_csvFileHE3 = ifile.getString(m_paths.c_str(), "CSVFILE_HE3");
  
  // SPLINE CFG
  m_splineGroupWidth = ifile.getIntVector(m_splineCfg.c_str(), "GROUP_WIDTH");
  m_splineSkipLow    = ifile.getIntVector(m_splineCfg.c_str(), "SKIP_LOW"  );
  m_splineSkipHigh   = ifile.getIntVector(m_splineCfg.c_str(), "SKIP_HIGH" );
  m_splineNPtsMin    = ifile.getIntVector(m_splineCfg.c_str(), "N_PTS_MIN" );
  
  // Geneate derived config quantities.
  m_nDacs          = m_dacSettings.size();
  m_nPulsesPerXtal = m_nPulsesPerDac * m_nDacs;
  m_nPulsesPerRun  = N_XTALS*m_nPulsesPerXtal;

  return 0;
}

void cfCfg::clear() {
  // SECTION: TEST_INFO //
  m_timestamp.clear();
  m_startTime.clear();
  m_stopTime.clear(); 

  m_instrument.clear();    
  m_towerList.clear();

  m_triggerMode.clear();   
  m_instrumentMode.clear();
  m_source.clear();

  m_dacSettings.clear();
  m_nPulsesPerDac = 0;

  // SECTION: PATHS //
  m_outputXMLPath.clear();   
  m_outputDTDPath.clear();   
  m_outputDTDVersion.clear();
  m_outputTXTPath.clear();

  m_infileType.clear();

  m_rootFileLE1.clear();
  m_rootFileHE1.clear();
  m_rootFileLE2.clear();
  m_rootFileHE2.clear();
  m_rootFileLE3.clear();
  m_rootFileHE3.clear();

  m_csvFileLE1.clear();
  m_csvFileHE1.clear();
  m_csvFileLE2.clear();
  m_csvFileHE2.clear();
  m_csvFileLE3.clear();
  m_csvFileHE3.clear();
  
  // SECTION: SPLINE CONFIG //
  m_splineGroupWidth.clear();
  m_splineSkipLow.clear();
  m_splineSkipHigh.clear();
  m_splineNPtsMin.clear();

  // DERIVED FROM CONFIG PARMS //
  m_nPulsesPerXtal = 0;
  m_nPulsesPerRun = 0;
  m_nDacs = 0;

  m_isValid = false;
}

void cfCfg::summarize(std::ostream &ostr) {
}


//////////////////////////////////////////////////////
// class cfData - represents one complete set of intNonlin data ////////
//////////////////////////////////////////////////////
class cfData {
  friend class RootCI;

public:

  cfData(cfCfg *cfg);
  ~cfData() {delete m_dacArr;}

  int FitData();
  int corr_FLE_Thresh(cfData& data_high_thresh);
  int WriteSplinesXML(const std::string &fileName);
  int WriteSplinesTXT(const std::string &fileName);
  int ReadSplinesTXT (const std::string &fileName);

  const int getNumSplineADC(const int xtal, const int layer, const int face, const int range) const {return m_numSplineADC[xtal][layer][face][range];}
  const std::vector<int>& getSplineDAC(int range) const {return m_splineDac[range];}
  const std::vector<float>& getSplineADC(const int xtal, const int layer, const int face, const int range) const {return m_splineADC[xtal][layer][face][range];}

  static const std::string RANGE_MNEM[];

private:
  TF1 splineFunc;

  std::vector<float>  m_adcSum[N_XTALS][N_LAYERS][N_FACES][N_RANGES];
  std::vector<int>    m_adcN[N_XTALS][N_LAYERS][N_FACES][N_RANGES];
  std::vector<float>  m_adcMean[N_XTALS][N_LAYERS][N_FACES][N_RANGES];

  std::vector<float>  m_splineADC[N_XTALS][N_LAYERS][N_FACES][N_RANGES];
  int                 m_numSplineADC[N_XTALS][N_LAYERS][N_FACES][N_RANGES];
  std::vector<int>    m_splineDac[N_RANGES];
  std::vector<int>    m_numSplineDac;

  cfCfg *m_cfg;
  // int c-style array copy of cfCfg::m_dacSettings needed by some fit routines
  float *m_dacArr;
};

const std::string cfData::RANGE_MNEM[] = {"LEX8",
                                        "LEX1",
                                        "HEX8",
                                        "HEX1"};

cfData::cfData(cfCfg *cfg) :
  splineFunc("spline_fitter","pol2",0,4095) {
  m_cfg = cfg;
  
  // need a c-style array of floats for DAC values.
  m_dacArr = new float[m_cfg->m_nDacs];
  for (int i = 0; i < m_cfg->m_nDacs; i++)
    m_dacArr[i] = m_cfg->m_dacSettings[i]; 

  // init vector arrays. c++ doesn't auto construct multidim vectors
  for (int xtal = 0; xtal < N_XTALS; xtal++)
    for (int layer = 0; layer < N_LAYERS; layer++)
      for (int face = 0; face < N_FACES; face++)
        for (int range = 0; range < N_RANGES; range++) {
          m_adcSum[xtal][layer][face][range] = std::vector<float>(m_cfg->m_nDacs);
          m_adcN[xtal][layer][face][range] = std::vector<int>(m_cfg->m_nDacs);
          m_adcMean[xtal][layer][face][range] = std::vector<float>(m_cfg->m_nDacs);
          m_splineADC[xtal][layer][face][range] = std::vector<float>(m_cfg->m_nDacs);
        }
  for (int range = 0; range < N_RANGES; range++)
    m_splineDac[range] = std::vector<int>(m_cfg->m_nDacs);
  
  m_numSplineDac = std::vector<int>(N_RANGES);
  
}

// smooth test lines & print output.
int cfData::FitData() {
  // 2 dimensional poly line f() to use for spline fitting.

  for (int range = 0; range < N_RANGES; range++) {
	 // following vals only change w/ range, so i'm getting them outside the other loops.
    int grpWid  = m_cfg->m_splineGroupWidth[range];
	 //int splLen  = grpWid*2 + 1;
    int skpLo   = m_cfg->m_splineSkipLow[range];
    int skpHi   = m_cfg->m_splineSkipHigh[range];
    int nPtsMin = m_cfg->m_splineNPtsMin[range];

	 // configure output stream format for rest of function
	 std::cout.setf(std::ios_base::fixed);
	 std::cout.precision(2);

	 for (int xtal = 0; xtal < N_XTALS; xtal++)
		for (int layer = 0; layer < N_LAYERS; layer++)
		  for (int face = 0; face < N_FACES; face++) {
           std::vector<float> &curADC = m_adcMean[xtal][layer][face][range];
			 // get pedestal
			 float ped     = m_adcSum[xtal][layer][face][range][0] /
				m_adcN[xtal][layer][face][range][0];

			 //calculate ped-subtracted means.
			 for (int dac = 0; dac < m_cfg->m_nDacs; dac++)
				curADC[dac] =
				  m_adcSum[xtal][layer][face][range][dac] /
				  m_adcN[xtal][layer][face][range][dac] - ped;

			 // get upper adc boundary
			 float adc_max = curADC[m_cfg->m_nDacs-1];
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
				m_splineDac[range][spl_idx] = (int)m_dacArr[i];
				m_splineADC[xtal][layer][face][range][spl_idx] = curADC[i];
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
				m_splineDac[range][spl_idx] = fitDac;
				m_splineADC[xtal][layer][face][range][spl_idx] = fitADC;
			 }

			 delete myGraph;

			 // copy SKPHI points directly from face of array.
			 for (int i = (nPtsMin-skpLo-skpHi)*grpWid + skpLo;
					i <= last_idx;
					i++,spl_idx++) {
				m_splineDac[range][spl_idx] = (int)m_dacArr[i];
				m_splineADC[xtal][layer][face][range][spl_idx] = curADC[i];
			 }

			 //
			 // UPDATE COUNTS
			 //
			 m_numSplineADC[xtal][layer][face][range] = spl_idx;
			 m_numSplineDac[range] = TMath::Max(m_numSplineDac[range],spl_idx);  // ensure we have just enough Dac points for largest spline
		  }
  }

  return 0;
}

int cfData::corr_FLE_Thresh(cfData& data_high_thresh){

	int range = 0;  // we correct LEX8 range only
	
	 for (int xtal = 0; xtal < N_XTALS; xtal++)
		for (int layer = 0; layer < N_LAYERS; layer++)
		  for (int face = 0; face < N_FACES; face++) {

				int numSpline = m_numSplineADC[xtal][layer][face][range];
			    double* arrADC = new double(numSpline);
				double* arrDAC = new double(numSpline);
				for (int i = 0; i<numSpline; i++){
					arrADC[i] = m_splineADC[xtal][layer][face][range][i];
					arrDAC[i] = m_splineDac[range][i];
				}
				int numSpline_hi_thr = data_high_thresh.getNumSplineADC(xtal,layer,face,range);
			    double* arrADC_hi_thr = new double(numSpline_hi_thr);
				double* arrDAC_hi_thr = new double(numSpline_hi_thr);
				const std::vector<float>& splineADC = data_high_thresh.getSplineADC(xtal,layer,face,range);
				const std::vector<int>& splineDAC = data_high_thresh.getSplineDAC(range);

					
				for (int i = 0; i<numSpline_hi_thr; i++){
					arrADC_hi_thr[i] = splineADC[i];
					arrDAC_hi_thr[i] = splineDAC[i];
				}

				TSpline3* spl = new TSpline3("spl",arrDAC,arrADC,numSpline); 
				TSpline3* spl_hi_thr = new TSpline3("spl_hi_thr",arrDAC_hi_thr,arrADC_hi_thr,numSpline_hi_thr); 

				for (int i = 0; i<numSpline;i++){
					float dac_corr = arrDAC[i]/mu2ci_thr_rat;
					m_splineADC[xtal][layer][face][range][i] = arrADC_hi_thr[i] + spl->Eval(dac_corr)-spl_hi_thr->Eval(dac_corr);
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

int cfData::WriteSplinesTXT(const std::string &fileName) {
  ofstream outFile(fileName.c_str());
  if (!outFile.is_open()) {
	 std::cout << "ERROR! unable to open txtFile='" << fileName << "'" << std::endl;
	 return -1;
  }
  outFile.precision(2);
  outFile.setf(std::ios_base::fixed);

  for (int xtal = 0; xtal < N_XTALS; xtal++) 
	 for (int layer = 0; layer < N_LAYERS; layer++)
		for (int face = 0; face < N_FACES; face++)
		  for (int range = 0; range < N_RANGES; range++)
			 for (int n = 0; n < m_numSplineADC[xtal][layer][face][range]; n++)
				outFile << xtal << " "
						  << layer  << " "
						  << face << " "
						  << range  << " "
						  << m_splineDac[range][n] << " "
						  << m_splineADC[xtal][layer][face][range][n]
						  << std::endl;
  
  return 0;
}

int cfData::ReadSplinesTXT (const std::string &fileName) {
  ifstream inFile(fileName.c_str());
  if (!inFile.is_open()) {
	 std::cout << "ERROR! unable to open txtFile='" << fileName << "'" << std::endl;
	 return -1;
  }

  int xtal, layer, face, range;
  int tmpDac;
  float tmpADC;
  while (inFile.good()) {
	 // load in one spline val w/ coords
	 inFile >> xtal 
			  >> layer
			  >> face
			  >> range
			  >> tmpDac
			  >> tmpADC;
	 
	 int cur_idx = m_numSplineADC[xtal][layer][face][range];
	 m_splineADC[xtal][layer][face][range][cur_idx] = tmpADC;
	 m_splineDac[range][cur_idx]                  = tmpDac;
	 
	 // update counters
	 m_numSplineADC[xtal][layer][face][range]++;
	 m_numSplineDac[range] = TMath::Max(m_numSplineDac[range],cur_idx+1);
  }

  return 0;
}

int cfData::WriteSplinesXML(const std::string &fileName) {
  // setup output file
  ofstream xmlFile(fileName.c_str());
  if (!xmlFile.is_open()) {
	 std::cout << "ERROR! unable to open xmlFile='" << fileName << "'" << std::endl;
	 return -1;
  }

  //
  // XML file header
  //
  xmlFile << "<?xml version=\"1.0\" ?>" << std::endl;
  xmlFile << "<!-- $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/ciFit.cxx,v 1.8 2004/08/24 18:40:17 fewtrell Exp $  -->" << std::endl;
  xmlFile << "<!-- Made-up  intNonlin XML file for EM, according to calCalib_v2r1.dtd -->" << std::endl;
  xmlFile << std::endl;
  xmlFile << "<!DOCTYPE calCalib SYSTEM \"" << m_cfg->m_outputDTDPath << "\" [] >" << std::endl;
  xmlFile << std::endl;
  xmlFile << "<calCalib>" << std::endl;
  xmlFile << "  <generic instrument=\"" << m_cfg->m_instrument
			 << "\" timestamp=\"" << m_cfg->m_startTime << "\"" << std::endl;
  xmlFile << "           calibType=\"CAL_IntNonlin\" fmtVersion=\"" << m_cfg->m_outputDTDVersion << "\">" << std::endl;
  xmlFile << std::endl;
  xmlFile << "    <inputSample startTime=\"" << m_cfg->m_startTime
			 << "\" stopTime=\"" << m_cfg->m_stopTime << "\"" << std::endl;
  xmlFile << "		triggers=\"" << m_cfg->m_triggerMode
			 << "\" mode=\"" << m_cfg->m_instrumentMode
			 << "\" source=\"" << m_cfg->m_source << "\" >" << std::endl;
  xmlFile << std::endl;
  xmlFile << "		Times are start and stop time of calibration run." << std::endl;
  xmlFile << "		Other attributes are just made up for code testing." << std::endl;
  xmlFile << "    </inputSample>" << std::endl;
  xmlFile << "  </generic>" << std::endl;
  xmlFile << std::endl;
  xmlFile << "<!-- EM instrument: 8 layers, 12 columns -->" << std::endl;
  xmlFile << std::endl;
  xmlFile << "<!-- number of collections of dac settings should normally be" << std::endl;
  xmlFile << "     0 (the default), if dacs aren't used to acquire data, or " << std::endl;
  xmlFile << "     equal to nRange -->" << std::endl;
  xmlFile << " <dimension nRow=\"" << 1 
			 << "\" nCol=\"" << 1 
			 << "\" nLayer=\"" << N_LAYERS 
			 << "\" nXtal=\"" << N_XTALS 
			 << "\" nFace=\"" << N_FACES 
			 << "\" nRange=\"" << N_RANGES << "\"" << std::endl;
  xmlFile << "           nDacCol=\"" << N_RANGES << "\" />" << std::endl;

  //
  // Dac values for rest of file.
  //
  xmlFile << std::endl;
  for (int range = 0; range < N_RANGES; range++) {
    xmlFile << " <dac range=\"" << RANGE_MNEM[range] << "\"" << std::endl;
	 xmlFile << "     values=\"";
	 for (int i = 0; i < m_numSplineDac[range]; i++) 
		xmlFile << m_splineDac[range][i] << " ";
	 xmlFile << "\"" << std::endl;
	 xmlFile << "     error=\"" << 0.1 << "\" />" << std::endl;
  }
  
  //
  // main data loop
  //
  
  xmlFile.setf(std::ios_base::fixed);
  xmlFile.precision(2);
  // TOWER // currently only using 1 tower.
  xmlFile << std::endl;
  xmlFile << " <tower iRow=\"" << 0 << "\" iCol=\"" << 0 << "\">" << std::endl;
  // LAYER //
  for (int layer = 0; layer < N_LAYERS; layer++) {
	 xmlFile << "  <layer iLayer=\"" << layer << "\">" << std::endl;
	 // XTAL //
	 for (int xtal = 0; xtal < N_XTALS; xtal++) {
		xmlFile << "   <xtal iXtal=\"" << xtal << "\">" << std::endl;
		// FACE //
		for (int face = 0; face < N_FACES; face++) {
		  const std::string facestr = (face == CalXtalId::NEG) ? "NEG" : "POS";
		  xmlFile << "    <face end=\"" << facestr << "\">" << std::endl;
		  // RANGE //
		  for (int range = 0; range < N_RANGES; range++) {
          xmlFile << "     <intNonlin range=\"" << RANGE_MNEM[range] << "\"" << std::endl;
			 // ADC VALS //
			 xmlFile << "             values=\"";
			 for (int i = 0; i < m_numSplineADC[xtal][layer][face][range]; i++) {
				xmlFile << m_splineADC[xtal][layer][face][range][i] << " ";
			 }
			 xmlFile << "\"" << std::endl;
		    xmlFile << "             error=\"" << 0.1 << "\" />" << std::endl;
		  }
		  xmlFile << "    </face>" << std::endl;
		}
		xmlFile << "   </xtal>" << std::endl;
	 }
	 xmlFile << "  </layer>" << std::endl;
  }
  xmlFile << " </tower>" << std::endl;
  xmlFile << "</calCalib>" << std::endl;
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
  RootCI(std::vector<std::string> *digiFileNames,
			cfData  *data, cfCfg *cfg);

  // standard dtor
  ~RootCI();

  // processes one 'event', as a collection of interactions.
  // loads up histograms.
  void DigiCal();

  // loops through all events in file
  void Go(Int_t numEvents);

  void SetDiode(RootCI::Diode d) {m_curDiode = d;}

private:
  bool   isRangeEnabled(enum CalXtalId::AdcRange range);          // checks range against m_curDiode setting
  Diode m_curDiode;

  cfData *m_cfData;
  cfCfg  *m_cfg;

};


RootCI::RootCI(std::vector<std::string> *digiFileNames, cfData  *data, cfCfg *cfg) :
  RootFileAnalysis(digiFileNames, 0, 0)
{
  m_cfData   = data;
  m_cfg      = cfg;
  m_curDiode = BOTH_DIODES;
}

// default dstor
RootCI::~RootCI() {
}


// checks range against m_curDiode setting
bool  RootCI::isRangeEnabled(enum CalXtalId::AdcRange range) {
  if (m_curDiode == BOTH_DIODES) return true;
  if (m_curDiode == LE && (range == CalXtalId::LEX8 || range == CalXtalId::LEX1)) return true;
  if (m_curDiode == HE && (range == CalXtalId::HEX8 || range == CalXtalId::HEX1)) return true;
  return false;
}

// compiles stats for each test type.
void RootCI::DigiCal() {
  // Determine test config for this event (which xtal?, which dac?)
  int testXtal   = digiEventId/m_cfg->m_nPulsesPerXtal;
  int testDac   = (digiEventId%m_cfg->m_nPulsesPerXtal)/m_cfg->m_nPulsesPerDac;

  const TObjArray* calDigiCol = evt->getCalDigiCol();
  if (!calDigiCol) return;
  TIter calDigiIter(calDigiCol);

  // Loop through each xtal interaction
  CalDigi *cdig = 0;
  while ((cdig = (CalDigi *)calDigiIter.Next())) {  //loop through each 'hit' in one event
	 CalXtalId id = cdig->getPackedId();  // get interaction information
    int xtal = id.getColumn();
	 if (xtal != testXtal) continue;

    int layer = id.getLayer();

	 // Loop through each readout on current xtal
	 int numRo = cdig->getNumReadouts();
	 for (int iRo=0; iRo<numRo; iRo++){
		const CalXtalReadout* acRo=cdig->getXtalReadout(iRo);

      // POS FACE
      CalXtalId::XtalFace face  = CalXtalId::POS;
      CalXtalId::AdcRange range = (CalXtalId::AdcRange)acRo->getRange(face);
		int adc                 = acRo->getAdc(face);
		// only interested in current diode!
		if (!isRangeEnabled(range)) continue;
		// assign to table
      m_cfData->m_adcSum[xtal][layer][face][range][testDac]   += adc;
      m_cfData->m_adcN[xtal][layer][face][range][testDac]++;

		// NEG FACE
		face = CalXtalId::NEG;
		range = (CalXtalId::AdcRange)acRo->getRange(face);
		adc = acRo->getAdc(face);
		// insanity check
		// assign to table
      m_cfData->m_adcSum[xtal][layer][face][range][testDac]   += adc;
      m_cfData->m_adcN[xtal][layer][face][range][testDac]++;

	 } // foreach readout
  } // foreach xtal
}

void RootCI::Go(Int_t numEvents)
{
  // Purpose and Method:  Event Loop

  //
  //  COMMENT OUT ANY BRANCHES YOU ARE NOT INTERESTED IN.
  //
  if (m_digiChain) {
	 m_digiChain->SetBranchStatus("*",0);  // disable all branches
	 // activate desired brances
	 m_digiChain->SetBranchStatus("m_cal*",1);
	 m_digiChain->SetBranchStatus("m_eventId", 1);
	 //digiChain->SetBranchStatus("m_runId", 1);
	 //digiChain->SetBranchStatus("m_timeStamp", 1);
  }

  //
  // DO WE HAVE ENOUGH EVENTS IN FILE?
  //
  Int_t nentries = GetEntries();
  std::cout << "\nNum Events in File is: " << nentries << std::endl;
  Int_t curI;
  Int_t nMax = TMath::Min(numEvents+m_StartEvent,nentries);

  if (numEvents+m_StartEvent >  nentries) {
	 std::cout << " not enough entries in file to proceed, we need " << nentries << std::endl;
	 return;
  }

  // BEGINNING OF EVENT LOOP
  for (Int_t ievent=m_StartEvent; ievent<nMax; ievent++, curI=ievent) {
	 if (evt) evt->Clear();
	 if (rec) rec->Clear();

	 GetEvent(ievent);
	 // Digi ONLY analysis
	 if (evt) {
		digiEventId = evt->getEventId();
		//digiRunNum = evt->getRunId();
      if(digiEventId%1000 == 0)
		  std::cout << " event " << digiEventId << std::endl;

		DigiCal();
	 }
  }  // end analysis code in event loop

  m_StartEvent = curI;
}

int main(int argc, char **argv) {
  // Load xml config file
  std::string cfgPath;
  if(argc > 1) cfgPath = argv[1];
  else cfgPath = "../src/ciFit_option.xml";

  cfCfg cfg;
  if (cfg.readCfgFile(cfgPath) != 0) {
    std::cout << "Error reading config file: " << cfgPath << std::endl;
    return -1;
  }

  cfData data(&cfg);

  // LE PASS
  {
    std::vector<std::string> digiFileNames;
    digiFileNames.push_back(cfg.m_rootFileLE1);
    RootCI rd(&digiFileNames,&data,&cfg);  
    // set HE/LE range
    rd.SetDiode(RootCI::LE);
    rd.Go(cfg.m_nPulsesPerRun);
  }

  // HE PASS
  {
    std::vector<std::string> digiFileNames;
    digiFileNames.push_back(cfg.m_rootFileHE1);
    RootCI rd(&digiFileNames, &data,&cfg);
    rd.SetDiode(RootCI::HE);
    rd.Go(cfg.m_nPulsesPerRun);
  }
 
  cfData data_high_thresh(&cfg);

  // LE PASS
  {
    std::vector<std::string> digiFileNames;
    digiFileNames.push_back(cfg.m_rootFileLE2);
    RootCI rd(&digiFileNames,&data_high_thresh,&cfg);  
    // set HE/LE range
    rd.SetDiode(RootCI::LE);
    rd.Go(cfg.m_nPulsesPerRun);
  }

  // HE PASS
  {
    std::vector<std::string> digiFileNames;
    digiFileNames.push_back(cfg.m_rootFileHE2);
    RootCI rd(&digiFileNames, &data_high_thresh, &cfg);
    rd.SetDiode(RootCI::HE);
    rd.Go(cfg.m_nPulsesPerRun);
  }


  data.FitData();
  // data_high_thresh.FitData();
  // data.corr_FLE_Thresh(data_high_thresh);
  //data->ReadSplinesTXT("../output/ciSplines.txt");
  data.WriteSplinesTXT(cfg.m_outputTXTPath);
  data.WriteSplinesXML(cfg.m_outputXMLPath);

  return 0;
}

