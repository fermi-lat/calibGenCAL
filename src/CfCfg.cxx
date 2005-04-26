// LOCAL INCLUDES
#include "CfCfg.h"
#include "CalDefs.h"
#include "CGCUtil.h"

// GLAST INCLUDES
#include "facilities/Util.h"

// EXTLIB INCLUDES

// STD INCLUDES
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>

using namespace CGCUtil;

const string CfCfg::TEST_INFO("TEST_INFO");
const string CfCfg::PATHS("PATHS");
const string CfCfg::SPLINE_CFG("SPLINE_CFG");
const string CfCfg::GENERAL("GENERAL");

void CfCfg::readCfgFile(const string& path) {
  clear();

  xmlBase::IFile ifile(path.c_str());
  
  // TEST INFO
  timestamp  = ifile.getString(TEST_INFO.c_str(), "TIMESTAMP");
  instrument = ifile.getString(TEST_INFO.c_str(), "INSTRUMENT");
  twrBay   = ifile.getInt(TEST_INFO.c_str(),    "TOWER_BAY"); 

  dacVals        = ifile.getIntVector(TEST_INFO.c_str(), "DAC_SETTINGS");
  nPulsesPerDAC  = ifile.getInt(TEST_INFO.c_str(), "N_PULSES_PER_DAC");

  // PATHS
  using facilities::Util;  
  outputDir = ifile.getString(PATHS.c_str(), "OUTPUT_FOLDER");
  Util::expandEnvVar(&outputDir);

  dtdPath       = ifile.getString(PATHS.c_str(), "DTD_FILE");
  Util::expandEnvVar(&dtdPath);
  dtdFilename   = dtdPath;
  path_remove_dir(dtdFilename);

  outputXMLPath = ifile.getString(PATHS.c_str(), "XMLPATH");
  Util::expandEnvVar(&outputXMLPath);
  outputTXTPath = ifile.getString(PATHS.c_str(), "TXTPATH");
  Util::expandEnvVar(&outputTXTPath);
  
  rootFileLE1 = ifile.getString(PATHS.c_str(), "ROOTFILE_LE1");
  Util::expandEnvVar(&rootFileLE1);
  rootFileHE1 = ifile.getString(PATHS.c_str(), "ROOTFILE_HE1");
  Util::expandEnvVar(&rootFileHE1);

  logfile = ifile.getString(PATHS.c_str(), "LOGFILE");
  Util::expandEnvVar(&logfile);

  // ridiculous comparison elminates cast warning in msvc
  genXML = ifile.getBool(GENERAL.c_str(), "GENERATE_XML") != 0;
  genTXT = ifile.getBool(GENERAL.c_str(), "GENERATE_TXT") != 0;
  genLogfile = ifile.getBool(GENERAL.c_str(), "GENERATE_LOGFILE") != 0;

  // SPLINE CFG
  splineGroupWidth = ifile.getIntVector(SPLINE_CFG.c_str(), "GROUP_WIDTH");
  splineSkipLow    = ifile.getIntVector(SPLINE_CFG.c_str(), "SKIP_LOW"  );
  splineSkipHigh   = ifile.getIntVector(SPLINE_CFG.c_str(), "SKIP_HIGH" );
  splineNPtsMin    = ifile.getIntVector(SPLINE_CFG.c_str(), "N_PTS_MIN" );
  
  // Geneate derived config quantities.
  nDACs          = dacVals.size();
  nPulsesPerXtal = nPulsesPerDAC * nDACs;
  nPulsesPerRun  = ColNum::N_VALS*nPulsesPerXtal;

  baseFilename = rootFileLE1;
  path_remove_dir(baseFilename);
  path_remove_ext(baseFilename);

  // Auto-generate output filenames

  string twrBayStr; // shared by all output filenames
  {
    ostringstream tmp;
    tmp << 'T' << setw(2) << setfill('0') << twrBay;
    twrBayStr = tmp.str();
  }
  if (outputXMLPath.length() == 0)
    outputXMLPath = outputDir + "ci_intnonlin." + baseFilename
      + '.' + twrBayStr + ".xml";
  if (outputTXTPath.length() == 0)
    outputTXTPath = outputDir + "ci_intnonlin." + baseFilename
      + '.' + twrBayStr + ".txt";
  if (logfile.length() == 0)
    logfile = outputDir + "ci_logfile." + baseFilename
      + '.' + twrBayStr + ".txt";

  // setup output stream
  // add cout by default
  ostrm.getostreams().push_back(&cout);
  // add user Req logfile
  if (genLogfile) {
    logStrm.open(logfile.c_str());
    ostrm.getostreams().push_back(&logStrm);
  }

  // create CVS_Tag string w/out illegal characters
  // xml does not allow '$' char which is used in CVS TAG replacment, also
  // xml counts spaces as delimeters & 'creator' is specified as a single value
  creator = CGCUtil::CVS_TAG;
  // this bizarre pattern is actually clearly described at this website
  // http://www.tempest-sw.com/cpp/draft/ch10-containers.html
  creator.erase(remove(creator.begin(), creator.end(), '$'),creator.end());
  replace(creator.begin(),creator.end(),' ','_');
}

void CfCfg::clear() {  
}

void CfCfg::summarize() {
}
