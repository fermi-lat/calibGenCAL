// LOCAL INCLUDES
#include "McCfg.h"
#include "CGCUtil.h"

// GLAST INCLUDES
#include "facilities/Util.h"

// EXTLIB INCLUDES

// STD INCLUDES
#include <algorithm>
#include <cctype>

const string McCfg::TEST_INFO("TEST_INFO");
const string McCfg::PATHS("PATHS");
const string McCfg::GENERAL("GENERAL");
const string McCfg::CONSTANTS("CONSTANTS");

using namespace CGCUtil;

void McCfg::readCfgFile(const string& cfgPath) {
  clear();

  xml::IFile ifile(cfgPath.c_str());
  
  // SECTION: TEST INFO
  timestamp = ifile.getString(TEST_INFO.c_str(), "TIMESTAMP");

  instrument    = ifile.getString(TEST_INFO.c_str(), "INSTRUMENT");
  towerList     = ifile.getIntVector(TEST_INFO.c_str(), "TOWER_LIST");

  // SECTION: PATHS
  using facilities::Util;
  rootFileListStr = ifile.getString(PATHS.c_str(), "INPUTFILE_LIST");
  Util::expandEnvVar(&rootFileListStr);
 
  intNonlinFile   = ifile.getString(PATHS.c_str(), "INTNONLINFILE_TXT");
  Util::expandEnvVar(&intNonlinFile);
  dtdFile         = ifile.getString(PATHS.c_str(), "DTD_FILE");
  Util::expandEnvVar(&dtdFile);

  outputDir       = ifile.getString(PATHS.c_str(), "OUTPUT_FOLDER");
  Util::expandEnvVar(&outputDir);

  pedFileXML      = ifile.getString(PATHS.c_str(), "PEDFILE_XML");
  Util::expandEnvVar(&pedFileXML);
  asymFileXML     = ifile.getString(PATHS.c_str(), "ASYMFILE_XML");
  Util::expandEnvVar(&asymFileXML);
  mpdFileXML      = ifile.getString(PATHS.c_str(), "MPDFILE_XML");
  Util::expandEnvVar(&mpdFileXML);

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

  maxAsymLL = ifile.getDouble(CONSTANTS.c_str(), "MAX_ASYM_LL");
  maxAsymLS = ifile.getDouble(CONSTANTS.c_str(), "MAX_ASYM_LS");
  maxAsymSL = ifile.getDouble(CONSTANTS.c_str(), "MAX_ASYM_SL");
  maxAsymSS = ifile.getDouble(CONSTANTS.c_str(), "MAX_ASYM_SS");
  minAsymLL = ifile.getDouble(CONSTANTS.c_str(), "MIN_ASYM_LL");
  minAsymLS = ifile.getDouble(CONSTANTS.c_str(), "MIN_ASYM_LS");
  minAsymSL = ifile.getDouble(CONSTANTS.c_str(), "MIN_ASYM_SL");
  minAsymSS = ifile.getDouble(CONSTANTS.c_str(), "MIN_ASYM_SS");
  
  // SECTION: GENERAL
  nEvtRoughPed = ifile.getInt(GENERAL.c_str(), "NEVENTS_ROUGHPED");
  nEvtPed  = ifile.getInt(GENERAL.c_str(), "NEVENTS_PED");
  nEvtAsym = ifile.getInt(GENERAL.c_str(), "NEVENTS_ASYM");
  nEvtMPD  = ifile.getInt(GENERAL.c_str(), "NEVENTS_MPD");

  // ridiculous comparison elminates cast warning in msvc
  readInPeds = ifile.getBool(GENERAL.c_str(), "READ_IN_PEDS") != 0;
  readInAsym = ifile.getBool(GENERAL.c_str(), "READ_IN_ASYM") != 0;
  skipMPD    = ifile.getBool(GENERAL.c_str(), "SKIP_MPD")     != 0;

  // ridiculous comparison elminates cast warning in msvc
  genXML = ifile.getBool(GENERAL.c_str(), "GENERATE_XML") != 0;
  genTXT = ifile.getBool(GENERAL.c_str(), "GENERATE_TXT") != 0;
  genHistfiles = ifile.getBool(GENERAL.c_str(), "GENERATE_HISTOGRAM_FILES") != 0;
  genLogfile   = ifile.getBool(GENERAL.c_str(), "GENERATE_LOGFILE")         != 0;

  // only asign genOptAsymHists is genHistfiles is also enabled.
  genOptAsymHists = false;
  if (genHistfiles) genOptAsymHists 
     = ifile.getBool(GENERAL.c_str(), "GEN_OPT_ASYM_HISTS") != 0;

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
  if (pedFileXML.length() == 0)
    pedFileXML = outputDir + "mc_peds." + baseFilename + ".xml";

  if (asymFileXML.length() == 0)
    asymFileXML = outputDir + "mc_asym." + baseFilename + ".xml";

  if (mpdFileXML.length() == 0)
    mpdFileXML = outputDir + "mc_mevperdac." + baseFilename + ".xml";

  
  if (pedHistFile.length() == 0)
    pedHistFile = outputDir + "mc_peds." + baseFilename + ".root";
  
  if (asymHistFile.length() == 0)
    asymHistFile = outputDir + "mc_asym." + baseFilename + ".root";
            
  if (mpdHistFile.length() == 0)
    mpdHistFile = outputDir + "mc_mevperdac." + baseFilename + ".root";


  if (pedFileTXT.length() == 0)
    pedFileTXT = outputDir + "mc_peds." + baseFilename + ".txt";
                
  if (asymFileLLTXT.length() == 0)
    asymFileLLTXT = outputDir + "mc_asymLL." + baseFilename + ".txt";
                  
  if (asymFileLSTXT.length() == 0)
    asymFileLSTXT = outputDir + "mc_asymLS." + baseFilename + ".txt";
                    
  if (asymFileSLTXT.length() == 0)
    asymFileSLTXT = outputDir + "mc_asymSL." + baseFilename + ".txt";
                      
  if (asymFileSSTXT.length() == 0)
    asymFileSSTXT = outputDir + "mc_asymSS." + baseFilename + ".txt";
                        
  if (largeMPDFileTXT.length() == 0)
    largeMPDFileTXT = outputDir + "mc_mpdLarge." + baseFilename + ".txt";
                            
  if (smallMPDFileTXT.length() == 0)
    smallMPDFileTXT = outputDir + "mc_mpdSmall." + baseFilename + ".txt";

  if (logfile.length() == 0)
    logfile = outputDir + "mc_logfile." + baseFilename + ".txt";

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
  // NO idea why this works, but it does.
  // Newsgroups: comp.lang.c++
  // From: "John Harrison" <jah...@dtn.ntl.com> - Find messages by this author
  // Date: 2000/07/27
  // Subject: Re: remove characters from a string*/
  creator.erase(remove_if(creator.begin(), 
                          creator.end(), 
                          ispunct), 
                creator.end());
  replace(creator.begin(),creator.end(),' ','_');
}

void McCfg::clear() {
}

void McCfg::summarize() {
}
