#include "mcOptions.h"

////////////////////////// GENERAL UTILITIES //////////////////////////////////////////

/** \brief splits a delmiited string into a vector of shorter token-strings
    stolen from http://www.linuxselfhelp.com/HOWTO/C++Programming-HOWTO-7.html
*/
void tokenize_str(const string& str,
                  vector<string>& tokens,
                  const string& delimiters = " ")
{
  // Skip delimiters at beginning.
  string::size_type lastPos = str.find_first_not_of(delimiters, 0);
  // Find first "non-delimiter".
  string::size_type pos     = str.find_first_of(delimiters, lastPos);

  while (string::npos != pos || string::npos != lastPos)
    {
      // Found a token, add it to the vector.
      tokens.push_back(str.substr(lastPos, pos - lastPos));
      // Skip delimiters.  Note the "not_of"
      lastPos = str.find_first_not_of(delimiters, pos);
      // Find next "non-delimiter"
      pos = str.find_first_of(delimiters, lastPos);
    }
}

/// finds position of last directory delimeter ('/' || '\\')
/// in path, returns -1 if no delim is found
int path_find_last_delim(string &path) {
  // find last directory delimeter.
  int fwdslash_pos = path.find_last_of('/');
  int bckslash_pos = path.find_last_of('\\');

  // check for 'not found' case
  if (fwdslash_pos == path.npos) fwdslash_pos = -1;
  if (bckslash_pos == path.npos) bckslash_pos = -1;

  return max(fwdslash_pos,bckslash_pos);

}

string &path_remove_dir(string &path) {
  int slash_pos;
  
  // if there was no delimeter, return path unaltered
  if ((slash_pos = path_find_last_delim(path)) == path.npos) 
    return path;

  // else remove everything up to & including the delimeter
  path.erase(0,slash_pos+1);

  return path;
}

/// removes filename extension from end of path string.
string &path_remove_ext(string &path) {
  // return path unaltered if there is no '.'
  int dot_pos;
  if ((dot_pos = path.find_last_of('.')) == path.npos)
    return path;

  // find last delim (extension must be after this point)
  int slash_pos = path_find_last_delim(path);

  // if ',' is before slash then there is
  // no extension in the filename itself
  if (slash_pos > dot_pos) return path;

  // erase everything from '.' onward.
  path.erase(dot_pos, path.size());

  return path;
}

///////////////////////////////////////////////////////////////////////////////////

const string mcCfg::TEST_INFO("TEST_INFO");
const string mcCfg::PATHS("PATHS");
const string mcCfg::GENERAL("GENERAL");
const string mcCfg::CONSTANTS("CONSTANTS");

int mcCfg::readCfgFile(const string& cfgPath) {
  clear();

  xml::IFile ifile(cfgPath.c_str());
  
  // SECTION: TEST INFO
  timestamp = ifile.getString(TEST_INFO.c_str(), "TIMESTAMP");

  instrument    = ifile.getString(TEST_INFO.c_str(), "INSTRUMENT");
  towerList     = ifile.getIntVector(TEST_INFO.c_str(), "TOWER_LIST");

  // SECTION: PATHS
  rootFileListStr = ifile.getString(PATHS.c_str(), "INPUTFILE_LIST");
 
  intNonlinFile = ifile.getString(PATHS.c_str(), "INTNONLINFILE_TXT");
  dtdFile = ifile.getString(PATHS.c_str(), "DTD_FILE");

  outputDir = ifile.getString(PATHS.c_str(), "OUTPUT_FOLDER");

  pedFileXML = ifile.getString(PATHS.c_str(), "PEDFILE_XML");
  asymFileXML = ifile.getString(PATHS.c_str(), "ASYMFILE_XML");
  mpdFileXML = ifile.getString(PATHS.c_str(), "MPDFILE_XML");

  pedHistFile = ifile.getString(PATHS.c_str(), "PED_HISTFILE");
  asymHistFile = ifile.getString(PATHS.c_str(), "ASYM_HISTFILE");
  mpdHistFile = ifile.getString(PATHS.c_str(), "MPD_HISTFILE");
 
  pedFileTXT = ifile.getString(PATHS.c_str(), "PEDFILE_TXT");
  asymFileLLTXT = ifile.getString(PATHS.c_str(), "ASYMFILELL_TXT");
  asymFileLSTXT = ifile.getString(PATHS.c_str(), "ASYMFILELS_TXT");
  asymFileSLTXT = ifile.getString(PATHS.c_str(), "ASYMFILESL_TXT");
  asymFileSSTXT = ifile.getString(PATHS.c_str(), "ASYMFILESS_TXT");
  largeMPDFileTXT = ifile.getString(PATHS.c_str(), "LARGEMPD_TXT");
  smallMPDFileTXT = ifile.getString(PATHS.c_str(), "SMALLMPD_TXT");

  logfile = ifile.getString(PATHS.c_str(), "LOGFILE");

  //  SECTION: CONSTANTS //
  hitThresh = ifile.getDouble(CONSTANTS.c_str(), "HIT_THRESH");
  
  cellHorPitch = ifile.getDouble(CONSTANTS.c_str(), "CELL_HOR_PITCH");
  cellVertPitch = ifile.getDouble(CONSTANTS.c_str(), "CELL_VERT_PITCH");

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
  nEvtPed = ifile.getInt(GENERAL.c_str(), "NEVENTS_PED");
  nEvtAsym = ifile.getInt(GENERAL.c_str(), "NEVENTS_ASYM");
  nEvtMPD = ifile.getInt(GENERAL.c_str(), "NEVENTS_MPD");

  readInPeds = ifile.getBool(GENERAL.c_str(), "READ_IN_PEDS");
  readInAsym = ifile.getBool(GENERAL.c_str(), "READ_IN_ASYM");
  skipMPD    = ifile.getBool(GENERAL.c_str(), "SKIP_MPD");

  genXML = ifile.getBool(GENERAL.c_str(), "GENERATE_XML");
  genTXT = ifile.getBool(GENERAL.c_str(), "GENERATE_TXT");
  genHistfiles = ifile.getBool(GENERAL.c_str(), "GENERATE_HISTOGRAM_FILES");
  genLogfile = ifile.getBool(GENERAL.c_str(), "GENERATE_LOGFILE");

  // only asign genOptAsymHists is genHistfiles is also enabled.
  genOptAsymHists = false;
  if (genHistfiles) genOptAsymHists = ifile.getBool(GENERAL.c_str(), "GEN_OPT_ASYM_HISTS");

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
  ostr.getostreams().push_back(&cout);
  // add user requested logfile
  if (genLogfile) {
    logstr.open(logfile.c_str());
    ostr.getostreams().push_back(&logstr);
  }

  return 0;
}

void mcCfg::clear() {
}

void mcCfg::summarize(ostream &ostr) {
}
