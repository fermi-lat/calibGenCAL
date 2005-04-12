// LOCAL INCLUDES
#include "TeCfg.h"
#include "CalDefs.h"
#include "CGCUtil.h"

// GLAST INCLUDES
#include "facilities/Util.h"

// EXTLIB INCLUDES

// STD INCLUDES

using namespace CGCUtil;

const string TeCfg::TEST_INFO("TEST_INFO");
const string TeCfg::PATHS("PATHS");
const string TeCfg::GENERAL("GENERAL");

void TeCfg::readCfgFile(const string& path) {
  clear();

  xmlBase::IFile ifile(path.c_str());
  
  // TEST INFO
  timestamp = ifile.getString(TEST_INFO.c_str(), "TIMESTAMP");
  startTime = ifile.getString(TEST_INFO.c_str(), "STARTTIME");
  stopTime  = ifile.getString(TEST_INFO.c_str(), "STOPTIME");

  instrument    = ifile.getString(TEST_INFO.c_str(), "INSTRUMENT");
  towerList     = ifile.getIntVector(TEST_INFO.c_str(), "TOWER_LIST");

  triggerMode    = ifile.getString(TEST_INFO.c_str(), "TRIGGER_MODE");
  instrumentMode = ifile.getString(TEST_INFO.c_str(), "INST_MODE");
  source         = ifile.getString(TEST_INFO.c_str(), "TEST_SOURCE");

  nTimePoints  = ifile.getInt(TEST_INFO.c_str(), "N_TIME_POINTS");
  nEvtsPerPoint  = ifile.getInt(TEST_INFO.c_str(), "N_EVENTS_PER_POINT");


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
  
  rootFile = ifile.getString(PATHS.c_str(), "ROOTFILE");
  Util::expandEnvVar(&rootFile);

  histFile = ifile.getString(PATHS.c_str(), "HISTFILE");
  Util::expandEnvVar(&histFile);

  logfile = ifile.getString(PATHS.c_str(), "LOGFILE");
  Util::expandEnvVar(&logfile);

  // ridiculous comparison elminates cast warning in msvc
  genXML      = ifile.getBool(GENERAL.c_str(), "GENERATE_XML") != 0;
  genTXT      = ifile.getBool(GENERAL.c_str(), "GENERATE_TXT") != 0;
  genLogfile  = ifile.getBool(GENERAL.c_str(), "GENERATE_LOGFILE") != 0;
  genHistfile = ifile.getBool(GENERAL.c_str(), "GENERATE_HISTFILE") != 0;

  baseFilename = rootFile;
  path_remove_dir(baseFilename);
  path_remove_ext(baseFilename);

  // Auto-generate output filenames
  if (outputXMLPath.length() == 0)
    outputXMLPath = outputDir + "threshEvol." + baseFilename + ".xml";
  if (outputTXTPath.length() == 0)
    outputTXTPath = outputDir + "threshEvol." + baseFilename + ".txt";
  if (logfile.length() == 0)
    logfile = outputDir + "threshEvol_logfile." + baseFilename + ".txt";
  if (histFile.length() == 0)
    histFile = outputDir + "threshEvol." + baseFilename + ".root";

  // setup output stream
  // add cout by default
  ostr.getostreams().push_back(&cout);
  // add user Req logfile
  if (genLogfile) {
    logstr.open(logfile.c_str());
    ostr.getostreams().push_back(&logstr);
  }
}

void TeCfg::clear() {  
}

void TeCfg::summarize() {
}
