// LOCAL INCLUDES
#include "MtCfg.h"
#include "CalDefs.h"
#include "CGCUtil.h"

// GLAST INCLUDES
#include "facilities/Util.h"

// EXTLIB INCLUDES

// STD INCLUDES

using namespace CGCUtil;

const string MtCfg::TEST_INFO("TEST_INFO");
const string MtCfg::PATHS("PATHS");
const string MtCfg::GENERAL("GENERAL");

void MtCfg::readCfgFile(const string& path) {
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
  
  rootFileA = ifile.getString(PATHS.c_str(), "ROOTFILE_A");
  Util::expandEnvVar(&rootFileA);
  rootFileB = ifile.getString(PATHS.c_str(), "ROOTFILE_B");
  Util::expandEnvVar(&rootFileB);
  rootFileCI = ifile.getString(PATHS.c_str(), "ROOTFILE_CI");
  Util::expandEnvVar(&rootFileCI);

  pedFileTXT      = ifile.getString(PATHS.c_str(), "PEDFILE_TXT");
  Util::expandEnvVar(&pedFileTXT);

  histFile = ifile.getString(PATHS.c_str(), "HISTFILE");
  Util::expandEnvVar(&histFile);

  logfile = ifile.getString(PATHS.c_str(), "LOGFILE");
  Util::expandEnvVar(&logfile);

  genXML = ifile.getBool(GENERAL.c_str(), "GENERATE_XML");
  genTXT = ifile.getBool(GENERAL.c_str(), "GENERATE_TXT");
  genLogfile = ifile.getBool(GENERAL.c_str(), "GENERATE_LOGFILE");
  genHistfile = ifile.getBool(GENERAL.c_str(), "GENERATE_HISTFILE");

  // Geneate derived config quantities.
  nDACs          = dacVals.size();
  nPulsesPerXtal = nPulsesPerDAC * nDACs;
  nPulsesPerRun  = ColNum::N_VALS*nPulsesPerXtal;

  baseFilename = rootFileA;
  path_remove_dir(baseFilename);
  path_remove_ext(baseFilename);

  // Auto-generate output filenames
  if (outputXMLPath.length() == 0)
    outputXMLPath = outputDir + "muTrig." + baseFilename + ".xml";
  if (outputTXTPath.length() == 0)
    outputTXTPath = outputDir + "muTrig." + baseFilename + ".txt";
  if (logfile.length() == 0)
    logfile = outputDir + "muTrig_logfile." + baseFilename + ".txt";
  if (histFile.length() == 0)
    histFile = outputDir + "muTrigEff." + baseFilename + ".root";

  if (pedFileTXT.length() == 0)
    pedFileTXT = outputDir + "mc_peds." + baseFilename + ".txt";

  // setup output stream
  // add cout by default
  ostr.getostreams().push_back(&cout);
  // add user Req logfile
  if (genLogfile) {
    logstr.open(logfile.c_str());
    ostr.getostreams().push_back(&logstr);
  }
}

void MtCfg::clear() {  
}

void MtCfg::summarize() {
}
