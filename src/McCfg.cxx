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
  using facilities::Util;
 
  // SECTION: TEST INFO
  timestamp        = ifile.getString(TEST_INFO.c_str(), "TIMESTAMP");
  instrument       = ifile.getString(TEST_INFO.c_str(), "INSTRUMENT");
  Util::expandEnvVar(&timestamp);
  Util::expandEnvVar(&instrument);

  string tmpStr = ifile.getString(TEST_INFO.c_str(), "TOWER_BAY");
  Util::expandEnvVar(&tmpStr);
  twrBay = Util::stringToInt(tmpStr);

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
  asymFileTXT     = ifile.getString(PATHS.c_str(), "ASYMFILE_TXT");
  Util::expandEnvVar(&asymFileTXT);
  mpdFileTXT      = ifile.getString(PATHS.c_str(), "MPDFILE_TXT");
  Util::expandEnvVar(&mpdFileTXT);

  logfile = ifile.getString(PATHS.c_str(), "LOGFILE");
  Util::expandEnvVar(&logfile);

  //  SECTION: CONSTANTS //
  hitThresh = ifile.getDouble(CONSTANTS.c_str(), "HIT_THRESH");
  
  cellHorPitch  = ifile.getDouble(CONSTANTS.c_str(), "CELL_HOR_PITCH");
  cellVertPitch = ifile.getDouble(CONSTANTS.c_str(), "CELL_VERT_PITCH");
  csiLength     = ifile.getDouble(CONSTANTS.c_str(), "CSI_LENGTH");

  // SECTION: GENERAL
  tmpStr = ifile.getString(GENERAL.c_str(), "NEVENTS_ROUGHPED");
  Util::expandEnvVar(&tmpStr);
  nEvtRoughPed = Util::stringToInt(tmpStr);
  tmpStr = ifile.getString(GENERAL.c_str(), "NEVENTS_PED");
  Util::expandEnvVar(&tmpStr);
  nEvtPed  = Util::stringToInt(tmpStr);
  tmpStr = ifile.getString(GENERAL.c_str(), "NEVENTS_ASYM");
  Util::expandEnvVar(&tmpStr);
  nEvtAsym = Util::stringToInt(tmpStr);
  tmpStr = ifile.getString(GENERAL.c_str(), "NEVENTS_MPD");
  Util::expandEnvVar(&tmpStr);
  nEvtMPD  = Util::stringToInt(tmpStr);

  // ridiculous comparison elminates cast warning in msvc
  // MAY BE SET BY ENVIRONMENT VARIABLES
  tmpStr = ifile.getString(GENERAL.c_str(), "READ_IN_PEDS");
  Util::expandEnvVar(&tmpStr);
  readInPeds = stringToBool(tmpStr);
  inputPedFile = ifile.getString(GENERAL.c_str(), "INPUT_PEDFILE");
  Util::expandEnvVar(&inputPedFile);
  tmpStr = ifile.getString(GENERAL.c_str(), "PEDS_ONLY")   ;
  Util::expandEnvVar(&tmpStr);
  pedsOnly   = stringToBool(tmpStr);
  Util::expandEnvVar(&tmpStr);
  tmpStr = ifile.getString(GENERAL.c_str(), "READ_IN_ASYM");
  Util::expandEnvVar(&tmpStr);
  readInAsym = stringToBool(tmpStr);
  tmpStr = ifile.getString(GENERAL.c_str(), "SKIP_MPD")    ;
  Util::expandEnvVar(&tmpStr);
  skipMPD    = stringToBool(tmpStr);

  // ridiculous comparison elminates cast warning in msvc
  tmpStr = ifile.getString(GENERAL.c_str(), "GENERATE_XML");
  Util::expandEnvVar(&tmpStr);
  genXML = stringToBool(tmpStr);
  tmpStr = ifile.getString(GENERAL.c_str(), "GENERATE_TXT");
  Util::expandEnvVar(&tmpStr);
  genTXT = stringToBool(tmpStr);
  tmpStr = ifile.getString(GENERAL.c_str(), "GENERATE_HISTOGRAM_FILES");
  Util::expandEnvVar(&tmpStr);
  genHistfiles = stringToBool(tmpStr);
  tmpStr = ifile.getString(GENERAL.c_str(), "GENERATE_LOGFILE")        ;
  Util::expandEnvVar(&tmpStr);
  genLogfile   = stringToBool(tmpStr);

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
    mpdFileXML = outputDir + "mc_mpd." + baseFilename
      + '.' + twrBayStr + ".xml";
  if (pedHistFile.length() == 0)
    pedHistFile = outputDir + "mc_peds." + baseFilename
      + '.' + twrBayStr + ".root";
  if (asymHistFile.length() == 0)
    asymHistFile = outputDir + "mc_asym." + baseFilename
      + '.' + twrBayStr + ".root";
  if (mpdHistFile.length() == 0)
    mpdHistFile = outputDir + "mc_mpd." + baseFilename
      + '.' + twrBayStr + ".root";
  if (pedFileTXT.length() == 0)
    pedFileTXT = outputDir + "mc_peds." + baseFilename 
      + '.' + twrBayStr + ".txt";
  if (asymFileTXT.length() == 0)
    asymFileTXT = outputDir + "mc_asym." + baseFilename 
      + '.' + twrBayStr + ".txt";
  if (mpdFileTXT.length() == 0)
    mpdFileTXT = outputDir + "mc_mpd." + baseFilename 
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
