// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/genGCRHists.cxx,v 1.13 2007/05/25 21:06:46 fewtrell Exp $

/** @file
    @author Zachary Fewtrell

    Generate CIDAC & ADC signal gain calibration histograms from digi & gcrRecon root files
*/

// LOCAL INCLUDES
#include "lib/CalibDataTypes/CalPed.h"
#include "lib/CalibDataTypes/CIDAC2ADC.h"
#include "lib/CalibDataTypes/CalMPD.h"
#include "lib/Hists/GCRHists.h"
#include "lib/Algs/GCRCalibAlg.h"
#include "lib/Util/SimpleIniFile.h"
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
               "input pedestal TXT file",
               ""
               ),
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
    nEntries("nEntries",
             'n',
             "stop filling histograms when min nEntries= n",
             ULONG_MAX)
  {
    cmdParser.registerArg(pedTXTFile);
    cmdParser.registerArg(inlTXTFile);
    cmdParser.registerArg(digiFilenames);
    cmdParser.registerArg(gcrFilenames);
    cmdParser.registerArg(outputBasename);

    cmdParser.registerSwitch(help);
    cmdParser.registerSwitch(summaryMode);

    cmdParser.registerVar(cfgPath);
    cmdParser.registerVar(nEntries);

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

  CmdArg<string> gcrFilenames;

  CmdArg<string> outputBasename;

  CmdSwitch summaryMode;

  /// print usage string
  CmdSwitch help;

  CmdOptVar<string> cfgPath;

  /// stop filling histograms when min nEnties = n (defaulit = MAX_UNSIGNED_INT)
  CmdOptVar<unsigned> nEntries;

};

int main(const int argc,
         const char **argv) {
  // libCalibGenCAL will throw runtime_error
  try {
    //-- COMMAND LINE --//
    AppCfg  cfg(argc,
                argv);

    // input file(s)
    vector<string> digiRootFileList(getLinesFromFile(cfg.digiFilenames.getVal()));
    if (digiRootFileList.size() < 1) {
      cout << __FILE__ << ": No input files specified" << endl;
      return -1;
    }

    vector<string> gcrSelectRootFileList(getLinesFromFile(cfg.gcrFilenames.getVal()));    
    if (gcrSelectRootFileList.size() < 1) {
      cout << __FILE__ << ": No input files specified" << endl;
      return -1;
    }

    //-- SETUP LOG FILE --//
    /// multiplexing output streams
    /// simultaneously to cout and to logfile
    LogStrm::addStream(cout);
    // generate logfile name
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

    //-- RETRIEVE CIDAC2ADC
    CIDAC2ADC dac2adc;
    LogStrm::get() << __FILE__ << ": reading in cidac2adc txt file: " << cfg.inlTXTFile.getVal() << endl;
    dac2adc.readTXT(cfg.inlTXTFile.getVal());
    LogStrm::get() << __FILE__ << ": generating cidac2adc splines: " << endl;
    dac2adc.genSplines();

    //-- GCR MPD

    // output histogram file name
    string histFilename(cfg.outputBasename.getVal() + ".root");

    // open file to save output histograms.
    LogStrm::get() << __FILE__ << ": opening output histogram file: " << histFilename << endl;
    // used when creating histgrams
    TFile histFile(histFilename.c_str(), "RECREATE", "CAL GCR MPD");


    GCRHists  gcrHists(cfg.summaryMode.getVal(), &histFile);
    GCRCalibAlg gcrCalib(cfg.cfgPath.getVal());
    CalMPD calMPD;

    LogStrm::get() << __FILE__ << ": reading digiRoot event file(s) starting w/ " << digiRootFileList[0] << endl;
    gcrCalib.fillHists(cfg.nEntries.getVal(),
                       digiRootFileList,
                       gcrSelectRootFileList,
                       peds,
                       dac2adc,
                       gcrHists
                       );

    gcrHists.summarizeHists(LogStrm::get());

    LogStrm::get() << __FILE__ << ": writing histogram file: " << histFilename << endl;
    histFile.Write();
    histFile.Close();

    // output txt file name
    string outputTXTFile(cfg.outputBasename.getVal() + ".txt");


  } catch (exception &e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
    return -1;
  }

  return 0;
}

