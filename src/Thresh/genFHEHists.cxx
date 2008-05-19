// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/Thresh/genFHEHists.cxx,v 1.3 2008/05/13 16:54:00 fewtrell Exp $

/** @file
    @author Zachary Fewtrell

    fill Trigger threshold histograms from LPA histograms.
*/

// LOCAL INCLUDES
#include "LPAFheAlg.h"
#include "src/lib/Util/CfgMgr.h"
#include "src/lib/Util/CGCUtil.h"
#include "src/lib/Util/string_util.h"
#include "src/lib/Util/stl_util.h"

// GLAST INCLUDES
#include "CalUtil/SimpleCalCalib/CalPed.h"
#include "CalUtil/SimpleCalCalib/ADC2NRG.h"

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

/// Manage application configuration parameters
class AppCfg {
public:
  AppCfg(const int argc,
         const char **argv) :
    cmdParser(path_remove_ext(__FILE__)),
    triggerPattern("triggerPattern",
                   "'EREC' (even row (gcrc) even column) or 'EROC' (even row (gcrc) odd column",
                   ""),
    expectedThresh("expectedThresh",
                   "expected threshold in MeV",
                   1500),
    digiFilenames("digiFilenames",
                  "text file w/ newline delimited list of input digi ROOT files",
                  ""),
    pedFilename("pedFilename",
                "text file with pedestal calibration data",
                ""),
    adc2nrgFilename("muSlopeFilename",
                    "text file with muSlope (adc2mev) calibration data",
                    ""),
    outputBasename("outputBasename",
                   "all output files will use this basename + some_ext",
                   ""),
    entriesPerHist("entriesPerHist",
                   'e',
                   "quit after all histograms have > n entries",
                   1000),
    safetyMargin("safetyMargin",
                 's',
                 "select events with only 1 enabled crystal with signal above expectedThresh - safetyMargin (avoids misidentifying triggered crystals",
                 500),
    help("help",
         'h',
         "print usage info")
  {
    cmdParser.registerArg(triggerPattern);
    cmdParser.registerArg(expectedThresh);
    cmdParser.registerArg(digiFilenames);
    cmdParser.registerArg(pedFilename);
    cmdParser.registerArg(adc2nrgFilename);
    cmdParser.registerArg(outputBasename);
    cmdParser.registerVar(entriesPerHist);
    cmdParser.registerVar(safetyMargin);
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

  CmdArg<string> triggerPattern;

  CmdArg<float> expectedThresh;

  CmdArg<string> digiFilenames;

  CmdArg<string> pedFilename;

  CmdArg<string> adc2nrgFilename;
  
  CmdArg<string> outputBasename;

  CmdOptVar<unsigned> entriesPerHist;

  CmdOptVar<float> safetyMargin;

  /// print usage string
  CmdSwitch help;

};

int main(int argc,
         const char **argv) {
  // libCalibGenCAL will throw runtime_error
  try {
    AppCfg cfg(argc,argv);

    // input file(s)
    vector<string> digiFileList(getLinesFromFile(cfg.digiFilenames.getVal().c_str()));
    if (digiFileList.size() < 1) {
      cout << __FILE__ << ": No input files specified" << endl;
      return -1;
    }

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

    //-- ENUMERATE CMDLINE OPTIONS --//
    map<string, LPAFheAlg::TriggerPattern> trigPatternMap;
    trigPatternMap["EREC"] = LPAFheAlg::EvenRowEvenCol;
    trigPatternMap["EROC"] = LPAFheAlg::EvenRowOddCol;
    if (trigPatternMap.find(cfg.triggerPattern.getVal()) == trigPatternMap.end()) {
      LogStrm::get() << __FILE__ << ": ERROR! Invalid trigPattern string: "
                     << cfg.triggerPattern.getVal() << endl;
      return -1;
    }
    const LPAFheAlg::TriggerPattern trigPattern = trigPatternMap[cfg.triggerPattern.getVal()];

    /// load up previous calibrations
    CalPed calPed;
    LogStrm::get() << __FILE__ << ": calib file: " << cfg.pedFilename.getVal() << endl;
    calPed.readTXT(cfg.pedFilename.getVal());
    /// load up previous calibrations
    ADC2NRG adc2nrg;
    LogStrm::get() << __FILE__ << ": calib file: " << cfg.adc2nrgFilename.getVal() << endl;
    adc2nrg.readTXT(cfg.adc2nrgFilename.getVal());

    // open new output histogram file
    // output histogram file
    const string histFileName(cfg.outputBasename.getVal() 
                              + ".FHE"
                              + "." + cfg.triggerPattern.getVal() 
                              + ".root");
    LogStrm::get() << __FILE__ << ": opening output histogram file: " << histFileName << endl;
    TFile histfile(histFileName.c_str(),
                   "RECREATE",
                   (string("FHE")
                    + "_" + cfg.triggerPattern.getVal()
                    + "_hists").c_str());

    /// store alogrithm histograms
    TrigHists trigHists("trigHist",
                        &histfile, 0,
                        100,0,3000);
    TrigHists specHists("specHist",
                        &histfile, 0,
                        100,0,3000);;
    LPAFheAlg lpaFheAlg(trigPattern,
                        calPed,
                        adc2nrg,
                        specHists,
                        trigHists,
                        cfg.expectedThresh.getVal(),
                        cfg.safetyMargin.getVal());

    const unsigned nEntries(cfg.entriesPerHist.getVal());


    LogStrm::get() << __FILE__ << ": reading root event file(s) starting w/ " << digiFileList[0] << endl;

    //DEBUG
    lpaFheAlg.fillHists(nEntries,
                        digiFileList);

    LogStrm::get() << __FILE__ << "Writing output ROOT file." << endl;
    histfile.Write();
    histfile.Close();

    LogStrm::get() << __FILE__ << "Successfully completed." << endl;
  } catch (exception &e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
    return -1;
  }
  return 0;
}

