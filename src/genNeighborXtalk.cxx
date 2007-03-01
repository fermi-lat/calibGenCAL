// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/genNeighborXtalk.cxx,v 1.3 2007/02/28 19:52:29 fewtrell Exp $

/** @file Gen Neighboring Crystal Cross-talk calibrations from singlex16 charge injection event files
    @author Zachary Fewtrell
 */

// LOCAL INCLUDES
#include "lib/CGCUtil.h"
#include "lib/CfgMgr.h"
#include "lib/NeighborXtalk.h"
#include "lib/NeighborXtalkAlg.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"

// EXTLIB INCLUDES

// STD INCLUDES
#include <iostream>
#include <string>
#include <fstream>

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
    outputBasePath("outputBasePath",
                   "output file path w/o extention (file extensions will be appended",
                   "")

  {
    // register commandline arguments in order

    // register positional arguments
    cmdParser.registerArg(rootFileLE);
    cmdParser.registerArg(outputBasePath);

    try {
      // parse commandline
      cmdParser.parseCmdLine(argc, argv);
    } catch (InvalidCmdLine &e) {
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
  CmdArg<string> outputBasePath;
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
    // generate logfile name
    const std::string outputDir("./");
    string logfile = cfg.outputBasePath.getVal() + ".log.txt";
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
    

    string txtfile = cfg.outputBasePath.getVal() + ".txt";
    LogStream::get() << __FILE__ << ": saving xtalk to txt file: "
                     << txtfile << endl;
    xtalk.writeTXT(txtfile);

    string tuplefile = cfg.outputBasePath.getVal() + ".tuple.root";
    LogStream::get() << __FILE__ << ": saving xtalk to tuple ROOT file: "
                     << tuplefile << endl;
    xtalk.writeTuple(tuplefile);


    
    
  } catch (exception &e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
  }

  return 0;
}

