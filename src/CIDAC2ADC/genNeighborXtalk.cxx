// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/CIDAC2ADC/genNeighborXtalk.cxx,v 1.4 2008/05/19 17:37:28 fewtrell Exp $

/** @file Gen Neighboring Crystal Cross-talk calibrations from singlex16 charge injection event files
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "NeighborXtalkAlg.h"
#include "src/lib/Util/CfgMgr.h"
#include "src/lib/Util/CGCUtil.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/SimpleCalCalib/NeighborXtalk.h"

// EXTLIB INCLUDES

// STD INCLUDES
#include <iostream>
#include <string>
#include <fstream>
#include <stdexcept>

using namespace std;
using namespace calibGenCAL;
using namespace CalUtil;
using namespace CfgMgr;

/// Manage application configuration parameters
class AppCfg {
public:
  AppCfg(const int argc,
         const char **argv) :
    cmdParser(argv[0]),
    nSamplesPerCIDAC("nSamplesPerCIDAC",
                    'n',
                     "Number of samples for each CIDAC level",
                     100),
    rootFileLE("rootFileLE",
               "low energy input singlex16 digi root event file",
               ""),
    outputBasename("outputBasename",
                   "output file path w/o extention (file extensions will be appended",
                   ""),
    altLoopScheme("altLoopScheme",
                  'a',
                  "enable alternate LCI loop scheme used in run #077015240"),
    help("help",
         'h',
         "print usage info")
  {
    // register commandline arguments in order

    // register positional arguments
    cmdParser.registerArg(rootFileLE);
    cmdParser.registerArg(outputBasename);
    cmdParser.registerVar(nSamplesPerCIDAC);
    cmdParser.registerSwitch(altLoopScheme);
    cmdParser.registerSwitch(help);

    try {
      // parse commandline
      cmdParser.parseCmdLine(argc, argv);
    } catch (exception &e) {
      // ignore invalid commandline if user asked for help.
      if (!help.getVal())
        cout << e.what() << endl;
      cmdParser.printUsage();
      exit(-1);
    }
  }

  // construct new parser
  CmdLineParser cmdParser;

  CmdOptVar<unsigned short> nSamplesPerCIDAC;

  /// low energy input singlex16 digi root event file
  CmdArg<string> rootFileLE;

  /// output file path w/o extention (file extensions will be appended)
  CmdArg<string> outputBasename;

  ///  enable alternate LCI loop scheme used in run #077015240
  CmdSwitch altLoopScheme;

  /// print usage string
  CmdSwitch help;

};

int main(const int argc,
         const char **argv) {
  // libCalibGenCAL will throw runtime_error
  try {
    //-- CONFIG --//
    AppCfg cfg(argc,
               argv);

    //-- SETUP LOG FILE --//

    /// multiplexing output streams
    /// simultaneously to cout and to logfile
    LogStrm::addStream(cout);
    string logfile = cfg.outputBasename.getVal() + ".log.txt";
    ofstream          tmpStrm(logfile.c_str());
    LogStrm::addStream(tmpStrm);


    cfg.cmdParser.printStatus(LogStrm::get());

    NeighborXtalk       xtalk;
    const singlex16 sx16(cfg.nSamplesPerCIDAC.getVal());
    NeighborXtalkAlg    xtalkAlg(sx16, cfg.altLoopScheme.getVal());

    //-- LOG SOFTWARE VERSION INFO --//
    output_env_banner(LogStrm::get());
    LogStrm::get() << endl;
    cfg.cmdParser.printStatus(LogStrm::get());
    LogStrm::get() << endl;

    LogStrm::get() << __FILE__ << ": reading LE calibGen event file: " << cfg.rootFileLE.getVal() << endl;
    xtalkAlg.readRootData(cfg.rootFileLE.getVal(), xtalk);

    LogStrm::get() << __FILE__ << ": pedestal subtract: " << endl;
    xtalk.pedSubtractADC();
    

    const string txtfile = cfg.outputBasename.getVal() + ".txt";
    LogStrm::get() << __FILE__ << ": saving xtalk to txt file: "
                     << txtfile << endl;
    xtalk.writeTXT(txtfile);

    const string tuplefile = cfg.outputBasename.getVal() + ".tuple.root";
    LogStrm::get() << __FILE__ << ": saving xtalk to tuple ROOT file: "
                     << tuplefile << endl;
    xtalk.writeTuples(tuplefile);
    
    LogStrm::get() << __FILE__ << ": Successfully completed." << endl;
  } catch (exception &e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
    return -1;
  }

  return 0;
}

