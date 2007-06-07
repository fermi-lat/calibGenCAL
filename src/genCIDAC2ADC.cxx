// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/genCIDAC2ADC.cxx,v 1.22 2007/05/25 21:06:46 fewtrell Exp $

/** @file Gen CIDAC2ADC calibrations from singlex16 charge injection event files
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "lib/Algs/IntNonlinAlg.h"
#include "lib/CalibDataTypes/CIDAC2ADC.h"
#include "lib/Util/CGCUtil.h"
#include "lib/Util/CfgMgr.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"

// EXTLIB INCLUDES

// STD INCLUDES
#include <iostream>
#include <string>
#include <fstream>

using namespace std;
using namespace calibGenCAL;
using namespace CalUtil;
using namespace CGCUtil;
using namespace CfgMgr;

class AppCfg {
public:
  AppCfg(const int argc,
         const char **argv) :
    cmdParser(path_remove_ext(__FILE__)),
    rootFileHE("rootFileHE",
               'h',
               "input HE DIODE singlex16 digi root file (one of eitehr -l or -h is required)",
               ""),
    rootFileLE("rootFileLE",
               'l',
               "input LE DIODE singlex16 digi root file (one of either -l or -h is required)",
               ""),
    columnMode("columnMode",
               'c',
               "singlex16 pulses 12 columns individually"),
    outputBasename("outputBasename",
                   "all output files will use this basename + some_ext",
                   ""),
    help("help",
         'h',
         "print usage info")
  {
    cmdParser.registerArg(outputBasename);
    
    cmdParser.registerVar(rootFileHE);
    cmdParser.registerVar(rootFileLE);

    cmdParser.registerSwitch(columnMode);
    cmdParser.registerSwitch(help);



    try {
      cmdParser.parseCmdLine(argc, argv);
    } catch (exception &e) {
      // ignore invalid commandline if user asked for help.
      if (!help.getVal())
        cout << e.what() << endl;
      cmdParser.printUsage();
      exit(-1);
    }

  }
  /// construct new parser
  CmdLineParser cmdParser;
  
  CmdOptVar<string> rootFileHE;
  CmdOptVar<string> rootFileLE;

  CmdSwitch columnMode;
  
  CmdArg<string> outputBasename;
  /// print usage string
  CmdSwitch help;

};

int main(int argc,
         const char **argv) {
  // libCalibGenCAL will throw runtime_error
  try {
    AppCfg cfg(argc,argv);

    // i can process 1 or 2 files, but not none
    if (cfg.rootFileLE.getVal().length() == 0 && cfg.rootFileHE.getVal().length() == 0) {
      cout << __FILE__ << ": no input files specified." << endl;
      return -1;
    }

    //-- SETUP LOG FILE --//
    /// multiplexing output streams
    /// simultaneously to cout and to logfile
    LogStrm::addStream(cout);
    // generate logfile name
    string logfile(cfg.outputBasename.getVal() + ".log.txt");
    ofstream tmpStrm(logfile.c_str());

    LogStrm::addStream(tmpStrm);

    //-- LOG SOFTWARE VERSION INFO --//
    output_env_banner(LogStrm::get());
    LogStrm::get() << endl;
    cfg.cmdParser.printStatus(LogStrm::get());
    LogStrm::get() << endl;

    // txt output filename
    string       outputTXTFile(cfg.outputBasename.getVal() + ".txt");

    CIDAC2ADC    adcMeans;
    CIDAC2ADC    cidac2adc;
    IntNonlinAlg inlAlg;

    const string       adcMeanFile(cfg.outputBasename.getVal() + ".adcmean.txt");

    if (cfg.rootFileLE.getVal().length()) {
      LogStrm::get() << __FILE__ << ": reading LE calibGen event file: " << cfg.rootFileLE.getVal() << endl;
      inlAlg.readRootData(cfg.rootFileLE.getVal(), adcMeans, LRG_DIODE, !cfg.columnMode.getVal());
    }

    if (cfg.rootFileHE.getVal().length()) {
      LogStrm::get() << __FILE__ << ": reading HE calibGen event file: " << cfg.rootFileHE.getVal() << endl;
      inlAlg.readRootData(cfg.rootFileHE.getVal(), adcMeans, SM_DIODE,  !cfg.columnMode.getVal());
    }

    LogStrm::get() << __FILE__ << ": saving adc means to txt file: "
                     << adcMeanFile << endl;
    adcMeans.writeTXT(adcMeanFile);

    LogStrm::get() << __FILE__ << ": generating smoothed spline points: " << endl;
    inlAlg.genSplinePts(adcMeans, cidac2adc);

    LogStrm::get() << __FILE__ << ": writing smoothed spline points: " << outputTXTFile << endl;
    cidac2adc.writeTXT(outputTXTFile);
  } catch (exception &e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
    return -1;
  }

  return 0;
}

