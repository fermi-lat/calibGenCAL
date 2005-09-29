#ifndef CfCfg_H
#define CfCfg_H

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

/** @class CfCfg
    @author Zachary Fewtrell
    @brief Configuration settings for runCIFit application
*/
class CfCfg : ICfg {
 public:
  // basic ctor
  CfCfg() {};
  virtual ~CfCfg() {};

  // read in config data from config file, calculate dependent vars
  void readCfgFile(const string& path);

 public:  // i know, don't make members public, but it's just easier this way!
  // CONFIGURABLE PARAMETERS //

  // SECTION: TEST_INFO //
  /// time of measurement
  string timestamp;    
  /// instrument name "LAT", "EM", etc... 
  string instrument;   
  /// work on any single tower bay (0-15)
  int    twrBay;     

  /// sequence of test DAC values
  vector<int> dacVals;
  /// number of pules for each DAC value
  int nPulsesPerDAC;

  // SECTION: PATHS //
  /// folder for autonamed output files
  string outputDir; 
  /// override output XML path
  string outputXMLPath;   
  /// override output TXT path
  string outputTXTPath;
  /// override output histogram path LE diode
  string outputHistPathLE;
  /// override output histogram path HE diode
  string outputHistPathHE; 

  /// Data descriptoin file for .xml output
  string dtdPath; 
  /// filename only
  string dtdFilename; 

  /// LE input singlex16 digi event file (ROOT format)
  string rootFileLE1;
  /// HE input singlex16 digi event file (ROOT format)
  string rootFileHE1;

  /// duplicate of stdout log
  string logfile; 


  // SECTION: SPLINE CONFIG //
  vector<int>  splineEnableGrp;
  vector<int>  splineGroupWidth;
  vector<int>  splineSkipLow;
  vector<int>  splineSkipHigh;
  vector<int>  splineNPtsMin;

  // DERIVED FROM CONFIG PARMS //
  /// number of pulses per xtal
  int nPulsesPerXtal;
  /// number of pulses per singlex16 run
  int nPulsesPerRun;
  /// number of DAC values pulsed
  int nDACs;

  // SECTION: GENERAL //
  /// generate xml output
  bool genXML; 
  /// generate text output
  bool genTXT; 
  /// clone stdout stream to a logfile
  bool genLogfile; 
  /// generate optional histogram output
  bool genHistfile;

  /// multiplexing output stream will contain at least cout, but
  /// may also contain a logfile stream if the user requests it.
  CGCUtil::multiplexor_ostream ostrm;
  ofstream logStrm;

  /// CVS Tag string cleaned up for proper format in output XML files
  string creator;  

 private:
  /// used as the base for auto-generating output filenames.  derived from input root filename.
  string baseFilename; 

  // Section decription strings
  static const string TEST_INFO;
  static const string PATHS;
  static const string SPLINE_CFG;
  static const string GENERAL;
};


#endif // CfCfg_H
