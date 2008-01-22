// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/fitGCRHists.cxx,v 1.11 2007/06/13 22:42:11 fewtrell Exp $ //

/** @file 
    @author Zachary Fewtrell

    fit histograms from genGCRHists & output mevPerDAC calibration constants.
*/

// LOCAL INCLUDES
#include "lib/Util/CfgMgr.h"
#include "lib/Util/CGCUtil.h"
#include "lib/Hists/GCRHists.h"
#include "lib/Hists/GCRFit.h"

// GLAST INCLUDES
#include "CalUtil/SimpleCalCalib/CalMPD.h"

// EXTLIB INCLUDES
#include "TFile.h"

// STD INCLUDES
#include <fstream>
#include <string>

using namespace std;
using namespace calibGenCAL;
using namespace CfgMgr;
using namespace CalUtil;

class AppCfg {
public:
  AppCfg(const int argc,
         const char **argv) :
    cmdParser(path_remove_ext(__FILE__)),
    inputROOTPath("inputROOTPath",
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
    cmdParser.registerArg(inputROOTPath);
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
  CmdArg<string> inputROOTPath;

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
    LogStrm::addStream(cout);
    // generate logfile name
    const string logfile(cfg.outputBasename.getVal() + ".log.txt");
    ofstream tmpStrm(logfile.c_str());

    LogStrm::addStream(tmpStrm);

    //-- LOG SOFTWARE VERSION INFO --//
    output_env_banner(LogStrm::get());
    LogStrm::get() << endl;
    cfg.cmdParser.printStatus(LogStrm::get());
    LogStrm::get() << endl;

    const string outputROOTFilename(cfg.outputBasename.getVal() + ".root");
    LogStrm::get() << __FILE__ << ": opening output histogram file: " << outputROOTFilename << endl;
    TFile outputROOTFile(outputROOTFilename.c_str(),
                         "RECREATE",
                         "Fitted GCR Histograms");

    /// retrieve histograms from previous analysis.
    LogStrm::get() << __FILE__ << ": opening input histogram file: " << cfg.inputROOTPath.getVal() << endl;
    TFile inputROOTFile(cfg.inputROOTPath.getVal().c_str(),"READ");
    GCRHists  gcrHists(cfg.summaryMode.getVal(), &outputROOTFile, &inputROOTFile);

    
    CalMPD calMPD;
    LogStrm::get() << __FILE__ << ": fitting histograms" << endl;
    GCRFit::gcrFitGaus(gcrHists, calMPD, &outputROOTFile);

    // output txt file name
    //const string   outputTXTFile(cfg.outputBasename.getVal()+".txt");
    //calMPD.writeTXT(outputTXTFile);
    
    outputROOTFile.Write();
    outputROOTFile.Close();
  } catch (exception &e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
    return -1;
  }

  return 0;
}
