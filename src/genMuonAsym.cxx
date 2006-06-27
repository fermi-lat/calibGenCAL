// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/genMuonAsym.cxx,v 1.2 2006/06/22 21:50:22 fewtrell Exp $
/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "lib/MuonPed.h"
#include "lib/CIDAC2ADC.h"
#include "lib/MuonAsym.h"
#include "lib/SimpleIniFile.h"

// GLAST INCLUDES

// EXTLIB INCLUDES

// STD INCLUDES
#include <iostream>
#include <string>
#include <climits>
#include <fstream>
#include <memory>

using namespace std;

const string usage_str("genMuonAsym.exe cfg_file");

int main(int argc, char **argv) {
  // libCalibGenCAL will throw runtime_error         
  try {
          
    //-- COMMAND LINE --//
    if (argc != 2) {
      cout << __FILE__ << ": " << usage_str << endl;
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
    cfgFile.getVector("MUON_ASYM", 
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
    MuonAsym::genOutputFilename(outputDir,
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

    MuonPed peds(logStrm);
    logStrm << __FILE__ << ": reading in muon pedestal file: " << pedTXTFile << endl;
    peds.readTXT(pedTXTFile);


  
    //-- RETRIEVE CIDAC2ADC
    // retrieve original input root filename for cidac2adc process
    // (in order to generate 'output' txt filename)
    string dac2adcRootFile = cfgFile.getVal("CIDAC2ADC", 
                                            "LE_ROOT_FILE", 
                                            string(""));
    if (dac2adcRootFile.length() < 1) {
      logStrm << __FILE__ << ": no LE root file specified" << endl;
      return -1;
    }
    // txt output filename
    string dac2adcTXTFile;
    CIDAC2ADC::genOutputFilename(outputDir,
                                 dac2adcRootFile,
                                 "txt",
                                 dac2adcTXTFile);

    CIDAC2ADC dac2adc(logStrm);
    logStrm << __FILE__ << ": reading in dac2adc txt file: " << dac2adcTXTFile << endl;
    dac2adc.readTXT(dac2adcTXTFile);
    logStrm << __FILE__ << ": generating dac2adc splines: " << endl;
    dac2adc.genSplines();

    //-- MUON ASYM
    // output histogram file name
    string histFilename;
    MuonAsym::genOutputFilename(outputDir,
                                rootFileList[0],
                                "root",
                                histFilename);

    // output txt file name
    string outputTXTFile;
    MuonAsym::genOutputFilename(outputDir,
                                rootFileList[0],
                                "txt",
                                outputTXTFile);

    unsigned nEntries = cfgFile.getVal<unsigned>("MUON_ASYM", 
                                                 "ENTRIES_PER_HIST",
                                                 10000);

    // read in Hist file?
    bool readInHists = cfgFile.getVal("MUON_ASYM",
                                      "READ_IN_HISTS",
                                      false);

    MuonAsym asym(logStrm);

    // used when creating histgrams
    auto_ptr<TFile> histFile;
    if (readInHists) { // skip event file processing & read in histograms directly
      logStrm << __FILE__ << ": opening input histogram file: " << histFilename<< endl;
      asym.loadHists(histFilename);
    } else {
    
      // open file to save output histograms.
      logStrm << __FILE__ << ": opening output histogram file: " << histFilename<< endl;
      histFile.reset(new TFile(histFilename.c_str(), "RECREATE", "CAL Muon Asymmetry", 9));
    
      logStrm << __FILE__ << ": reading root event file(s) starting w/ " << rootFileList[0] << endl;
      asym.fillHists(nEntries,
                     rootFileList,
                     peds,
                     dac2adc);
    }

    asym.summarizeHists(logStrm);
    
    logStrm << __FILE__ << ": fitting muon asymmmetry histograms." << endl;
    asym.fitHists();

    logStrm << __FILE__ << ": writing muon asymmetry: " << outputTXTFile << endl;
    asym.writeTXT(outputTXTFile);
    if (!readInHists) {
      logStrm << __FILE__ << ": writing histogram file: " << histFilename << endl;
      histFile->Write();
    }
  } catch (exception e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
  }

  return 0;
}
