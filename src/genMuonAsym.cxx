// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/genMuonAsym.cxx,v 1.21 2007/05/25 21:06:46 fewtrell Exp $

/** @file generate Light Asymmetry calibrations from Muon event filesusing Cal Digi Hodoscope
    for track & hit information

    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "lib/CalibDataTypes/CalPed.h"
#include "lib/CalibDataTypes/CIDAC2ADC.h"
#include "lib/CalibDataTypes/CalAsym.h"
#include "lib/Hists/AsymHists.h"
#include "lib/Algs/MuonAsymAlg.h"
#include "lib/Util/CGCUtil.h"
#include "lib/Util/CfgMgr.h"

// GLAST INCLUDES

// EXTLIB INCLUDES

// STD INCLUDES
#include <iostream>
#include <string>
#include <climits>
#include <fstream>

using namespace std;
using namespace calibGenCAL;
using namespace CGCUtil;
using namespace CfgMgr;

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
                  "text file w/ newline delimited list of input digi ROOT files (must match input gcr recon root files)",
                  ""),
    outputBasename("outputBasename",
                   "all output files will use this basename + some_ext",
                   ""),
    entriesPerHist("entriesPerHist",
                   'e',
                   "quit after all histograms have > n entries",
                   10000),
    help("help",
         'h',
         "print usage info")
  
  {

    cmdParser.registerArg(pedTXTFile);
    cmdParser.registerArg(inlTXTFile);
    cmdParser.registerArg(digiFilenames);
    cmdParser.registerArg(outputBasename);
    cmdParser.registerVar(entriesPerHist);
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
  CmdArg<string> outputBasename;
  CmdOptVar<unsigned> entriesPerHist;
  /// print usage string
  CmdSwitch help;

};

int main(int argc,
         const char **argv) {
  // libCalibGenCAL will throw runtime_error
  try {
    AppCfg cfg(argc, argv);

    // input file(s)
    vector<string> rootFileList(getLinesFromFile(cfg.digiFilenames.getVal()));
    if (rootFileList.size() < 1) {
      cout << __FILE__ << ": No input files specified" << endl;
      return -1;
    }

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

    //-- RETRIEVE PEDESTALS
    CalPed peds;
    LogStrm::get() << __FILE__ << ": reading in muon pedestal file: "
                     << cfg.pedTXTFile.getVal() << endl;
    peds.readTXT(cfg.pedTXTFile.getVal());

    //-- RETRIEVE CIDAC2ADC
    CIDAC2ADC dac2adc;
    LogStrm::get() << __FILE__ << ": reading in dac2adc txt file: "
                     << cfg.inlTXTFile.getVal() << endl;
    dac2adc.readTXT(cfg.inlTXTFile.getVal());

    LogStrm::get() << __FILE__ << ": generating dac2adc splines: " << endl;
    dac2adc.genSplines();

    //-- MUON ASYM
    // output histogram file name
    string histFilename(cfg.outputBasename.getVal() + ".root");

    // output txt file name
    string    outputTXTFile(cfg.outputBasename.getVal() + ".txt");

    AsymHists asymHists;
    MuonAsymAlg muonAsym(peds,
                         dac2adc,
                         asymHists);

    CalAsym   calAsym;

    // open file to save output histograms.
    LogStrm::get() << __FILE__ << ": opening output histogram file: "
                     << histFilename << endl;
    TFile histFile(histFilename.c_str(),
                   "RECREATE",
                   "CAL Muon Asymmetry");

    LogStrm::get() << __FILE__ << ": reading root event file(s) starting w/ "
                     << rootFileList[0] << endl;
    muonAsym.fillHists(cfg.entriesPerHist.getVal(),
                       rootFileList);
    asymHists.trimHists();

    // Save file to disk before entering fit portion (saves time if i crash during debugging).
    //histFile->Write();
    
    asymHists.summarizeHists(LogStrm::get());

    LogStrm::get() << __FILE__ << ": fitting muon asymmmetry histograms." << endl;
    asymHists.fitHists(calAsym);

    LogStrm::get() << __FILE__ << ": writing muon asymmetry: "
                     << outputTXTFile << endl;
    calAsym.writeTXT(outputTXTFile);
    LogStrm::get() << __FILE__ << ": writing histogram file: "
                     << histFilename << endl;
    histFile.Write();
  } catch (exception &e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
    return -1;
  }

  return 0;
}

