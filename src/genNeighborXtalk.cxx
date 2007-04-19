// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/genNeighborXtalk.cxx,v 1.9 2007/04/10 21:26:41 fewtrell Exp $

/** @file Gen Neighboring Crystal Cross-talk calibrations from singlex16 charge injection event files
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "lib/CalibDataTypes/NeighborXtalk.h"
#include "lib/Algs/NeighborXtalkAlg.h"
#include "lib/Util/CGCUtil.h"
#include "lib/Util/CfgMgr.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"

// EXTLIB INCLUDES

// STD INCLUDES
#include <iostream>
#include <string>
#include <fstream>
#include <stdexcept>

using namespace std;
using namespace CalUtil;
using namespace CGCUtil;
using namespace CfgMgr;

class AppCfg {
public:
  AppCfg(const int argc,
         const char **argv) :
    cmdParser(argv[0]),
    rootFileLE("rootFileLE",
               "low energy input singlex16 digi root event file",
               ""),
    outputBasename("outputBasename",
                   "output file path w/o extention (file extensions will be appended",
                   ""),
    help("help",
         'h',
         "print usage info")
  {
    // register commandline arguments in order

    // register positional arguments
    cmdParser.registerArg(rootFileLE);
    cmdParser.registerArg(outputBasename);
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

  /// low energy input singlex16 digi root event file
  CmdArg<string> rootFileLE;

  /// output file path w/o extention (file extensions will be appended)
  CmdArg<string> outputBasename;

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
    LogStream::addStream(cout);
    string logfile = cfg.outputBasename.getVal() + ".log.txt";
    ofstream          tmpStrm(logfile.c_str());
    LogStream::addStream(tmpStrm);


    cfg.cmdParser.printStatus(LogStream::get());

    NeighborXtalk       xtalk;
    NeighborXtalkAlg    xtalkAlg;

    //-- LOG SOFTWARE VERSION INFO --//
    output_env_banner(LogStream::get());

    LogStream::get() << __FILE__ << ": reading LE calibGen event file: " << cfg.rootFileLE.getVal() << endl;
    xtalkAlg.readRootData(cfg.rootFileLE.getVal(), xtalk);

    LogStream::get() << __FILE__ << ": pedestal subtract: " << endl;
    xtalk.pedSubtractADC();
    

    string txtfile = cfg.outputBasename.getVal() + ".txt";
    LogStream::get() << __FILE__ << ": saving xtalk to txt file: "
                     << txtfile << endl;
    xtalk.writeTXT(txtfile);

    string tuplefile = cfg.outputBasename.getVal() + ".tuple.root";
    LogStream::get() << __FILE__ << ": saving xtalk to tuple ROOT file: "
                     << tuplefile << endl;
    xtalk.writeTuples(tuplefile);
    
  } catch (exception &e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
    return -1;
  }

  return 0;
}

