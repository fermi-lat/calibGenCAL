#include "cfCfg.h"
#include "CalDefs.h"
#include "CGCUtil.h"

using namespace CGCUtil;

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
