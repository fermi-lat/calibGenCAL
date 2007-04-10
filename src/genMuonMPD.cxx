// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/genMuonMPD.cxx,v 1.18 2007/04/10 14:51:01 fewtrell Exp $

/** @file Gen MevPerDAC calibrations from Muon event files using Cal Digi Hodoscope
    for track & hit information
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "lib/CalibDataTypes/CalPed.h"
#include "lib/CalibDataTypes/CalAsym.h"
#include "lib/CalibDataTypes/CIDAC2ADC.h"
#include "lib/CalibDataTypes/CalMPD.h"
#include "lib/Hists/MPDHists.h"
#include "lib/Algs/MuonMPDAlg.h"
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
                   3000)

  
  {

    cmdParser.registerArg(pedTXTFile);
    cmdParser.registerArg(inlTXTFile);
    cmdParser.registerArg(asymTXTFile);
    cmdParser.registerArg(digiFilenames);
    cmdParser.registerArg(outputBasename);
    cmdParser.registerVar(entriesPerHist);
        
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
  CmdArg<string> pedTXTFile;
  CmdArg<string> inlTXTFile;
  CmdArg<string> asymTXTFile;
  
  CmdArg<string> digiFilenames;

  CmdArg<string> outputBasename;

  CmdOptVar<unsigned> entriesPerHist;
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
    LogStream::addStream(cout);
    string logfile(cfg.outputBasename.getVal() + ".log.txt");
    ofstream tmpStrm(logfile.c_str());

    LogStream::addStream(tmpStrm);

    //-- LOG SOFTWARE VERSION INFO --//
    output_env_banner(LogStream::get());

    //-- RETRIEVE PEDESTALS
    CalPed    peds;
    LogStream::get() << __FILE__ << ": reading in muon pedestal file: " << cfg.pedTXTFile.getVal() << endl;
    peds.readTXT(cfg.pedTXTFile.getVal());

    // first see if use has explicitly chosen a txt filename
    CIDAC2ADC dac2adc;
    LogStream::get() << __FILE__ << ": reading in cidac2adc txt file: " << cfg.inlTXTFile.getVal() << endl;
    dac2adc.readTXT(cfg.inlTXTFile.getVal());
    LogStream::get() << __FILE__ << ": generating cidac2adc splines: " << endl;
    dac2adc.genSplines();

    //-- RETRIEVE ASYM
    CalAsym asym;
    LogStream::get() << __FILE__ << ": reading in muon asym file: " << cfg.asymTXTFile.getVal() << endl;
    asym.readTXT(cfg.asymTXTFile.getVal());
    LogStream::get() << __FILE__ << ": building asymmetry splines: " << endl;
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
    LogStream::get() << __FILE__ << ": opening output histogram file: "
                     << histFilename << endl;
    TFile histFile(histFilename.c_str(), "RECREATE", "CAL Muon Calib", 9);
    
    MPDHists mpdHists(MPDHists::FitMethods::LANDAU);
    MuonMPDAlg  muonMPD(peds,
                        dac2adc,
                        asym,
                        mpdHists);

    CalMPD calMPD;

    LogStream::get() << __FILE__ << ": reading root event file(s) starting w/ " << rootFileList[0] << endl;
    muonMPD.fillHists(cfg.entriesPerHist.getVal(),
                      rootFileList);
    mpdHists.trimHists();

    // Save file to disk before entering fit portion (saves time if i crash during debugging).
    //histFile->Write();
    
    LogStream::get() << __FILE__ << ": fitting muon mpd histograms." << endl;
    mpdHists.fitHists(calMPD);

    LogStream::get() << __FILE__ << ": writing muon mpd: " << outputTXTFile << endl;
    calMPD.writeTXT(outputTXTFile);

    string adc2nrgFile(cfg.outputBasename.getVal() + ".adc2nrg.txt");

    LogStream::get() << __FILE__ << ": writing histogram file: " << histFilename << endl;
    histFile.Write();

  } catch (exception &e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
    return -1;
  }

  return 0;
}

