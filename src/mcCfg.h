#ifndef mcCfg_H
#define mcCfg_H 1

#include <fstream>

#include "xml/IFile.h"
#include "ICfg.h"

using namespace std;

/////////////////////////////////// GENERAL UTILITIES /////////////////////////////////

typedef vector<ostream*> streamvector;

/** wrapper class for treating multiple streambuf objects as one 
 *
 * used by multiplexor_ostream
 */
class multiplexor_streambuf : public streambuf
{
public:
  multiplexor_streambuf() : streambuf() {}
  
  virtual int overflow(int c)
  {
    // write the incoming character into each stream
    streamvector::iterator _b = _streams.begin(), _e = _streams.end();
    for(; _b != _e; _b++)
      (*_b)->put(c);
    
    return c;
  }
  
public:
  streamvector _streams;
};

/** ostream class will write to any number of streambuffers simultaneously
 *
 * taken from here: 
 * http://www.gamedev.net/community/forums/topic.asp?topic_id=286078
 *
 * CoffeeMug  GDNet+  Member since: 3/25/2003  From: NY, USA
 * Posted - 12/1/2004 9:41:18 PM
 *
 *
 */
class multiplexor_ostream : public ostream
{
public:
  multiplexor_ostream() : ostream(new multiplexor_streambuf()), ios(0) {}
  virtual ~multiplexor_ostream() { delete rdbuf(); }
  
  streamvector& getostreams() { return ((multiplexor_streambuf*)rdbuf())->_streams; }
};

///////////////////////////////////////////////////////////////////////////////////////////////

class mcCfg : ICfg {
public:
  /// basic ctor
  mcCfg() {valid = false;}
  virtual ~mcCfg() {};

  /// clear all values, delete all pointers
  void clear();

  /// read in config data from config file, calculate dependent vars
  int readCfgFile(const string& cfgPath);

  /// return data valid flag.
  bool isValid() {return valid;}

  /// print summary to ostream
  void summarize();
public:  // i know I'm not supposed to make data members public, but it's just easier this way!
  // CONFIGURABLE PARAMETERS //

  // SECTION: TEST_INFO //
  string timestamp;     ///< time of observation/measurement

  string instrument;    ///< instrument name "EM", "FM101", etc...
  vector<int> towerList; ///< list of installed towers in lat.

  // SECTION: PATHS //
  string rootFileListStr; ///< list of input root files

  string intNonlinFile; ///< input txt filename for integral non-linearity
  string dtdFile; ///< Data descriptoin file for .xml output

  string outputDir; ///< folder for autonamed output files

  string pedFileXML; ///< output xml filename for pedestals
  string asymFileXML; ///< output xml filename for asymmetry calibrations
  string mpdFileXML; ///< output xml filename for MevPerDac calibrations

  string pedHistFile; ///< output ROOT histogram file - pedestal phase
  string asymHistFile; ///< output ROOT histogram file - asymmetry phase
  string mpdHistFile; ///< output ROOT histogram file - MevPerDac phase

  string pedFileTXT; ///< output txt filename for pedestals
  string asymFileLLTXT; ///< output txt filename for asymmetry Large Diode Pos face 2 Large Diode Neg face 
  string asymFileLSTXT; ///< output txt filename for asymmetry Large Diode Pos face 2 Small Diode Neg face 
  string asymFileSLTXT; ///< output txt filename for asymmetry Small Diode Pos face 2 Large Diode Neg face 
  string asymFileSSTXT; ///< output txt filename for asymmetry Small Diode Pos face 2 Small Diode Neg face 
  string largeMPDFileTXT; ///< output txt filename for Mev per Dac Large Diode
  string smallMPDFileTXT; ///< output txt filename for Mev per Dac Small Diode

  string logfile; ///< duplicate of stdout log
  
  
  // SECTION: CONSTANTS //
  double hitThresh;  ///< threshold to count a hit 
  
  double cellHorPitch; ///< horizontal pitch between 2 cal xtals
  double cellVertPitch; ///< vertical pitch between 2 cal xtals

  double maxAsymLL; ///< used in omission of events w/ bad asymmetry logratio
  double maxAsymLS; ///< used in omission of events w/ bad asymmetry logratio
  double maxAsymSL; ///< used in omission of events w/ bad asymmetry logratio
  double maxAsymSS; ///< used in omission of events w/ bad asymmetry logratio
  double minAsymLL; ///< used in omission of events w/ bad asymmetry logratio
  double minAsymLS; ///< used in omission of events w/ bad asymmetry logratio
  double minAsymSL; ///< used in omission of events w/ bad asymmetry logratio
  double minAsymSS; ///< used in omission of events w/ bad asymmetry logratio
  
  // SECTION: GENERAL //
  int nEvtRoughPed; ///< number of events for rough pedestal calibration
  int nEvtPed; ///< number of events for Pedestal calibration
  int nEvtAsym; ///< number of events for Asymmetry calibration
  int nEvtMPD; ///< number of events for MevPerDac calibration

  bool readInPeds; ///< skip pedestal calibration and read in previous results from .txt file
  bool readInAsym; ///< skip Asymmetry calibration and read in previous results from .txt file
  bool skipMPD;  ///< smip MevPerDac calibration

  bool genXML; ///< generate xml output
  bool genTXT; ///< generate text output
  bool genHistfiles; ///< generate histogram output
  bool genLogfile; ///< clone stdout stream to a logfile

  bool genOptAsymHists; ///< generate optional asymmetry histograms

  // DERIVED FROM CFG PARAMES //
  vector<string> rootFileList;
  
  /// multiplexing output stream will contain at least cout, but
  /// may also contain a logfile stream if the user requests it.
  multiplexor_ostream ostr;
  ofstream logstr;

private:
  string baseFilename; ///< string shared by all autogenerated output filenames 

  // Section decription strings
  static const string TEST_INFO; ///< TEST_INFO xml IFile section name
  static const string PATHS; ///< PATHS xml IFile section name
  static const string CONSTANTS; ///< CONSTANTS xml IFile section name
  static const string GENERAL; ///< GENERAL xml IFile section name

  bool valid;   // set to false member data is incomplete/invalid.
};

#endif // mcCfg_H
