#ifndef teCfg_H
#define teCfg_H 1

// LOCAL INCLUDES
#include "ICfg.h"
#include "CalDefs.h"
#include "CGCUtil.h"

// GLAST INCLUDES
#include "xmlBase/IFile.h"

// EXTLIB INCLUDES

// STD INCLUDES
#include <string>
#include <fstream>

using namespace std;
using namespace CalDefs;

class TeCfg : ICfg {
 public:
  // basic ctor
  TeCfg() : valid(false) {};
  virtual ~TeCfg() {};

  // clear all values, delete all pointers
  void clear();

  // read in config data from config file, calculate dependent vars
  void readCfgFile(const string& path);

  // return data valid flag.
  bool isValid() {return valid;}

  // print summary to ostream
  void summarize();
 public:  // i know, don't make members public, but it's just easier this way!
  // CONFIGURABLE PARAMETERS //

  // SECTION: TEST_INFO //
  string timestamp;   ///< time of measurement
  string instrument;  ///< "LAT", "EM", etccc    
  int twrBay;       ///< work on single tower bay (0-15)

  int nTimePoints;
  int nEvtsPerPoint;

  // SECTION: PATHS //
  string outputDir; ///< folder for autonamed output files
  string outputXMLPath;   
  string outputTXTPath;

  string dtdFile; ///< Data descriptoin file for .xml output

  string rootFile;
  
  string logfile; ///< duplicate of stdout log
  string pedHistFile; ///< output ROOT histogram file
  string histFile; ///< output ROOT histogram file

  // SECTION: GENERAL //
  bool genXML; ///< generate xml output
  bool genTXT; ///< generate text output
  bool genLogfile; ///< clone stdout stream to a logfile
  bool genHistfile; ///< generate histogram root file 

  /// multiplexing output stream will contain at least cout, but
  /// may also contain a logfile stream if the user requests it.
  CGCUtil::multiplexor_ostream ostr;
  ofstream logstr;

 private:
  bool valid;   // set to false member data is incomplete/invalid.

  string baseFilename; ///< used as the base for auto-generating output filenames.  derived from input root filename.

  // Section decription strings
  static const string TEST_INFO;
  static const string PATHS;
  static const string GENERAL;
};


#endif // MtCfg_H
