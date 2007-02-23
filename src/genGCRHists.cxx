// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/genGCRHists.cxx,v 1.5 2006/09/18 14:12:53 fewtrell Exp $
/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "lib/CalPed.h"
#include "lib/CIDAC2ADC.h"
#include "lib/GCRHists.h"
#include "lib/SimpleIniFile.h"
#include "lib/CGCUtil.h"
#include "lib/CalMPD.h"
#include "lib/CfgMgr.h"

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
     cfgPath("CfgPath",
             "path to configuration file",
             ""),
     pedTXTFile("pedTXTFile", 
                "input pedestal TXT file",
                ""
                ),
     inlTXTFile("inlTXTFIle", 
                "input CIDAC2ADC (intNonlin) file",
                ""
                ),
     summaryMode("summaryMode",
                 's',
                 "generate summary histograms only (no individual channel hists)"
                 )
  {

    cmdParser.registerArg(cfgPath);

    cmdParser.registerArg(pedTXTFile);
    
    cmdParser.registerArg(inlTXTFile);

    cmdParser.registerSwitch(summaryMode);

    cmdParser.parseCmdLine(argc, argv);
  }

  // construct new parser
  CmdLineParser cmdParser;

  CmdArg<string> cfgPath;

  CmdArg<string> pedTXTFile;

  CmdArg<string> inlTXTFile;

  CmdSwitch summaryMode;
};

int main(const int argc, 
         const char **argv) {
  // libCalibGenCAL will throw runtime_error         
  try {
          
    //-- COMMAND LINE --//
    AppCfg cfg(argc, argv);

    //-- CONFIG FILE --//
    SimpleIniFile cfgFile(cfg.cfgPath.getVal());

    // input file(s)
    vector<string> digiRootFileList= cfgFile.getVector<string>("GCR_CALIB", 
                                                               "DIGI_ROOT_FILES", 
                                                               ", ");
    if (digiRootFileList.size() < 1) {
      cout << __FILE__ << ": No input files specified" << endl;
      return -1;
    }

    vector<string> gcrSelectRootFileList = 
      cfgFile.getVector<string>("GCR_CALIB", 
                                "GCRSELECT_ROOT_FILES", 
                                ", ");
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
    CalPed peds;
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
    string histFilename=
      CalMPD::genFilename(outputDir,
                          digiRootFileList[0],
                          "root");

    // output txt file name
    string outputTXTFile=
      CalMPD::genFilename(outputDir,
                          digiRootFileList[0],
                          "txt");

    unsigned nEntries = cfgFile.getVal<unsigned>("GCR_CALIB", 
                                                 "ENTRIES_PER_HIST",
                                                 3000); 

    GCRHists gcrMPD(&cfgFile);
    CalMPD calMPD;

    // used when creating histgrams
    auto_ptr<TFile> histFile;

    // open file to save output histograms.
    LogStream::get() << __FILE__ << ": opening output histogram file: " << histFilename<< endl;
    histFile.reset(new TFile(histFilename.c_str(), "RECREATE", "CAL GCR MPD", 9));
    
    LogStream::get() << __FILE__ << ": reading digiRoot event file(s) starting w/ " << digiRootFileList[0] << endl;
    gcrMPD.fillHists(nEntries,
                     digiRootFileList,
                     gcrSelectRootFileList,
                     peds,
                     dac2adc);
    gcrMPD.trimHists();
    gcrMPD.summarizeHists(LogStream::get());
    
    LogStream::get() << __FILE__ << ": writing histogram file: " << histFilename << endl;
    histFile->Write();
    histFile->Close();
  } catch (exception &e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
  }

  return 0;
}
