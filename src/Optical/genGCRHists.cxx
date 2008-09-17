// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/Optical/genGCRHists.cxx,v 1.5 2008/07/29 20:03:26 fewtrell Exp $

/** @file
    @author Zachary Fewtrell

    Generate CIDAC & ADC signal gain calibration histograms from digi & gcrRecon root files
*/

// LOCAL INCLUDES
#include "src/lib/Hists/GCRHists.h"
#include "src/lib/Hists/AsymHists.h"
#include "src/lib/Hists/PedHists.h"
#include "GCRCalibAlg.h"
#include "src/lib/Util/SimpleIniFile.h"
#include "src/lib/Util/CfgMgr.h"
#include "src/lib/Util/CGCUtil.h"
#include "src/lib/Util/stl_util.h"
#include "src/lib/Util/string_util.h"
#include "src/Ped/MuonPedAlg.h"

// GLAST INCLUDES
#include "CalUtil/SimpleCalCalib/CalPed.h"
#include "CalUtil/SimpleCalCalib/CIDAC2ADC.h"
#include "CalUtil/SimpleCalCalib/CalMPD.h"

// EXTLIB INCLUDES
#include "TFile.h"

// STD INCLUDES
#include <iostream>
#include <string>
#include <climits>
#include <fstream>

using namespace std;
using namespace calibGenCAL;
using namespace CfgMgr;
using namespace CalUtil;

namespace {
  /// minimum number of entries for rough ped histograms
  static const unsigned short ROUGHPED_MIN_ENTRIES = 1000;
  /// desired minimum number of entries for pedestal histograms
  static const unsigned short PED_DESIRED_MIN_ENTRIES = 10000;
  /// absolute min n entries for ped histograms
  static const unsigned short PED_MIN_ENTRIES = 2000;
}

/// Manage application configuration parameters
class AppCfg {
public:
  AppCfg(const int argc,
         const char **argv) :
    cmdParser(path_remove_ext(__FILE__)),
    inlTXTFile("inlTXTFIle",
               "input CIDAC2ADC (intNonlin) file",
               ""
               ),
    digiFilenames("digiFilenames",
                  "text file w/ newline delimited list of input digi ROOT files (must match input gcr recon root files)",
                  ""),
    gcrFilenames("gcrFilenames",
                 "text file w/ newline delimited list of input gcr recon ROOT files (must match input DIGI root files)",
                 ""),
    outputBasename("outputBasename",
                   "all output files will use this basename + some_ext",
                   ""),
    summaryMode("summaryMode",
                's',
                "generate summary histograms only (no individual channel hists)"
                ),
    help("help",
         'h',
         "print usage info"),
    cfgPath("cfgPath",
            'c',
            "(optional) path to configuration file",
            ""),
    inputMPDTXTFile("inputMPDTXTFile",
                    'm',
                    "Optional input mevPerDAC file - enables histograms in energy scale",
                    "")
  {
    cmdParser.registerArg(inlTXTFile);
    cmdParser.registerArg(digiFilenames);
    cmdParser.registerArg(gcrFilenames);
    cmdParser.registerArg(outputBasename);

    cmdParser.registerSwitch(help);
    cmdParser.registerSwitch(summaryMode);

    cmdParser.registerVar(cfgPath);
    cmdParser.registerVar(inputMPDTXTFile);

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

  CmdArg<string> inlTXTFile;

  CmdArg<string> digiFilenames;

  CmdArg<string> gcrFilenames;

  CmdArg<string> outputBasename;

  CmdSwitch summaryMode;

  /// print usage string
  CmdSwitch help;

  CmdOptVar<string> cfgPath;

  CmdOptVar<string> inputMPDTXTFile;

};

int main(const int argc,
         const char **argv) {
  // libCalibGenCAL will throw runtime_error
  try {
    //-- COMMAND LINE --//
    AppCfg  cfg(argc,
                argv);

    // input file(s)
    const vector<string> digiFileList(getLinesFromFile(cfg.digiFilenames.getVal().c_str()));
    if (digiFileList.size() < 1) {
      cout << __FILE__ << ": No input files specified" << endl;
      return -1;
    }

    const vector<string> gcrFileList(getLinesFromFile(cfg.gcrFilenames.getVal().c_str()));
    if (gcrFileList.size() < 1) {
      cout << __FILE__ << ": No input files specified" << endl;
      return -1;
    }

    //-- SETUP LOG FILE --//
    /// multiplexing output streams
    /// simultaneously to cout and to logfile
    LogStrm::addStream(cout);
    // generate logfile name
    const string logfile(cfg.outputBasename.getVal() + ".gcr_hist.log.txt");
    ofstream tmpStrm(logfile.c_str());

    LogStrm::addStream(tmpStrm);

    //-- LOG SOFTWARE VERSION INFO --//
    output_env_banner(LogStrm::get());
    LogStrm::get() << endl;
    cfg.cmdParser.printStatus(LogStrm::get());
    LogStrm::get() << endl;

    //-- RETRIEVE CIDAC2ADC
    CIDAC2ADC dac2adc;
    LogStrm::get() << __FILE__ << ": reading in cidac2adc txt file: " << cfg.inlTXTFile.getVal() << endl;
    dac2adc.readTXT(cfg.inlTXTFile.getVal());
    LogStrm::get() << __FILE__ << ": generating cidac2adc splines: " << endl;
    dac2adc.genSplines();

    //-- GCR MPD

    // output histogram file name
    const string mpdHistFilename(cfg.outputBasename.getVal() + ".gcr_mpd_hist.root");
    const string asymHistFilename(cfg.outputBasename.getVal() + ".gcr_asym_hist.root");

    /// pedestal histograms will be re-used on each input file, create them outside of loop
    PedHists pedHists(gDirectory);

    // open file to save output histograms.
    LogStrm::get() << __FILE__ << ": opening output histogram file: " << mpdHistFilename << endl;
    // used when creating histgrams
    TFile mpdHistFile(mpdHistFilename.c_str(), "RECREATE", "CAL GCR MPD");

    // open file to save output histograms.
    LogStrm::get() << __FILE__ << ": opening output histogram file: " << asymHistFilename << endl;
    // used when creating histgrams
    TFile asymHistFile(asymHistFilename.c_str(), "RECREATE", "CAL GCR ASYM");


    GCRHists  gcrHists(cfg.summaryMode.getVal(), 
                       cfg.inputMPDTXTFile.getVal() != "",
                       &mpdHistFile);
    AsymHists asymHists(CalResponse::FLIGHT_GAIN,
                        12,
                        10,
                        &asymHistFile);
    GCRCalibAlg gcrCalib(cfg.cfgPath.getVal(), cfg.inputMPDTXTFile.getVal());
    CalMPD calMPD;

    // INPUT FILE LOOP
    vector<string>::const_iterator digiFileIt = digiFileList.begin();
    vector<string>::const_iterator gcrFileIt = gcrFileList.begin();

    for (; digiFileIt != digiFileList.end() && gcrFileIt != gcrFileList.end();
         digiFileIt++, gcrFileIt++) {
      // create filename 'list' of length 1
      const vector<string> curDigiFileList(1,*digiFileIt);
      const vector<string> curGcrFileList(1,*gcrFileIt);

      //-- DETERMINE PEDESTALS FROM DIGI FILE. --//
      // ped step 1: rough pedestals
      MuonPedAlg roughPedAlg;
      CalPed roughPed;

      LogStrm::get() << __FILE__ << ": filling rough pedestal histograms:" << *digiFileIt << endl;
      /// zero out pedestal histograms
      pedHists.resetHists();
      roughPedAlg.fillHists(ROUGHPED_MIN_ENTRIES,
                            curDigiFileList,
                            NULL,
                            pedHists,
                            MuonPedAlg::PERIODIC_TRIGGER);
      pedHists.trimHists();
      LogStrm::get() << __FILE__ << ": fitting rough pedestal histograms." << *digiFileIt << endl;
      pedHists.fitHists(roughPed);

      // ped step 2: rough pedestals
      MuonPedAlg pedAlg;
      CalPed ped;
      LogStrm::get() << __FILE__ << ": filling final pedestal histograms:" << *digiFileIt << endl;
      /// zero out pedestal histograms
      pedHists.resetHists();
      pedAlg.fillHists(PED_DESIRED_MIN_ENTRIES,
                       curDigiFileList,
                       &roughPed,
                       pedHists,
                       MuonPedAlg::PERIODIC_TRIGGER);
      pedHists.trimHists();
      if (pedHists.getMinEntries() < PED_MIN_ENTRIES) {
        LogStrm::get() << __FILE__ << ": digiFile " << *digiFileIt << "contains less than "
                       << PED_MIN_ENTRIES << " periodic triggers, skipping file. " << endl;
        continue;
      }

      LogStrm::get() << __FILE__ << ": fitting final pedestal histograms." << *digiFileIt << endl;
      pedHists.fitHists(ped);
      const string pedFileName = path_remove_dir(*digiFileIt + ".calPed.txt");
      LogStrm::get() << __FILE__ << ": writing pedestals to txt:" << pedFileName << endl;
      ped.writeTXT(pedFileName);

      gcrCalib.fillHists(UINT_MAX,
                         curDigiFileList,
                         curGcrFileList,
                         ped,
                         dac2adc,
                         gcrHists,
                         asymHists
                         );
    }
           

    gcrHists.summarizeHists(LogStrm::get());

    LogStrm::get() << __FILE__ << ": writing histogram file: " << mpdHistFilename << endl;
    mpdHistFile.Write();
    mpdHistFile.Close();

    LogStrm::get() << __FILE__ << ": writing histogram file: " << asymHistFilename << endl;
    asymHistFile.Write();
    asymHistFile.Close();

    // output txt file name
    const string outputTXTFile(cfg.outputBasename.getVal() + ".txt");

    LogStrm::get() << __FILE__ << ": Successfully completed." << endl;
  } catch (exception &e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
    return -1;
  }

  return 0;
}

