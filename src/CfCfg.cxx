// LOCAL INCLUDES
#include "CfCfg.h"
#include "CalDefs.h"
#include "CGCUtil.h"

// GLAST INCLUDES
#include "facilities/Util.h"

// EXTLIB INCLUDES

// STD INCLUDES

using namespace CGCUtil;

const string CfCfg::TEST_INFO("TEST_INFO");
const string CfCfg::PATHS("PATHS");
const string CfCfg::SPLINE_CFG("SPLINE_CFG");
const string CfCfg::GENERAL("GENERAL");

void CfCfg::readCfgFile(const string& path) {
  clear();

  xml::IFile ifile(path.c_str());
  
  // TEST INFO
  timestamp = ifile.getString(TEST_INFO.c_str(), "TIMESTAMP");
  startTime = ifile.getString(TEST_INFO.c_str(), "STARTTIME");
  stopTime  = ifile.getString(TEST_INFO.c_str(), "STOPTIME");

  instrument    = ifile.getString(TEST_INFO.c_str(), "INSTRUMENT");
  towerList     = ifile.getIntVector(TEST_INFO.c_str(), "TOWER_LIST");

  triggerMode    = ifile.getString(TEST_INFO.c_str(), "TRIGGER_MODE");
  instrumentMode = ifile.getString(TEST_INFO.c_str(), "INST_MODE");
  source         = ifile.getString(TEST_INFO.c_str(), "TEST_SOURCE");
  
  dacVals    = ifile.getIntVector(TEST_INFO.c_str(), "DAC_SETTINGS");
  nPulsesPerDAC  = ifile.getInt(TEST_INFO.c_str(), "N_PULSES_PER_DAC");

  // PATHS
  using facilities::Util;  
  outputDir = ifile.getString(PATHS.c_str(), "OUTPUT_FOLDER");
  Util::expandEnvVar(&outputDir);

  dtdFile   = ifile.getString(PATHS.c_str(), "DTD_FILE");
  Util::expandEnvVar(&dtdFile);
  outputXMLPath = ifile.getString(PATHS.c_str(), "XMLPATH");
  Util::expandEnvVar(&outputXMLPath);
  outputTXTPath = ifile.getString(PATHS.c_str(), "TXTPATH");
  Util::expandEnvVar(&outputTXTPath);
  
  rootFileLE1 = ifile.getString(PATHS.c_str(), "ROOTFILE_LE1");
  Util::expandEnvVar(&rootFileLE1);
  rootFileHE1 = ifile.getString(PATHS.c_str(), "ROOTFILE_HE1");
  Util::expandEnvVar(&rootFileHE1);
  rootFileLE2 = ifile.getString(PATHS.c_str(), "ROOTFILE_LE2");
  Util::expandEnvVar(&rootFileLE2);
  rootFileHE2 = ifile.getString(PATHS.c_str(), "ROOTFILE_HE2");
  Util::expandEnvVar(&rootFileHE2);
  rootFileLE3 = ifile.getString(PATHS.c_str(), "ROOTFILE_LE3");
  Util::expandEnvVar(&rootFileLE3);
  rootFileHE3 = ifile.getString(PATHS.c_str(), "ROOTFILE_HE3");
  Util::expandEnvVar(&rootFileHE3);

  logfile = ifile.getString(PATHS.c_str(), "LOGFILE");
  Util::expandEnvVar(&logfile);

  genXML = ifile.getBool(GENERAL.c_str(), "GENERATE_XML");
  genTXT = ifile.getBool(GENERAL.c_str(), "GENERATE_TXT");
  genLogfile = ifile.getBool(GENERAL.c_str(), "GENERATE_LOGFILE");

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
  if (outputXMLPath.length() == 0)
    outputXMLPath = outputDir + "ci_intnonlin." + baseFilename + ".xml";
  if (outputTXTPath.length() == 0)
    outputTXTPath = outputDir + "ci_intnonlin." + baseFilename + ".txt";
  if (logfile.length() == 0)
    logfile = outputDir + "ci_logfile." + baseFilename + ".txt";

  // setup output stream
  // add cout by default
  ostr.getostreams().push_back(&cout);
  // add user Req logfile
  if (genLogfile) {
    logstr.open(logfile.c_str());
    ostr.getostreams().push_back(&logstr);
  }
}

void CfCfg::clear() {  
}

void CfCfg::summarize() {
}
