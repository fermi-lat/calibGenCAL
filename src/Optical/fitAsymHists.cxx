// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/Optical/genMuonAsym.cxx,v 1.4 2008/05/19 17:37:28 fewtrell Exp $

/** @file fit Cal Light Asymmetry Histograms & output TXT calibration data

    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "MuonAsymAlg.h"
#include "src/lib/Hists/AsymHists.h"
#include "src/lib/Util/CfgMgr.h"
#include "src/lib/Util/CGCUtil.h"
#include "src/lib/Util/string_util.h"
#include "src/lib/Util/stl_util.h"

// GLAST INCLUDES
#include "CalUtil/SimpleCalCalib/CalAsym.h"

// EXTLIB INCLUDES
#include "TFile.h"

// STD INCLUDES
#include <iostream>
#include <string>
#include <fstream>

using namespace std;
using namespace calibGenCAL;
using namespace CfgMgr;
using namespace CalUtil;

/// Manage application configuration parameters
class AppCfg {
public:
  AppCfg(const int argc,
         const char **argv) :
    cmdParser(path_remove_ext(__FILE__)),
    outputBasename("outputBasename",
                   "all output files will use this basename + some_ext",
                   ""),
    muonGain("muonGain",
             'm',
             "assume cal in MUON GAIN mode instead of FLIGHT_GAIN"),
    help("help",
         'h',
         "print usage info")
  {
    cmdParser.registerArg(outputBasename);
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
  CmdArg<string> outputBasename;

  CmdSwitch muonGain;
  /// print usage string
  CmdSwitch help;

};

int main(int argc,
         const char **argv) {
  // libCalibGenCAL will throw runtime_error
  try {
    AppCfg cfg(argc, argv);

    //-- SETUP LOG FILE --//
    /// multiplexing output streams
    /// simultaneously to cout and to logfile
    LogStrm::addStream(cout);
    const string logfile(cfg.outputBasename.getVal() + ".log.txt");
    ofstream tmpStrm(logfile.c_str());

    LogStrm::addStream(tmpStrm);

    //-- LOG SOFTWARE VERSION INFO --//
    output_env_banner(LogStrm::get());
    LogStrm::get() << endl;
    cfg.cmdParser.printStatus(LogStrm::get());
    LogStrm::get() << endl;

    //-- LIGHT ASYM
    // input histogram file name
    const string histFilename(cfg.outputBasename.getVal() + ".root");

    // output txt file name
    const string outputTXTFile(cfg.outputBasename.getVal() + ".txt");

    CalAsym   calAsym;

    // open file to save output histograms.
    LogStrm::get() << __FILE__ << ": opening input histogram file: "
                   << histFilename << endl;
    TFile histFile(histFilename.c_str(),
                   "READ",
                   "CAL Light Asymmetry");
    if (!histFile.IsOpen()) {
      LogStrm::get() << __FILE__ << ": ERROR: Opening file: " << histFilename << endl;
      return -1;
    }
    
    CalResponse::CAL_GAIN_INTENT calGain = (cfg.muonGain.getVal()) ? CalResponse::MUON_GAIN : CalResponse::FLIGHT_GAIN;
    AsymHists asymHists(calGain, 12,10,0,&histFile);

    LogStrm::get() << __FILE__ << ": fitting light asymmmetry histograms." << endl;
    asymHists.fitHists(calAsym);

    LogStrm::get() << __FILE__ << ": writing light asymmetry: "
                     << outputTXTFile << endl;
    calAsym.writeTXT(outputTXTFile);
    LogStrm::get() << __FILE__ << ": writing histogram file: "
                     << histFilename << endl;

    LogStrm::get() << __FILE__ << ": Successfully completed." << endl;
  } catch (exception &e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
    return -1;
  }

  return 0;
}

