// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/genMuonPed.cxx,v 1.7 2006/09/21 16:22:34 fewtrell Exp $
/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "lib/MuonPed.h"
#include "lib/SimpleIniFile.h"
#include "lib/CalPed.h"

// GLAST INCLUDES

// EXTLIB INCLUDES

// STD INCLUDES
#include <iostream>
#include <string>
#include <fstream>

using namespace std;

const string usage_str("genMuonPed.exe cfg_file");

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
    // input files
    vector<string> rootFileList;
    cfgFile.getVector("MUON_PEDS", 
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
    MuonPed::genOutputFilename(outputDir,
                               rootFileList[0],
                               "log.txt",
                               logfile);
    ofstream tmpStrm(logfile.c_str());
    logStrm.getostreams().push_back(&tmpStrm);

    //-- LOG SOFTWARE VERSION INFO --//
    output_env_banner(logStrm);


    //-- ROUGH PEDS --//
    // - LEX8 only include hits in histograms.
    MuonPed muonRoughPed(logStrm);
    CalPed roughPed;

    // txt output filename
    string roughPedTXTFile;
    CGCUtil::genOutputFilename(outputDir,
                               "roughPeds",
                               rootFileList[0],
                               "txt",
                               roughPedTXTFile);
    // output histogram file
    string roughPedHistFile;
    CGCUtil::genOutputFilename(outputDir,
                               "roughPeds",
                               rootFileList[0],
                               "root",
                               roughPedHistFile);


    // what type of cut?
    const string PASS_THROUGH_CUT("PASS_THROUGH");
    const string PERIODIC_TRIGGER_CUT("PERIODIC_TRIGGER");
    const string EXTERNAL_TRIGGER_CUT("EXTERNAL_TRIGGER");
    map<string,MuonPed::TRIGGER_CUT> trigCutMap;
    trigCutMap[PASS_THROUGH_CUT] = MuonPed::PASS_THROUGH;
    trigCutMap[PERIODIC_TRIGGER_CUT] = MuonPed::PERIODIC_TRIGGER;
    trigCutMap[EXTERNAL_TRIGGER_CUT] = MuonPed::EXTERNAL_TRIGGER;
    string trigCutStr = cfgFile.getVal<string>("MUON_PEDS",
                                               "TRIGGER_CUT",
                                               "PERIODIC_TRIGGER");

    if (trigCutMap.find(trigCutStr) == trigCutMap.end()) {
      logStrm << __FILE__ << ": ERROR! Invalid trigger_cut string: " << trigCutStr << endl;
      return -1;
    }
    MuonPed::TRIGGER_CUT trigCut = trigCutMap[trigCutStr];



    unsigned nEntries = cfgFile.getVal<unsigned>("MUON_PEDS", 
                                                 "ENTRIES_PER_HIST", 
                                                 1000);

    // read in Hist file?
    bool readInHists = cfgFile.getVal("MUON_PEDS",
                                      "READ_IN_HISTS",
                                      false);
    
    if (!readInHists) {
      // open new output histogram file
      logStrm << __FILE__ << ": opening output rough pedestal histogram file: " << roughPedHistFile << endl;
      TFile outputHistFile(roughPedHistFile.c_str(),
                           "RECREATE",
                           "Muon rough pedestals",
                           9);

    
      logStrm << __FILE__ << ": reading root event file(s) starting w/ " << rootFileList[0] << endl;
      muonRoughPed.fillHists(nEntries, 
                             rootFileList,
                             NULL,
                             trigCut);
      outputHistFile.Write();
    
      logStrm << __FILE__ << ": fitting rough pedestal histograms." << endl;
      muonRoughPed.fitHists(roughPed);
      logStrm << __FILE__ << ": writing rough pedestals: " << roughPedTXTFile << endl;
      roughPed.writeTXT(roughPedTXTFile);
    }


    //-- MUON PEDS --//
    // - LEX8 only include hits in histograms.
    MuonPed muPed(logStrm);
    CalPed calPed;
    // txt output filename
    string muPedTXTFile;
    MuonPed::genOutputFilename(outputDir,
                               rootFileList[0],
                               "txt",
                               muPedTXTFile);
    // output histogram file
    string muPedHistFile;
    MuonPed::genOutputFilename(outputDir,
                               rootFileList[0],
                               "root",
                               muPedHistFile);

    if (readInHists) {
      logStrm << __FILE__ << ": loading muon pedestal histograms: " << muPedHistFile << endl;
      muPed.loadHists(muPedHistFile);
      logStrm << __FILE__ << ": fitting muon pedestal histograms." << endl;
      muPed.fitHists(calPed);
    } else {
      // open new output histogram file
      logStrm << __FILE__ << ": opening muon pedestal output histogram file: " << muPedHistFile << endl;
      TFile outputHistFile(muPedHistFile.c_str(),
                           "RECREATE",
                           "Muon pedestals",
                           9);

      logStrm << __FILE__ << ": reading root event file(s) starting w/ " << rootFileList[0] << endl;
      muPed.fillHists(nEntries, 
                      rootFileList, 
                      &roughPed, 
                      trigCut);
      outputHistFile.Write();
    
      logStrm << __FILE__ << ": fitting muon pedestal histograms." << endl;
      muPed.fitHists(calPed);
    }

    logStrm << __FILE__ << ": writing muon pedestals: " << muPedTXTFile << endl;
    calPed.writeTXT(muPedTXTFile);
  } catch (exception e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
  }
  return 0;
}
