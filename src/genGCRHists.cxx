// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/genGCRHists.cxx,v 1.5 2007/03/28 17:48:37 fewtrell Exp $

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
#include "facilities/Util.h"

// EXTLIB INCLUDES

// STD INCLUDES
#include <iostream>
#include <string>
#include <climits>
#include <fstream>

using namespace std;
using namespace CGCUtil;
using namespace CfgMgr;
using namespace facilities;

class AppCfg {
public:
  AppCfg(const int argc,
         const char **argv) :
    cmdParser(path_remove_ext(__FILE__)),
    cfgPath("CfgPath",
            'c',
            "(optional) path to configuration file",
            "$(CALIBGENCALROOT)/cfg/defaults.cfg"),
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
    summaryMode("summaryMode",
                's',
                "generate summary histograms only (no individual channel hists)"
    )

  {
    cmdParser.registerArg(pedTXTFile);

    cmdParser.registerArg(inlTXTFile);

    cmdParser.registerArg(digiFilenames);

    cmdParser.registerArg(gcrFilenames);

    cmdParser.registerSwitch(summaryMode);

    cmdParser.registerVar(cfgPath);


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

  CmdOptVar<string> cfgPath;

  CmdArg<string> pedTXTFile;

  CmdArg<string> inlTXTFile;

  CmdArg<string> digiFilenames;

  CmdArg<string> gcrFilenames;

  CmdSwitch summaryMode;


};

int main(const int argc,
         const char **argv) {
  // libCalibGenCAL will throw runtime_error
  try {
    //-- COMMAND LINE --//
    AppCfg  cfg(argc,
            argv);

    //-- CONFIG FILE --//
	string fullCfgPath(cfg.cfgPath.getVal());
	Util::expandEnvVar(&fullCfgPath);
	SimpleIniFile  cfgFile(fullCfgPath);

    // input file(s)
    vector<string> digiRootFileList = getLinesFromFile(cfg.digiFilenames.getVal());
    if (digiRootFileList.size() < 1) {
      cout << __FILE__ << ": No input files specified" << endl;
      return -1;
    }

	vector<string> gcrSelectRootFileList = getLinesFromFile(cfg.gcrFilenames.getVal());    
    if (gcrSelectRootFileList.size() < 1) {
      cout << __FILE__ << ": No input files specified" << endl;
      return -1;
    }

    //-- SETUP LOG FILE --//
    /// multiplexing output streams
    /// simultaneously to cout and to logfile
    LogStream::addStream(cout);
    // generate logfile name
    const string outputDir("./");

    string logfile =
      CGCUtil::genOutputFilename(outputDir,
                                 "gcrCalib",
                                 digiRootFileList[0],
                                 "log.txt");
    ofstream tmpStrm(logfile.c_str());

    LogStream::addStream(tmpStrm);

    //-- LOG SOFTWARE VERSION INFO --//
    output_env_banner(LogStream::get());

    //-- RETRIEVE PEDESTALS
    CalPed    peds;
    LogStream::get() << __FILE__ << ": reading in muon pedestal file: " << cfg.pedTXTFile.getVal() << endl;
    peds.readTXT(cfg.pedTXTFile.getVal());

    //-- RETRIEVE CIDAC2ADC
    CIDAC2ADC dac2adc;
    LogStream::get() << __FILE__ << ": reading in cidac2adc txt file: " << cfg.inlTXTFile.getVal() << endl;
    dac2adc.readTXT(cfg.inlTXTFile.getVal());
    LogStream::get() << __FILE__ << ": generating cidac2adc splines: " << endl;
    dac2adc.genSplines();

    //-- GCR MPD

    // output histogram file name
    string histFilename =
      CalMPD::genFilename(outputDir,
                          digiRootFileList[0],
                          "root");

    // output txt file name
    string   outputTXTFile =
      CalMPD::genFilename(outputDir,
                          digiRootFileList[0],
                          "txt");

    unsigned nEntries = cfgFile. getVal<unsigned>("GCR_CALIB",
                                                  "ENTRIES_PER_HIST",
                                                  3000);

    GCRHists  gcrHists(cfg.summaryMode.getVal());

    GCRCalibAlg gcrCalib(&cfgFile);

    CalMPD calMPD;

    // used when creating histgrams
    auto_ptr<TFile> histFile;

    // open file to save output histograms.
    LogStream::get() << __FILE__ << ": opening output histogram file: " << histFilename<< endl;
    histFile.reset(new TFile(histFilename.c_str(), "RECREATE", "CAL GCR MPD", 9));

    LogStream::get() << __FILE__ << ": reading digiRoot event file(s) starting w/ " << digiRootFileList[0] << endl;
    gcrCalib.fillHists(nEntries,
                       digiRootFileList,
                       gcrSelectRootFileList,
                       peds,
                       dac2adc,
                       gcrHists
                       );
    gcrHists.trimHists();
    gcrHists.summarizeHists(LogStream::get());

    LogStream::get() << __FILE__ << ": writing histogram file: " << histFilename << endl;
    histFile->Write();
    histFile->Close();
  } catch (exception &e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
  }

  return 0;
}

