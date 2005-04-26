// LOCAL INCLUDES
#include "MtCfg.h"
#include "CalDefs.h"
#include "CGCUtil.h"

// GLAST INCLUDES
#include "facilities/Util.h"

// EXTLIB INCLUDES

// STD INCLUDES
#include <sstream>
#include <iomanip>

using namespace CGCUtil;

const string MtCfg::TEST_INFO("TEST_INFO");
const string MtCfg::PATHS("PATHS");
const string MtCfg::GENERAL("GENERAL");

void MtCfg::readCfgFile(const string& path) {
  clear();

  xmlBase::IFile ifile(path.c_str());
  
  // TEST INFO
  timestamp  = ifile.getString(TEST_INFO.c_str(), "TIMESTAMP");
  instrument = ifile.getString(TEST_INFO.c_str(), "INSTRUMENT");
  twrBay   = ifile.getInt(TEST_INFO.c_str(),    "TOWER_BAY"); 

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

  // ridiculous comparison elminates cast warning in msvc
  genXML      = ifile.getBool(GENERAL.c_str(), "GENERATE_XML") != 0;
  genTXT      = ifile.getBool(GENERAL.c_str(), "GENERATE_TXT") != 0;
  genLogfile  = ifile.getBool(GENERAL.c_str(), "GENERATE_LOGFILE") != 0;
  genHistfile = ifile.getBool(GENERAL.c_str(), "GENERATE_HISTFILE") != 0;

  // Geneate derived config quantities.
  nDACs          = dacVals.size();
  nPulsesPerXtal = nPulsesPerDAC * nDACs;
  nPulsesPerRun  = ColNum::N_VALS*nPulsesPerXtal;

  baseFilename = rootFileA;
  path_remove_dir(baseFilename);
  path_remove_ext(baseFilename);
  vector<string> tokens;
  string delim("_");
  tokenize_str(baseFilename,tokens,delim); 
  string moduleName = ((tokens[1]).size() == 5) ? tokens[1] : tokens[2];

  // Auto-generate output filenames
  string twrBayStr; //shared by all output filenames
  {
    ostringstream tmp;
    tmp << 'T' << setw(2) << setfill('0') << twrBay;
    twrBayStr = tmp.str();
  }
  if (outputXMLPath.length() == 0)
    outputXMLPath = outputDir + tokens[0]+ "_" + moduleName + "_" + "CAL_flefheBias"
    + '.' + twrBayStr + ".xml";
  if (outputTXTPath.length() == 0)
    outputTXTPath = outputDir + "muTrig." + baseFilename +'.' + twrBayStr + ".txt";
  if (logfile.length() == 0)
    logfile = outputDir + "muTrig_logfile." + baseFilename + '.' + twrBayStr + ".txt";
  if (histFile.length() == 0)
    histFile = outputDir + "muTrigEff." + baseFilename + '.' + twrBayStr + ".root";

  if (pedFileTXT.length() == 0)
    pedFileTXT = outputDir + "mc_peds." + baseFilename + '.' + twrBayStr + ".txt";

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
