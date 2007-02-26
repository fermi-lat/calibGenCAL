// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/genNeighborXtalk.cxx,v 1.12 2007/02/08 21:26:17 fewtrell Exp $

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
    rootFileLE("rootFileLE",
               'l',
               "low energy input singlex16 digi root event file",
               ""),
    rootFileHE("rootFileHE",
               'h',
               "high energy input singlex16 digi root event file",
               ""),
     outputBasePath("outputBasePath",
	                "output file path w/o extention (file extensions will be appended",
					"")

  {
    // register commandline arguments in order
    cmdParser.registerVar(rootFileLE);
    cmdParser.registerVar(rootFileHE);

	// register positional arguments


    // parse commandline
    cmdParser.parseCmdLine(argc, argv);
  }
  // construct new parser
  CmdLineParser cmdParser;

  /// low energy input singlex16 digi root event file
  CmdOptVar<string> rootFileLE;

  /// high energy input singlex16 digi root event file
  CmdOptVar<string> rootFileHE;

  /// output file path w/o extention (file extensions will be appended)
  CmdArg<string> outputBasePath;

};


int main(const int argc,
         const char **argv) {
  // libCalibGenCAL will throw runtime_error
  try {
    //-- CONFIG --//
    AppCfg cfg(argc, argv);

    //-- POST CFG --//
    // must be at least 1 file to process.
    if (cfg.rootFileLE.getVal().length() == 0 
        && 
        cfg.rootFileHE.getVal().length() == 0) {
      cout << __FILE__ << ": no input files specified." << endl;
      cfg.cmdParser.printUsage();
      return -1;
    }

    const  string &outputBaseName = (cfg.rootFileLE.getVal().length()) ?
      cfg.rootFileLE.getVal() : cfg.rootFileHE.getVal();

    //-- SETUP LOG FILE --//

    /// multiplexing output streams
    /// simultaneously to cout and to logfile
    LogStream::addStream(cout);
    // generate logfile name
    const std::string outputDir("./");
    string logfile = cfg.outputBasePath.getVal() + ".log.txt";
      
    ofstream tmpStrm(logfile.c_str());

    LogStream::addStream(tmpStrm);

    cfg.cmdParser.printStatus(LogStream::get());

	NeighborXtalk xtalk;
	NeighborXtalkAlg xtalkAlg;

    //-- LOG SOFTWARE VERSION INFO --//
    output_env_banner(LogStream::get());

    if (cfg.rootFileLE.getVal().length()) {
      LogStream::get() << __FILE__ << ": reading LE calibGen event file: " << cfg.rootFileLE.getVal() << endl;
	  xtalkAlg.readRootData(cfg.rootFileLE.getVal(), xtalk, LRG_DIODE);
    }
    
    if (cfg.rootFileHE.getVal().length()) {
      LogStream::get() << __FILE__ << ": reading HE calibGen event file: " << cfg.rootFileHE.getVal() << endl;
      xtalkAlg.readRootData(cfg.rootFileHE.getVal(), xtalk, SM_DIODE);
    }

	string txtfile = cfg.outputBasePath.getVal() + ".txt";
    
    LogStream::get() << __FILE__ << ": saving xtalk to txt file: "
                     << txtfile << endl;
    xtalk.writeTXT(txtfile);

  } catch (exception &e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
  }

  return 0;
}

