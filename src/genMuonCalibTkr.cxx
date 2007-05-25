/** @file Generate MevPerDAC & Asym calibrations from Muon event files using
    Tracker recon (from svac) for track & hit information.

    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "lib/CalibDataTypes/CalAsym.h"
#include "lib/CalibDataTypes/CalMPD.h"
#include "lib/CalibDataTypes/CalPed.h"
#include "lib/CalibDataTypes/CIDAC2ADC.h"
#include "lib/Hists/AsymHists.h"
#include "lib/Hists/MPDHists.h"
#include "lib/Algs/MuonCalibTkrAlg.h"
#include "lib/Util/CGCUtil.h"
#include "lib/Util/CfgMgr.h"


// GLAST INCLUDES
#include "facilities/Util.h"

// EXTLIB INCLUDES

// STD INCLUDES
#include <sstream>
#include <fstream>

using namespace std;
using namespace calibGenCAL;
using namespace CGCUtil;
using namespace CfgMgr;
using namespace facilities;

class AppCfg {
public:
  AppCfg(const int argc,
         const char **argv) :
    cmdParser(path_remove_ext(__FILE__)),
    pedTXTFile("pedTXTFile",
               "input cal pedestals txt file",
               ""),
    inlTXTFile("inlTXTFile",
               "input cal cidac2adc txt file",
               ""),
    digiFilenames("digiFilenames",
                  "text file w/ newline delimited list of input digi ROOT files (events must match svac events)",
                  ""
                  ),
    svacFilenames("svacFilenames",
                  "text file w/ newline delimited list of input digi ROOT files(events must match digi events)",
                  ""
                  ),
    outputBasename("outputBasename",
                   "all output files will use this basename + some_ext",
                   ""),
    entriesPerHist("entriesPerHist",
                   'e',
                   "quit after all histograms have > n entries",
                   10000),
    startEvent("startEvent",
               's',
               "start processing @ event # n",
               0),
    cfgPath("cfgPath",
            'c',
            "(optional) read cfg info from this file (supports env var expansion)",
			""),
    help("help",
         'h',
         "print usage info")


  {
    cmdParser.registerArg(pedTXTFile);
    cmdParser.registerArg(inlTXTFile);
    cmdParser.registerArg(digiFilenames);
    cmdParser.registerArg(svacFilenames);
    cmdParser.registerArg(outputBasename);
    cmdParser.registerVar(entriesPerHist);
    cmdParser.registerVar(startEvent);
    cmdParser.registerVar(cfgPath);
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
  CmdArg<string> pedTXTFile;
  CmdArg<string> inlTXTFile;

  CmdArg<string> digiFilenames;
  CmdArg<string> svacFilenames;
  
  CmdArg<string> outputBasename;
  
  CmdOptVar<unsigned> entriesPerHist;
  CmdOptVar<unsigned> startEvent;

  CmdOptVar<string> cfgPath;

  /// print usage string
  CmdSwitch help;
};

int main(int argc,
         const char **argv) {


  // libCalibGenCAL will throw runtime_error
  try {
    AppCfg cfg(argc, argv);

    vector<string> digiFileList(getLinesFromFile(cfg.digiFilenames.getVal()));
    if (digiFileList.size() < 1) {
      cout << __FILE__ << ": No input files specified" << endl;
      return -1;
    }

    vector<string> svacFileList(getLinesFromFile(cfg.svacFilenames.getVal()));
    if (svacFileList.size() < 1) {
      cout << __FILE__ << ": No input files specified" << endl;
      return -1;
    }

    //-- SETUP LOG FILE --//
    /// multiplexing output streams
    /// simultaneously to cout and to logfile
    LogStream::addStream(cout);
    string logfile(cfg.outputBasename.getVal() + ".log.txt");
    ofstream tmpStrm(logfile.c_str());

    LogStream::addStream(tmpStrm);

    //-- LOG SOFTWARE VERSION INFO --//
    output_env_banner(LogStream::get());
    LogStream::get() << endl;
    cfg.cmdParser.printStatus(LogStream::get());
    LogStream::get() << endl;

    //-- RETRIEVE PEDESTALS
    
    CalPed peds;
    LogStream::get() << __FILE__ << ": reading in muon pedestal file: " << cfg.pedTXTFile.getVal() << endl;
    peds.readTXT(cfg.pedTXTFile.getVal());

    //-- RETRIEVE CIDAC2ADC
    CIDAC2ADC dac2adc;
    LogStream::get() << __FILE__ << ": reading in dac2adc txt file: " << cfg.inlTXTFile.getVal() << endl;
    dac2adc.readTXT(cfg.inlTXTFile.getVal());

    LogStream::get() << __FILE__ << ": generating dac2adc splines: " << endl;
    dac2adc.genSplines();

    //-- MUON CALIB
    // output histogram file name
    string histFilename(cfg.outputBasename.getVal() + ".root");
    string   asymTXTFile(cfg.outputBasename.getVal() + ".calAsym.txt");
    string   mpdTXTFile(cfg.outputBasename.getVal() + ".calMPD.txt");

    ///////////////////////////////////////
    //-- OPEN HISTOGRAM FILE             //
    //   (either 'CREATE' or 'UPDATE') --//
    ///////////////////////////////////////

    // open file to save output histograms.
    LogStream::get() << __FILE__ << ": opening output histogram file: "
                     << histFilename << endl;
    TFile histFile(histFilename.c_str(), "RECREATE", "CAL Muon Calib");

    AsymHists asymHists;
    MPDHists     mpdHists(MPDHists::FitMethods::LANGAU);

    CalAsym   calAsym;
    CalMPD    calMPD;
    MuonCalibTkrAlg tkrCalib(peds,
                             dac2adc,
                             asymHists,
                             mpdHists,
                             cfg.cfgPath.getVal());

    LogStream::get() << __FILE__ << ": reading root event file(s) starting w/ "
                     << digiFileList[0] << endl;
    tkrCalib.fillHists(cfg.entriesPerHist.getVal(),
                       digiFileList,
                       svacFileList,
                       cfg.startEvent.getVal());
    mpdHists.trimHists();
    asymHists.trimHists();

    // READ IN TXT OUTPUT (SKIP HISTOGRAM FITS)
    LogStream::get() << __FILE__ << ": fitting asymmetry histograms." << endl;
    asymHists.fitHists(calAsym);

    LogStream::get() << __FILE__ << ": writing muon asymmetry: "
                     << asymTXTFile << endl;
    calAsym.writeTXT(asymTXTFile);

    LogStream::get() << __FILE__ << ": fitting MeVPerDAC histograms." << endl;
    mpdHists.fitHists(calMPD);

    LogStream::get() << __FILE__ << ": writing muon mevPerDAC: "
                     << mpdTXTFile << endl;
    calMPD.writeTXT(mpdTXTFile);

    LogStream::get() << __FILE__ << ": generating mpd fit result tuple: " << endl;
    mpdHists.buildTuple();

    LogStream::get() << __FILE__ << ": writing histogram file: "
                     << histFilename << endl;
    histFile.Write();

  } catch (exception &e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
    return -1;
  }

  return 0;
}

