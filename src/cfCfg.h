#ifndef cfCfg_H
#define cfCfg_H 1

#include <string>
#include <fstream>

#include "xml/IFile.h"

#include "ICfg.h"
#include "CalDefs.h"
#include "CGCUtil.h"

using namespace std;

class cfCfg : ICfg, protected CalDefs {
public:
  // basic ctor
  cfCfg() : valid(false) {};
  virtual ~cfCfg() {};

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
  string timestamp;
  string startTime;
  string stopTime; 

  string instrument;    
  vector<int> towerList;

  string triggerMode;   
  string instrumentMode;
  string source;

  vector<int> dacSettings;
  int nPulsesPerDAC;

  // SECTION: PATHS //
  // current setup has us reading in 6 input files 2 rngs each and 3 different FLE settings
  string outputDir; ///< folder for autonamed output files
  string outputXMLPath;   
  string outputTXTPath;

  string dtdFile; ///< Data descriptoin file for .xml output

  string rootFileLE1;
  string rootFileHE1;
  string rootFileLE2;
  string rootFileHE2;
  string rootFileLE3;
  string rootFileHE3;
  
  string logfile; ///< duplicate of stdout log


  // SECTION: SPLINE CONFIG //
  vector<int> splineGroupWidth;
  vector<int> splineSkipLow;
  vector<int> splineSkipHigh;
  vector<int> splineNPtsMin;

  // DERIVED FROM CONFIG PARMS //
  int nPulsesPerXtal;
  int nPulsesPerRun;
  int nDACs;

  // SECTION: GENERAL //
  bool genXML; ///< generate xml output
  bool genTXT; ///< generate text output
  bool genLogfile; ///< clone stdout stream to a logfile

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
  static const string SPLINE_CFG;
  static const string GENERAL;
};


#endif // cfCfg_H
