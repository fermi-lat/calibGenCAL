// LOCAL INCLUDES
#include "ICfg.h"
#include "CalDefs.h"
#include "CGCUtil.h"

// GLAST INCLUDES
#include "xml/IFile.h"
#include "facilities/Util.h"

// EXTLIB INCLUDES

// STD INCLUDES
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;
using namespace CalDefs;
using namespace CGCUtil;

class TholdCICfg : ICfg {
public:  // i know I'm not supposed to make data members public, but it's just easier this way!
  /// basic ctor
  TholdCICfg() {valid = false;}
  virtual ~TholdCICfg() {};

  /// clear all values, delete all pointers
  void clear();

  /// read in config data from config file, calculate dependent vars
  void readCfgFile(const string& cfgPath);

  /// return data valid flag.
  bool isValid() {return valid;}

  /// print summary to ostream
  void summarize();
  // CONFIGURABLE PARAMETERS //

  // SECTION: TEST_INFO //
  string timestamp;     ///< time of observation/measurement

  string instrument;    ///< instrument name "EM", "FM101", etc...
  vector<int> towerList; ///< list of installed towers in lat.

  // SECTION: PATHS
  string perFaceFileTXT; ///< input txt file for per-face threshold values (FLE,FHE,LAC)
  string perRangeFileTXT; ///< input txt file for per-rng threshold values (ULD & peds)
  string dtdFile; ///< Data descriptoin file for .xml output
  string outputDir; ///< folder for autonamed output files
  string tholdFileXML; ///< output xml filename for charge injection based threshold info

private:
  string baseFilename; ///< string shared by all autogenerated output filenames 

  // Section decription strings
  static const string TEST_INFO; ///< TEST_INFO xml IFile section name
  static const string PATHS; ///< PATHS xml IFile section name

  bool valid;   // set to false member data is incomplete/invalid.

};

const string TholdCICfg::TEST_INFO("TEST_INFO");
const string TholdCICfg::PATHS("PATHS");

void TholdCICfg::readCfgFile(const string& cfgPath) {
  clear();

  xml::IFile ifile(cfgPath.c_str());
  
  using facilities::Util;

  // SECTION: TEST INFO
  timestamp       = ifile.getString(TEST_INFO.c_str(), "TIMESTAMP");

  instrument      = ifile.getString(TEST_INFO.c_str(), "INSTRUMENT");
  towerList       = ifile.getIntVector(TEST_INFO.c_str(), "TOWER_LIST");

  // SECTION: PATHS
  dtdFile         = ifile.getString(PATHS.c_str(), "DTD_FILE");
  Util::expandEnvVar(&dtdFile);

  outputDir       = ifile.getString(PATHS.c_str(), "OUTPUT_FOLDER");
  Util::expandEnvVar(&outputDir);

  tholdFileXML    = ifile.getString(PATHS.c_str(), "THOLDFILE_XML");
  Util::expandEnvVar(&tholdFileXML);

  perFaceFileTXT  = ifile.getString(PATHS.c_str(), "PERFACE_FILE_TXT");
  Util::expandEnvVar(&perFaceFileTXT);

  perRangeFileTXT  = ifile.getString(PATHS.c_str(), "PERRANGE_FILE_TXT");
  Util::expandEnvVar(&perRangeFileTXT);

  //-- POST PROCESS --//

  // extract basename from 1st ROOT input file
  baseFilename = perFaceFileTXT;
  path_remove_dir(baseFilename);
  path_remove_ext(baseFilename);

  // make sure that outputDir has slash on end
  outputDir += '/';

  // autogenerate output filenames (only autogen if string is empty)
  if (tholdFileXML.length() == 0)
    tholdFileXML = outputDir + "thold_ci." + baseFilename + ".xml";
}

void TholdCICfg::clear() {
}

void TholdCICfg::summarize() {
}

//-- MAIN DATA STORAGE CLASS --//
class TholdCI {
public:
  // init arrays
  TholdCI(TholdCICfg &cfg) : 
    m_FLEVal(FaceIdx::N_VALS,0),
    m_FLESig(FaceIdx::N_VALS,0),
    m_FHEVal(FaceIdx::N_VALS,0),
    m_FHESig(FaceIdx::N_VALS,0),
    m_LACVal(FaceIdx::N_VALS,0),
    m_LACSig(FaceIdx::N_VALS,0),
    m_pedVal(RngIdx::N_VALS,0),
    m_pedSig(RngIdx::N_VALS,0),
    m_ULDVal(RngIdx::N_VALS,0),
    m_ULDSig(RngIdx::N_VALS,0),
    m_cfg(cfg)
  {}

  void readTXT(const string &perFaceFile, 
               const string &perRangeFile);
  void writeXML(const string &filename);

  //-- MAIN CALIB DATA STORAGE --//
  CalVec<FaceIdx,float> m_FLEVal;
  CalVec<FaceIdx,float> m_FLESig;
  CalVec<FaceIdx,float> m_FHEVal;
  CalVec<FaceIdx,float> m_FHESig;
  CalVec<FaceIdx,float> m_LACVal;
  CalVec<FaceIdx,float> m_LACSig;

  CalVec<RngIdx,float> m_pedVal;
  CalVec<RngIdx,float> m_pedSig;
  CalVec<RngIdx,float> m_ULDVal;
  CalVec<RngIdx,float> m_ULDSig;

private:
  TholdCICfg &m_cfg;
};

void TholdCI::readTXT(const string &perFaceFile,
		      const string &perRangeFile) {

  //-- PER FACE THRESHOLDS --//
  ifstream faceFile(perFaceFile.c_str());
  if (!faceFile.is_open())
    throw string("Unable to open " + perFaceFile);
  
  unsigned nRead = 0;
  while(faceFile.good()) {
    float fleVal, fleSig, fheVal, fheSig, lacVal, lacSig;
    short twr, lyr, col, face;
    faceFile >> twr
        >> lyr
	     >> col
	     >> face
	     >> fleVal
	     >> fleSig
	     >> fheVal
	     >> fheSig
	     >> lacVal
	     >> lacSig;
    if (faceFile.fail()) break; // quit once we can't read any more values
    nRead++;
    
    FaceIdx idx(twr,lyr, col, face);
    m_FLEVal[idx] = fleVal;
    m_FLESig[idx] = fleSig;
    m_FHEVal[idx] = fheVal;
    m_FHESig[idx] = fheSig;
    m_LACVal[idx] = lacVal;
    m_LACSig[idx] = lacSig;
  }
  
  if (nRead != m_FLEVal.size()) {
    ostringstream temp;
    temp << "TholdCI file '" << perFaceFile << "' is incomplete: " << nRead
         << " values read, " << m_FLEVal.size() << " vals required.";
    throw temp.str();
  }

  //-- PER RANGE THRESHOLDS --//
  ifstream rngFile(perRangeFile.c_str());
  if (!rngFile.is_open())
    throw string("Unable to open " + perRangeFile);
  
  nRead = 0;
  while(rngFile.good()) {
    float uldVal, uldSig, pedVal, pedSig;
    short twr, lyr, col, face, rng;
    rngFile >> twr
         >> lyr
	      >> col
	      >> face
	      >> rng
	      >> uldVal
	      >> uldSig
	      >> pedVal
	      >> pedSig;
    
    if (rngFile.fail()) break; // quit once we can't read any more values
    nRead++;
    
    RngIdx idx(0,lyr, col, face, rng);
    m_ULDVal[idx] = uldVal;
    m_ULDSig[idx] = uldSig;
    m_pedVal[idx] = pedVal;
    m_pedSig[idx] = pedSig;
  }
  
  if (nRead != m_ULDVal.size()) {
    ostringstream temp;
    temp << "TholdCI file '" << perRangeFile << "' is incomplete: " << nRead
         << " values read, " << m_FLEVal.size() << " vals required.";
    throw temp.str();
  }
}

void TholdCI::writeXML(const string &filename) {
  ofstream outfile(filename.c_str());
  ifstream dtdfile(m_cfg.dtdFile.c_str());
  
  if (!outfile.is_open())
    throw string("Unable to open " + filename);
  
  if (!dtdfile.is_open())
    throw string("Unable to open " + m_cfg.dtdFile);

  outfile << "<?xml version=\"1.0\"?>" << endl;
  
  // INSERT ENTIRE .DTD FILE //
  outfile << "<!DOCTYPE calCalib [";
  string temp;
  while (dtdfile.good()) {
    getline(dtdfile, temp);
    if (dtdfile.fail()) continue; // bad get
    outfile << temp << endl;
  }
  outfile << "]>" << endl;

  outfile << "<calCalib>" << endl;
  outfile << " <generic instrument=\"" << m_cfg.instrument <<"\" timestamp=\""<< m_cfg.timestamp <<"\"";
  outfile << " calibType=\"CAL_TholdCI\" fmtVersion=\"v2r2\" creator=\"" << CGCUtil::CVS_TAG << "\">" << endl;

  outfile << " </generic>" << endl;
  outfile << " <dimension nRow=\"1\" nCol=\"1\" nLayer=\"" << LyrNum::N_VALS 
          << "\" nXtal=\"" << ColNum::N_VALS 
          <<"\" nFace=\"" << FaceNum::N_VALS 
          << "\" nRange=\"" << 1 // there isonly one 'tholdCI' entry per xtal face for this type, not the usual 4. 
          << "\"/>" << endl;
  for (TwrNum twr; twr.isValid(); twr++) {
    outfile << " <tower iRow=\"0\" iCol=\"0\">"<< endl;

    for (LyrNum lyr; lyr.isValid(); lyr++) {
      outfile << "  <layer iLayer=\"" << lyr << "\">" << endl;
      for (ColNum col; col.isValid(); col++) {
        outfile << "   <xtal iXtal=\"" << col << "\">" << endl;
        for (FaceNum face; face.isValid(); face++) {
          FaceIdx faceIdx(twr,lyr,col,face);
          outfile << "    <face end=\"" << FaceNum::MNEM[face] << "\">" << endl;
          outfile << "      <tholdCI FLEVal=\"" << m_FLEVal[faceIdx] 
                  << "\" FLESig=\"" << m_FLESig[faceIdx]
                  << "\" FHEVal=\"" << m_FHEVal[faceIdx]
                  << "\" FHESig=\"" << m_FHESig[faceIdx]
                  << "\" LACVal=\"" << m_LACVal[faceIdx]
                  << "\" LACSig=\"" << m_LACSig[faceIdx]
                  << "\" >" << endl;
          for (RngNum rng; rng.isValid(); rng++) {
            RngIdx rngIdx(faceIdx,rng);
            outfile << "       <tholdCIRange range=\"" << rng.MNEM[rng] 
                    << "\" pedVal=\"" << m_pedVal[rngIdx]
                    << "\" pedSig=\"" << m_pedSig[rngIdx]
                    << "\" ULDVal=\"" << m_ULDVal[rngIdx]
                    << "\" ULDSig=\"" << m_ULDSig[rngIdx]
                    << "\" />" << endl;
          }
          outfile << "      </tholdCI>" << endl;
          outfile << "    </face>" << endl;
        }
        outfile << "   </xtal>" << endl;
      }
      outfile<<"  </layer>" << endl;
    }
    outfile << " </tower>"<< endl;
  }
  outfile << "</calCalib>" << endl;
}

int main(int argc, char** argv) {
  string cfgPath;
  if(argc > 1) cfgPath = argv[1];
  else cfgPath = "../src/genTholdCI_XML_option.xml";
    
  TholdCICfg cfg;
  try {
    cfg.readCfgFile(cfgPath);

    // identify calibGenCAL package version
    cout << "calibGenCAL CVS Tag: " << CGCUtil::CVS_TAG << endl;

    TholdCI tholds(cfg);

    tholds.readTXT(cfg.perFaceFileTXT,
                   cfg.perRangeFileTXT);
    tholds.writeXML(cfg.tholdFileXML);

  } catch (string s) {
    // generic exception handler...  all my exceptions are simple C++ strings
    cout << "TholdCI_XML:  exception thrown: " << s << endl;
    return -1;
  }

  return 0;
}
