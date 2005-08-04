// LOCAL INCLUDES
#include "McCfg.h"
#include "CGCUtil.h"

// GLAST INCLUDES
#include "facilities/Util.h"

// EXTLIB INCLUDES

// STD INCLUDES
#include <algorithm>
#include <cctype>
#include <functional>
#include <sstream>
#include <iomanip>

const string McCfg::TEST_INFO("TEST_INFO");
const string McCfg::PATHS("PATHS");
const string McCfg::GENERAL("GENERAL");
const string McCfg::CONSTANTS("CONSTANTS");

using namespace CGCUtil;
using namespace std;

void McCfg::readCfgFile(const string& cfgPath) {
  clear();

  xmlBase::IFile ifile(cfgPath.c_str());
  
  
  // SECTION: TEST INFO
  timestamp        = ifile.getString(TEST_INFO.c_str(), "TIMESTAMP");
  instrument       = ifile.getString(TEST_INFO.c_str(), "INSTRUMENT");
  // TOWER BAY MAY BE SET BY AN ENVIRONMENT VARYABLE
  using facilities::Util;
  string tmpStr = ifile.getString(TEST_INFO.c_str(), "TOWER_BAY");
  Util::expandEnvVar(&tmpStr);
  istringstream twrBayStrm(tmpStr);
  twrBayStrm >> twrBay;

  // SECTION: PATHS
  rootFileListStr = ifile.getString(PATHS.c_str(), "INPUTFILE_LIST");
  Util::expandEnvVar(&rootFileListStr);
 
  intNonlinFile   = ifile.getString(PATHS.c_str(), "INTNONLINFILE_TXT");
  Util::expandEnvVar(&intNonlinFile);
  dtdPath         = ifile.getString(PATHS.c_str(), "DTD_FILE");
  Util::expandEnvVar(&dtdPath);

  outputDir       = ifile.getString(PATHS.c_str(), "OUTPUT_FOLDER");
  Util::expandEnvVar(&outputDir);

  pedFileXML      = ifile.getString(PATHS.c_str(), "PEDFILE_XML");
  Util::expandEnvVar(&pedFileXML);
  asymFileXML     = ifile.getString(PATHS.c_str(), "ASYMFILE_XML");
  Util::expandEnvVar(&asymFileXML);
  mpdFileXML      = ifile.getString(PATHS.c_str(), "MPDFILE_XML");
  Util::expandEnvVar(&mpdFileXML);
  adc2nrgFileXML      = ifile.getString(PATHS.c_str(), "ADC2NRGFILE_XML");
  Util::expandEnvVar(&adc2nrgFileXML);

  pedHistFile     = ifile.getString(PATHS.c_str(), "PED_HISTFILE");
  Util::expandEnvVar(&pedHistFile);
  asymHistFile    = ifile.getString(PATHS.c_str(), "ASYM_HISTFILE");
  Util::expandEnvVar(&asymHistFile);
  mpdHistFile     = ifile.getString(PATHS.c_str(), "MPD_HISTFILE");
  Util::expandEnvVar(&mpdHistFile);
 
  pedFileTXT      = ifile.getString(PATHS.c_str(), "PEDFILE_TXT");
  Util::expandEnvVar(&pedFileTXT);
  asymFileLLTXT   = ifile.getString(PATHS.c_str(), "ASYMFILELL_TXT");
  Util::expandEnvVar(&asymFileLLTXT);
  asymFileLSTXT   = ifile.getString(PATHS.c_str(), "ASYMFILELS_TXT");
  Util::expandEnvVar(&asymFileLSTXT);
  asymFileSLTXT   = ifile.getString(PATHS.c_str(), "ASYMFILESL_TXT");
  Util::expandEnvVar(&asymFileSLTXT);
  asymFileSSTXT   = ifile.getString(PATHS.c_str(), "ASYMFILESS_TXT");
  Util::expandEnvVar(&asymFileSSTXT);
  largeMPDFileTXT = ifile.getString(PATHS.c_str(), "LARGEMPD_TXT");
  Util::expandEnvVar(&largeMPDFileTXT);
  smallMPDFileTXT = ifile.getString(PATHS.c_str(), "SMALLMPD_TXT");
  Util::expandEnvVar(&smallMPDFileTXT);

  logfile = ifile.getString(PATHS.c_str(), "LOGFILE");
  Util::expandEnvVar(&logfile);

  //  SECTION: CONSTANTS //
  hitThresh = ifile.getDouble(CONSTANTS.c_str(), "HIT_THRESH");
  
  cellHorPitch  = ifile.getDouble(CONSTANTS.c_str(), "CELL_HOR_PITCH");
  cellVertPitch = ifile.getDouble(CONSTANTS.c_str(), "CELL_VERT_PITCH");
  csiLength     = ifile.getDouble(CONSTANTS.c_str(), "CSI_LENGTH");

  // SECTION: GENERAL
  nEvtRoughPed = ifile.getInt(GENERAL.c_str(), "NEVENTS_ROUGHPED");
  nEvtPed  = ifile.getInt(GENERAL.c_str(), "NEVENTS_PED");
  nEvtAsym = ifile.getInt(GENERAL.c_str(), "NEVENTS_ASYM");
  nEvtMPD  = ifile.getInt(GENERAL.c_str(), "NEVENTS_MPD");

  // ridiculous comparison elminates cast warning in msvc
  readInPeds = ifile.getBool(GENERAL.c_str(), "READ_IN_PEDS") != 0;
  pedsOnly   = ifile.getBool(GENERAL.c_str(), "PEDS_ONLY")    != 0;
  readInAsym = ifile.getBool(GENERAL.c_str(), "READ_IN_ASYM") != 0;
  skipMPD    = ifile.getBool(GENERAL.c_str(), "SKIP_MPD")     != 0;

  // ridiculous comparison elminates cast warning in msvc
  genXML = ifile.getBool(GENERAL.c_str(), "GENERATE_XML") != 0;
  genTXT = ifile.getBool(GENERAL.c_str(), "GENERATE_TXT") != 0;
  genHistfiles = ifile.getBool(GENERAL.c_str(), "GENERATE_HISTOGRAM_FILES")!=0;
  genLogfile   = ifile.getBool(GENERAL.c_str(), "GENERATE_LOGFILE")        !=0;
  verbose      = ifile.getBool(GENERAL.c_str(), "VERBOSE") != 0;

  // only asign genOptAsymHists is genHistfiles is also enabled.
  genOptAsymHists = false;
  if (genHistfiles) genOptAsymHists 
                      = ifile.getBool(GENERAL.c_str(), "GEN_OPT_ASYM_HISTS")!=0;

  // *** POST PROCESS *** //
  // parse list of ROOT input files
  tokenize_str(rootFileListStr, rootFileList);

  // extract basename from 1st ROOT input file
  baseFilename = rootFileList[0];
  path_remove_dir(baseFilename);
  path_remove_ext(baseFilename);

  // make sure that outputDir has slash on end
  outputDir += '/';

  // autogenerate output filenames (only autogen if string is empty)

  string twrBayStr; //shared by all output filenames
  {
    ostringstream tmp;
    tmp << 'T' << setw(2) << setfill('0') << twrBay;
    twrBayStr = tmp.str();
  }
  if (pedFileXML.length() == 0)
    pedFileXML = outputDir + "mc_peds." + baseFilename 
      + '.' + twrBayStr + ".xml";
  if (asymFileXML.length() == 0)
    asymFileXML = outputDir + "mc_asym." + baseFilename
      + '.' + twrBayStr + ".xml";
  if (mpdFileXML.length() == 0)
    mpdFileXML = outputDir + "mc_mevperdac." + baseFilename
      + '.' + twrBayStr + ".xml";
  if (pedHistFile.length() == 0)
    pedHistFile = outputDir + "mc_peds." + baseFilename
      + '.' + twrBayStr + ".root";
  if (asymHistFile.length() == 0)
    asymHistFile = outputDir + "mc_asym." + baseFilename
      + '.' + twrBayStr + ".root";
  if (mpdHistFile.length() == 0)
    mpdHistFile = outputDir + "mc_mevperdac." + baseFilename
      + '.' + twrBayStr + ".root";
  if (pedFileTXT.length() == 0)
    pedFileTXT = outputDir + "mc_peds." + baseFilename 
      + '.' + twrBayStr + ".txt";
  if (asymFileLLTXT.length() == 0)
    asymFileLLTXT = outputDir + "mc_asymLL." + baseFilename 
      + '.' + twrBayStr + ".txt";
  if (asymFileLSTXT.length() == 0)
    asymFileLSTXT = outputDir + "mc_asymLS." + baseFilename 
      + '.' + twrBayStr + ".txt";
  if (asymFileSLTXT.length() == 0)
    asymFileSLTXT = outputDir + "mc_asymSL." + baseFilename 
      + '.' + twrBayStr + ".txt";
  if (asymFileSSTXT.length() == 0)
    asymFileSSTXT = outputDir + "mc_asymSS." + baseFilename 
      + '.' + twrBayStr + ".txt";
  if (largeMPDFileTXT.length() == 0)
    largeMPDFileTXT = outputDir + "mc_mpdLarge." + baseFilename 
      + '.' + twrBayStr + ".txt";
  if (smallMPDFileTXT.length() == 0)
    smallMPDFileTXT = outputDir + "mc_mpdSmall." + baseFilename 
      + '.' + twrBayStr + ".txt";
  if (logfile.length() == 0)
    logfile = outputDir + "mc_logfile." + baseFilename 
      + '.' + twrBayStr + ".txt";

  // generate adc2nrg output filename
  if (adc2nrgFileXML.length() == 0){
    vector<string> baseFilenameElem;
	string moduleName = "FMxxx";

    // expected basefilename format has '_' delimited fields.
    tokenize_str(baseFilename,baseFilenameElem,"_");
	string runID = baseFilenameElem[0];

    if (baseFilenameElem.size() < 2) // unexpected filename format.
      adc2nrgFileXML = outputDir  + runID
        + "_" + twrBayStr + "_" + moduleName + "_cal_adc2nrg"+ ".xml";
	else {   // default filename format
		if ((baseFilenameElem[1]).size() == 5 && ((baseFilenameElem[1]).substr(0,2))== string("FM")) moduleName=baseFilenameElem[1];
		if ((baseFilenameElem[2]).size() == 5 && ((baseFilenameElem[2]).substr(0,2))== string("FM")) moduleName=baseFilenameElem[2];

		if (runID.substr(0,12) == string("digitization")) runID = baseFilenameElem[1];


		adc2nrgFileXML = outputDir + runID+"_"+ twrBayStr + "_"  + moduleName + "_cal_adc2nrg" + ".xml";
	}
  }
  

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

void McCfg::clear() {
}

void McCfg::summarize() {
}
