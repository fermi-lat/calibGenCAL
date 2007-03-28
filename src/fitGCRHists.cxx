// $Header: $ //

/** @file 
    @author Zachary Fewtrell

    fit histograms from genGCRHists & output mevPerDAC calibration constants.
*/

// LOCAL INCLUDES
#include "lib/Hists/GCRHists.h"
#include "lib/Util/CfgMgr.h"
#include "lib/Util/CGCUtil.h"
#include "lib/CalibDataTypes/CalMPD.h"

// GLAST INCLUDES

// EXTLIB INCLUDES

// STD INCLUDES
#include <fstream>
#include <string>

using namespace std;
using namespace CfgMgr;
using namespace CGCUtil;

class AppCfg {
public:
  AppCfg(const int argc,
         const char **argv) :
    cmdParser(path_remove_ext(__FILE__)),
    histFile("histFile",
             "input ROOT histogram file",
             ""),
    summaryMode("summaryMode",
                's',
                "generate summary histograms only (no individual channel hists)")

  {
    cmdParser.registerSwitch(summaryMode);
    cmdParser.registerArg(histFile);

    try {
      cmdParser.parseCmdLine(argc, argv);
    } catch (exception &e) {
      cout << e.what() << endl;
      cmdParser.printUsage();
      throw e;
    }
  }
              
  /// construct new parser
  CmdLineParser cmdParser;

  /// input ROOT file w/ histograms
  CmdArg<string> histFile;

  /// operate histograms in 'summary mode' where you use histograms w/ sums of all channels together.
  CmdSwitch summaryMode;
};

int main(const int argc,
         const char **argv) {
  // libCalibGenCAL will throw runtime_error
  try {

    //-- COMMAND LINE --//
    AppCfg                                     cfg(argc,
                                                   argv);

    //-- SETUP LOG FILE --//
    /// multiplexing output streams
    /// simultaneously to cout and to logfile
    LogStream::addStream(cout);
    // generate logfile name
    const string outputDir("./");

    string logfile =
      CGCUtil::genOutputFilename(outputDir,
                                 "gcrCalib",
                                 cfg.histFile.getVal(),
                                 "log.txt");
    ofstream tmpStrm(logfile.c_str());

    LogStream::addStream(tmpStrm);

    //-- LOG SOFTWARE VERSION INFO --//
    output_env_banner(LogStream::get());

    GCRHists  gcrHists(cfg.summaryMode.getVal());

	gcrHists.loadHists(cfg.histFile.getVal());

    CalMPD calMPD;
	set<unsigned short> zSet;
	zSet.insert(1);

	gcrHists.fitHists(calMPD, zSet);

	// output txt file name
    string   outputTXTFile =
      CalMPD::genFilename(outputDir,
                          cfg.histFile.getVal(),
                          "txt");



  } catch (exception &e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
  }

  return 0;
}
