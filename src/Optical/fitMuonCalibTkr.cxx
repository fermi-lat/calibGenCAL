/** @file Fit MevPerDAC & Asym histograms generated with genMuonCalibTkr

    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "src/lib/Hists/AsymHists.h"
#include "src/lib/Hists/MPDHists.h"
#include "src/lib/Util/CfgMgr.h"
#include "src/lib/Util/CGCUtil.h"
#include "src/lib/Util/string_util.h"


// GLAST INCLUDES
#include "facilities/Util.h"
#include "CalUtil/SimpleCalCalib/CalAsym.h"
#include "CalUtil/SimpleCalCalib/CalMPD.h"

// EXTLIB INCLUDES
#include "TFile.h"

// STD INCLUDES
#include <sstream>
#include <fstream>

using namespace std;
using namespace calibGenCAL;
using namespace CfgMgr;
using namespace facilities;
using namespace CalUtil;

class AppCfg {
public:
  AppCfg(const int argc,
         const char **argv) :
    cmdParser(path_remove_ext(__FILE__)),
    skipAsym("skipAsym",
             'm',
             "Skip processing asymmetry histograms"),
    outputBasename("outputBasename",
                   "all output files will use this basename + some_ext",
                   ""),
    help("help",
         'h',
         "print usage info")

  {
    cmdParser.registerArg(outputBasename);
    cmdParser.registerSwitch(skipAsym);
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

  /// skip asymmetry processing (mpd only)
  CmdSwitch skipAsym;

  CmdArg<string> outputBasename;


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
    string logfile(cfg.outputBasename.getVal() + ".log.txt");
    ofstream tmpStrm(logfile.c_str());

    LogStrm::addStream(tmpStrm);

    //-- LOG SOFTWARE VERSION INFO --//
    output_env_banner(LogStrm::get());
    LogStrm::get() << endl;
    cfg.cmdParser.printStatus(LogStrm::get());
    LogStrm::get() << endl;

    //-- MUON CALIB
    // output histogram file name
    string histFilename(cfg.outputBasename.getVal() + ".root");
    string mpdTXTFile(cfg.outputBasename.getVal() + ".calMPD.txt");

    ///////////////////////////////////////
    //-- OPEN HISTOGRAM FILE             //
    //   (either 'CREATE' or 'UPDATE') --//
    ///////////////////////////////////////

    // open file to save output histograms.
    LogStrm::get() << __FILE__ << ": opening output histogram file: "
                     << histFilename << endl;
    TFile histFile(histFilename.c_str(), "UPDATE", "CAL Muon Calib");

    MPDHists  mpdHists(MPDHists::FitMethods::LANGAU);
    mpdHists.loadHists(histFile);

    CalMPD    calMPD;

    if (!cfg.skipAsym.getVal()) {
      CalAsym   calAsym;

      AsymHists asymHists;
      asymHists.loadHists(histFile);
      LogStrm::get() << __FILE__ << ": fitting asymmetry histograms." << endl;
      asymHists.fitHists(calAsym);
      
      string asymTXTFile(cfg.outputBasename.getVal() + ".calAsym.txt");
      LogStrm::get() << __FILE__ << ": writing muon asymmetry: "
                     << asymTXTFile << endl;
      calAsym.writeTXT(asymTXTFile);
    }

    LogStrm::get() << __FILE__ << ": fitting MeVPerDAC histograms." << endl;
    mpdHists.fitHists(calMPD);

    LogStrm::get() << __FILE__ << ": writing muon mevPerDAC: "
                     << mpdTXTFile << endl;
    calMPD.writeTXT(mpdTXTFile);
    
    LogStrm::get() << __FILE__ << ": generating mpd fit result tuple: " << endl;
    mpdHists.buildTuple();

    LogStrm::get() << __FILE__ << ": writing histogram file: "
                     << histFilename << endl;
    histFile.Write();

    LogStrm::get() << __FILE__ << ": Successfully completed." << endl;
  } catch (exception &e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
    return -1;
  }

  return 0;
}

