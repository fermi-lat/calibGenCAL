// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/genMuonMPD.cxx,v 1.25 2007/06/13 22:42:11 fewtrell Exp $

/** @file Gen MevPerDAC calibrations from Muon event files using Cal Digi Hodoscope
    for track & hit information
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "lib/Hists/MPDHists.h"
#include "lib/Algs/MuonMPDAlg.h"
#include "lib/Util/CfgMgr.h"
#include "lib/Util/CGCUtil.h"
#include "lib/Util/string_util.h"

// GLAST INCLUDES
#include "CalUtil/SimpleCalCalib/CalPed.h"
#include "CalUtil/SimpleCalCalib/CalAsym.h"
#include "CalUtil/SimpleCalCalib/CIDAC2ADC.h"
#include "CalUtil/SimpleCalCalib/CalMPD.h"

// EXTLIB INCLUDES
#include "TFile.h"

// STD INCLUDES
#include <iostream>
#include <string>
#include <fstream>

using namespace std;
using namespace calibGenCAL;
using namespace CfgMgr;
using namespace CalUtil;

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
    asymTXTFile("asymTXTFile",
                "input cal asym txt file",
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
                   3000),
    help("help",
         'h',
         "print usage info")
  {

    cmdParser.registerArg(pedTXTFile);
    cmdParser.registerArg(inlTXTFile);
    cmdParser.registerArg(asymTXTFile);
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
  CmdArg<string> asymTXTFile;
  
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
    CalPed    peds;
    LogStrm::get() << __FILE__ << ": reading in muon pedestal file: " << cfg.pedTXTFile.getVal() << endl;
    peds.readTXT(cfg.pedTXTFile.getVal());

    // first see if use has explicitly chosen a txt filename
    CIDAC2ADC dac2adc;
    LogStrm::get() << __FILE__ << ": reading in cidac2adc txt file: " << cfg.inlTXTFile.getVal() << endl;
    dac2adc.readTXT(cfg.inlTXTFile.getVal());
    LogStrm::get() << __FILE__ << ": generating cidac2adc splines: " << endl;
    dac2adc.genSplines();

    //-- RETRIEVE ASYM
    CalAsym asym;
    LogStrm::get() << __FILE__ << ": reading in muon asym file: " << cfg.asymTXTFile.getVal() << endl;
    asym.readTXT(cfg.asymTXTFile.getVal());
    LogStrm::get() << __FILE__ << ": building asymmetry splines: " << endl;
    asym.genSplines();

    //-- MUON MPD

    // output histogram file name
    string histFilename(cfg.outputBasename.getVal() + ".root");

    // output txt file name
    string   outputTXTFile(cfg.outputBasename.getVal() + ".txt");

    ///////////////////////////////////////
    //-- OPEN HISTOGRAM FILE             //
    //   (either 'CREATE' or 'UPDATE') --//
    ///////////////////////////////////////

    // open file to save output histograms.
    LogStrm::get() << __FILE__ << ": opening output histogram file: "
                     << histFilename << endl;
    TFile histFile(histFilename.c_str(), "RECREATE", "CAL Muon Calib");
    
    MPDHists mpdHists(MPDHists::FitMethods::LANDAU);
    MuonMPDAlg  muonMPD(peds,
                        dac2adc,
                        asym,
                        mpdHists);

    CalMPD calMPD;

    LogStrm::get() << __FILE__ << ": reading root event file(s) starting w/ " << rootFileList[0] << endl;
    muonMPD.fillHists(cfg.entriesPerHist.getVal(),
                      rootFileList);
    mpdHists.trimHists();

    // Save file to disk before entering fit portion (saves time if i crash during debugging).
    //histFile->Write();
    
    LogStrm::get() << __FILE__ << ": fitting muon mpd histograms." << endl;
    mpdHists.fitHists(calMPD);

    LogStrm::get() << __FILE__ << ": writing muon mpd: " << outputTXTFile << endl;
    calMPD.writeTXT(outputTXTFile);

    LogStrm::get() << __FILE__ << ": writing histogram file: " << histFilename << endl;
    histFile.Write();

  } catch (exception &e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
    return -1;
  }

  return 0;
}

