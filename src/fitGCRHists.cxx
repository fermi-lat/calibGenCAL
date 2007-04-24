// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/fitGCRHists.cxx,v 1.5 2007/04/19 15:03:14 fewtrell Exp $ //

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
#include "TFile.h"

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
    outputBasename("outputBaseName",
                   "base filename for all output files",
                   ""),
    summaryMode("summaryMode",
                's',
                "generate summary histograms only (no individual channel hists)"),
    help("help",
         'h',
         "print usage info")

  {
    cmdParser.registerArg(histFile);
    cmdParser.registerArg(outputBasename);
    cmdParser.registerSwitch(summaryMode);
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

  /// input ROOT file w/ histograms
  CmdArg<string> histFile;

  /// base filename for all output files
  CmdArg<string> outputBasename;

  /// operate histograms in 'summary mode' where you use histograms w/ sums of all channels together.
  CmdSwitch summaryMode;

  /// print usage string
  CmdSwitch help;

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
    string logfile(cfg.outputBasename.getVal() + ".log.txt");
    ofstream tmpStrm(logfile.c_str());

    LogStream::addStream(tmpStrm);

    //-- LOG SOFTWARE VERSION INFO --//
    output_env_banner(LogStream::get());
    LogStream::get() << endl;
    cfg.cmdParser.printStatus(LogStream::get());
    LogStream::get() << endl;

    GCRHists  gcrHists(cfg.summaryMode.getVal());

    gcrHists.loadHists(cfg.histFile.getVal());


    string outputROOTFilename(cfg.outputBasename.getVal() + ".root");
    TFile outputROOTFile(outputROOTFilename.c_str(),
                         "RECREATE",
                         "Fitted GCR Histograms");
    gcrHists.setHistDir(&outputROOTFile);
    
    CalMPD calMPD;
    set<unsigned short> zSet;
    zSet.insert(1);

    gcrHists.fitHists(calMPD, zSet);

    // output txt file name
    string   outputTXTFile(cfg.outputBasename.getVal()+".txt");
    

    calMPD.writeTXT(outputTXTFile);
    
    outputROOTFile.Write();
    outputROOTFile.Close();
  } catch (exception &e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
    return -1;
  }

  return 0;
}
