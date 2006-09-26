// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/genMuonMPD.cxx,v 1.5 2006/09/18 14:12:53 fewtrell Exp $
/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "lib/MuonPed.h"
#include "lib/IntNonlin.h"
#include "lib/MuonAsym.h"
#include "lib/MuonMPD.h"
#include "lib/SimpleIniFile.h"
#include "lib/CalPed.h"
#include "lib/CalAsym.h"
#include "lib/CIDAC2ADC.h"
#include "lib/CalMPD.h"

// GLAST INCLUDES

// EXTLIB INCLUDES

// STD INCLUDES
#include <iostream>
#include <string>
#include <climits>
#include <fstream>

using namespace std;

const string usage_str("genMuonMPD.exe cfg_file");

int main(int argc, char **argv) {
  // libCalibGenCAL will throw runtime_error         
  try {
          
    //-- COMMAND LINE --//
    if (argc != 2) {
      cout << __FILE__ << ": Usage: " << usage_str << endl;
      return -1;
    }

    //-- CONFIG FILE --//
    SimpleIniFile cfgFile(argv[1]);

    // output dir
    string outputDir = cfgFile.getVal("GENERAL",
                                      "OUTPUT_DIR",
                                      string("./"));

    // input file(s)
    vector<string> rootFileList;
    cfgFile.getVector("MUON_MPD", 
                      "ROOT_FILES", 
                      rootFileList,
                      ", ");
    if (rootFileList.size() < 1) {
      cout << __FILE__ << ": No input files specified" << endl;
      return -1;
    }


    //-- SETUP LOG FILE --//
    /// multiplexing output streams 
    /// simultaneously to cout and to logfile
    CGCUtil::multiplexor_ostream logStrm;
    logStrm.getostreams().push_back(&cout);
    // generate logfile name
    string logfile;
    MuonMPD::genOutputFilename(outputDir,
                               rootFileList[0],
                               "log.txt",
                               logfile);
    ofstream tmpStrm(logfile.c_str());
    logStrm.getostreams().push_back(&tmpStrm);

    //-- LOG SOFTWARE VERSION INFO --//
    output_env_banner(logStrm);



    //-- RETRIEVE PEDESTALS
    // retrieve original input root filename for pedestal process
    // (in order to generate associated 'output' txt filename)
    vector<string> pedRootFileList;
    cfgFile.getVector("MUON_PEDS", 
                      "ROOT_FILES", 
                      pedRootFileList,
                      ", ");
    if (pedRootFileList.size() < 1) {
      logStrm << __FILE__ << ": No input files specified" << endl;
      return -1;
    }
    // txt output filename
    string pedTXTFile;
    MuonPed::genOutputFilename(outputDir,
                               pedRootFileList[0],
                               "txt",
                               pedTXTFile);

    CalPed peds;
    logStrm << __FILE__ << ": reading in muon pedestal file: " << pedTXTFile << endl;
    peds.readTXT(pedTXTFile);


  
    //-- RETRIEVE CIDAC2ADC
    // retrieve original input root filename for cidac2adc process
    // (in order to generate 'output' txt filename)
    string rootFileLE = cfgFile.getVal("CIDAC2ADC", 
                                       "LE_ROOT_FILE", 
                                       string(""));
    if (rootFileLE.length() < 1) {
      logStrm << __FILE__ << ": no LE root file specified" << endl;
      return -1;
    }
    // txt output filename
    string cidac2adcTXTFile;
    IntNonlin::genOutputFilename(outputDir,
                                 rootFileLE,
                                 "txt",
                                 cidac2adcTXTFile);

    CIDAC2ADC dac2adc;
    logStrm << __FILE__ << ": reading in cidac2adc txt file: " << cidac2adcTXTFile << endl;
    dac2adc.readTXT(cidac2adcTXTFile);
    logStrm << __FILE__ << ": generating cidac2adc splines: " << endl;
    dac2adc.genSplines();

    //-- RETRIEVE ASYM
    // retrieve original input root filename for asymmetry process
    // (in order to generate associated 'output' txt filename)
    vector<string> asymRootFileList;
    cfgFile.getVector("MUON_ASYM", 
                      "ROOT_FILES", 
                      asymRootFileList, 
                      ", ");
    if (asymRootFileList.size() < 1) {
      logStrm << __FILE__ << ": No input files specified" << endl;
      return -1;
    }
    // txt output filename
    string asymTXTFile;
    MuonAsym::genOutputFilename(outputDir,
                                asymRootFileList[0],
                                "txt",
                                asymTXTFile);

    CalAsym asym;
    logStrm << __FILE__ << ": reading in muon asym file: " << asymTXTFile << endl;
    asym.readTXT(asymTXTFile);
    logStrm << __FILE__ << ": building asymmetry splines: " << endl;
    asym.buildSplines();


    //-- MUON MPD

    // output histogram file name
    string histFilename;
    MuonMPD::genOutputFilename(outputDir,
                               rootFileList[0],
                               "root",
                               histFilename);

    // output txt file name
    string outputTXTFile;
    MuonMPD::genOutputFilename(outputDir,
                               rootFileList[0],
                               "txt",
                               outputTXTFile);

    unsigned nEntries = cfgFile.getVal<unsigned>("MUON_MPD", 
                                                 "ENTRIES_PER_HIST",
                                                 3000); 

    // read in Hist file?
    bool readInHists = cfgFile.getVal("MUON_MPD",
                                      "READ_IN_HISTS",
                                      false);

    MuonMPD muonMPD(logStrm);
    CalMPD calMPD;

    // used when creating histgrams
    auto_ptr<TFile> histFile;
    if (readInHists) { // skip event file processing & read in histograms directly
      logStrm << __FILE__ << ": opening input histogram file: " << histFilename<< endl;
      muonMPD.loadHists(histFilename);
    } else {
    
      // open file to save output histograms.
      logStrm << __FILE__ << ": opening output histogram file: " << histFilename<< endl;
      histFile.reset(new TFile(histFilename.c_str(), "RECREATE", "CAL Muon MPD", 9));
    
      logStrm << __FILE__ << ": reading root event file(s) starting w/ " << rootFileList[0] << endl;
      muonMPD.fillHists(nEntries,
                        rootFileList,
                        peds,
                        asym,
                        dac2adc);
    }
    
    logStrm << __FILE__ << ": fitting muon mpd histograms." << endl;
    muonMPD.fitHists(calMPD);

    logStrm << __FILE__ << ": writing muon mpd: " << outputTXTFile << endl;
    calMPD.writeTXT(outputTXTFile);

    string adc2nrgFile;
    MuonMPD::genOutputFilename(outputDir,
                               rootFileList[0],
                               "adc2nrg.txt",
                               adc2nrgFile);
  
    logStrm << __FILE__ << ": writing muon adc2nrg: " << adc2nrgFile << endl;
    muonMPD.writeADC2NRG(adc2nrgFile, asym, dac2adc, calMPD);
  
    if (!readInHists) {
      logStrm << __FILE__ << ": writing histogram file: " << histFilename << endl;
      histFile->Write();
      histFile->Close();
    }
  } catch (exception e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
  }

  return 0;
}
