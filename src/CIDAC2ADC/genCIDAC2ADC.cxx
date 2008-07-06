// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/CIDAC2ADC/genCIDAC2ADC.cxx,v 1.9 2008/05/19 17:37:28 fewtrell Exp $

/** @file Gen CIDAC2ADC calibrations from singlex16 charge injection event files
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "IntNonlinAlg.h"
#include "src/lib/Util/CfgMgr.h"
#include "src/lib/Util/CGCUtil.h"
#include "src/lib/Util/string_util.h"
#include "src/lib/Specs/singlex16.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/SimpleCalCalib/CIDAC2ADC.h"

// EXTLIB INCLUDES
#include "TFile.h"

// STD INCLUDES
#include <iostream>
#include <string>
#include <fstream>

using namespace std;
using namespace calibGenCAL;
using namespace CalUtil;
using namespace CfgMgr;

/// Manage application configuration parameters
class AppCfg {
public:
  AppCfg(const int argc,
         const char **argv) :
    cmdParser(path_remove_ext(__FILE__)),
    rootFileHE("rootFileHE",
               'h',
               "input HE DIODE singlex16 digi root file (one of either -l or -h is required)",
               ""),
    rootFileLE("rootFileLE",
               'l',
               "input LE DIODE singlex16 digi root file (one of either -l or -h is required)",
               ""),
    nSamplesPerCIDAC("nSamplesPerCIDAC",
                     'n',
                     "Number of samples for each CIDAC level",
                     100),
    columnMode("columnMode",
               'c',
               "singlex16 pulses 12 columns individually"),
    hugeTuple("hugeTuple",
              't',
              "generate HUGE tuple with every ADC value (good for in-depth noise studies"),
    outputBasename("outputBasename",
                   "all output files will use this basename + some_ext",
                   "")
  {
    cmdParser.registerArg(outputBasename);
    
    cmdParser.registerVar(rootFileHE);
    cmdParser.registerVar(rootFileLE);
    cmdParser.registerVar(nSamplesPerCIDAC);

    cmdParser.registerSwitch(columnMode);
    cmdParser.registerSwitch(hugeTuple);

    try {
      cmdParser.parseCmdLine(argc, argv);
    } catch (exception &e) {
      cmdParser.printUsage();
      exit(-1);
    }

  }
  /// construct new parser
  CmdLineParser cmdParser;
  
  CmdOptVar<string> rootFileHE;
  CmdOptVar<string> rootFileLE;
  CmdOptVar<unsigned short> nSamplesPerCIDAC;

  CmdSwitch columnMode;
  CmdSwitch hugeTuple;
  
  CmdArg<string> outputBasename;

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
    const string logfile(cfg.outputBasename.getVal() + ".adcmean.log.txt");
    ofstream tmpStrm(logfile.c_str());

    LogStrm::addStream(tmpStrm);

    //-- LOG SOFTWARE VERSION INFO --//
    output_env_banner(LogStrm::get());
    LogStrm::get() << endl;
    cfg.cmdParser.printStatus(LogStrm::get());
    LogStrm::get() << endl;

    /// root output filename
    const string outputROOTPath(cfg.outputBasename.getVal() + ".adcmean.root");
    LogStrm::get() << __FILE__ << ": opening output ROOT file: " << outputROOTPath << endl;
    TFile outputROOTFile(outputROOTPath.c_str(), "RECREATE", "Cal IntNolin calib", 9);

    CIDAC2ADC    adcMeans;
    CIDAC2ADC    cidac2adc;
    const singlex16 sx16(cfg.nSamplesPerCIDAC.getVal());
    IntNonlinAlg inlAlg(sx16, cfg.hugeTuple.getVal());

    /// adc mean output filename
    const string adcMeanPath(cfg.outputBasename.getVal() + ".adcmean.txt");

    if (cfg.rootFileLE.getVal().length()) {
      LogStrm::get() << __FILE__ << ": reading LE calibGen event file: " << cfg.rootFileLE.getVal() << endl;
      inlAlg.readRootData(cfg.rootFileLE.getVal(), adcMeans, LRG_DIODE, !cfg.columnMode.getVal());
    }

    if (cfg.rootFileHE.getVal().length()) {
      LogStrm::get() << __FILE__ << ": reading HE calibGen event file: " << cfg.rootFileHE.getVal() << endl;
      inlAlg.readRootData(cfg.rootFileHE.getVal(), adcMeans, SM_DIODE,  !cfg.columnMode.getVal());
    }

    LogStrm::get() << __FILE__ << ": saving adc means to txt file: "
                   << adcMeanPath << endl;
    adcMeans.writeTXT(adcMeanPath);

    LogStrm::get() << __FILE__ << ": Writing output ROOT file." << endl;
    outputROOTFile.Write();
    outputROOTFile.Close();

    LogStrm::get() << __FILE__ << ": Successfully completed." << endl;
  } catch (exception &e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
    return -1;
  }

  return 0;
}

