// $Header$
/** @file
    @author Zachary Fewtrell
 */

// LOCAL INCLUDES
#include "lib/RoughPed.h"
#include "lib/MuonPed.h"
#include "lib/SimpleIniFile.h"

// GLAST INCLUDES

// EXTLIB INCLUDES

// STD INCLUDES
#include <iostream>
#include <string>
#include <fstream>

using namespace std;

const string usage_str("genMuonPed.exe cfg_file");

int main(int argc, char **argv) {
        
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
  // input files
  vector<string> muPedRootFileList;
  cfgFile.getVector("MUON_PEDS", 
                    "ROOT_FILES", 
                    muPedRootFileList,
                    ", ");
  if (muPedRootFileList.size() < 1) {
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
                             muPedRootFileList[0],
                             "log.txt",
                             logfile);
  ofstream tmpStrm(logfile.c_str());
  logStrm.getostreams().push_back(&tmpStrm);

  //-- LOG SOFTWARE VERSION INFO --//
  output_env_banner(logStrm);


  //-- ROUGH PEDS --//
  // - LEX8 only include hits in histograms.
  RoughPed roughPed(logStrm);
  // input files
  vector<string> roughPedRootFileList;
  cfgFile.getVector("ROUGH_PEDS", 
                    "ROOT_FILES", 
                    roughPedRootFileList, 
                    ", ");
  if (roughPedRootFileList.size() < 1) {
    cout << __FILE__ << ": No input files specified" << endl;
    return -1;
  }
  // read in txt file?
  bool readInTXTRoughPeds = cfgFile.getVal("ROUGH_PEDS",
                                           "READ_IN_TXT",
                                           false);
  // read in Hist file?
  bool readInHistsRoughPeds = cfgFile.getVal("ROUGH_PEDS",
                                             "READ_IN_HISTS",
                                             false);
  // txt output filename
  string roughPedTXTFile;
  RoughPed::genOutputFilename(outputDir,
                              roughPedRootFileList[0],
                              "txt",
                              roughPedTXTFile);
  // output histogram file
  string roughPedHistFile;
  RoughPed::genOutputFilename(outputDir,
                             roughPedRootFileList[0],
                             "root",
                             roughPedHistFile);

  if (readInTXTRoughPeds) {
    logStrm << "genMuonPed: reading in rough pedestals: " << roughPedTXTFile << endl;
    roughPed.readTXT(roughPedTXTFile);
  } else if (readInHistsRoughPeds) {
    logStrm << __FILE__ << ": loading rough pedestal histograms: " << roughPedHistFile << endl;
    roughPed.loadHists(roughPedHistFile);
    logStrm << __FILE__ << ": fitting rough pedestal histograms." << endl;
    roughPed.fitHists();
    logStrm << __FILE__ << ": writing rough pedestals: " << roughPedTXTFile << endl;
    roughPed.writeTXT(roughPedTXTFile);
  } else {
    unsigned nEventsRoughPeds = cfgFile.getVal<unsigned>("ROUGH_PEDS", 
                                                         "N_EVENTS", 
                                                         10000);
    
    // open new output histogram file
    logStrm << __FILE__ << ": opening output rough pedestal histogram file: " << roughPedHistFile << endl;
    TFile outputHistFile(roughPedHistFile.c_str(),
                         "RECREATE",
                         "Muon rough pedestals",
                         9);

    
    logStrm << __FILE__ << ": reading root event file(s) starting w/ " << roughPedRootFileList[0] << endl;
    roughPed.fillHists(nEventsRoughPeds, 
                       roughPedRootFileList);
    outputHistFile.Write();
    
    logStrm << __FILE__ << ": fitting rough pedestal histograms." << endl;
    roughPed.fitHists();
    logStrm << __FILE__ << ": writing rough pedestals: " << roughPedTXTFile << endl;
    roughPed.writeTXT(roughPedTXTFile);
  }


  //-- MUON PEDS --//
  // - LEX8 only include hits in histograms.
  MuonPed muPed(logStrm);
  // read in Hist file?
  bool readInHistsMuonPeds = cfgFile.getVal("MUON_PEDS",
                                            "READ_IN_HISTS",
                                            false);
  // txt output filename
  string muPedTXTFile;
  MuonPed::genOutputFilename(outputDir,
                             muPedRootFileList[0],
                             "txt",
                             muPedTXTFile);
  // output histogram file
  string muPedHistFile;
  MuonPed::genOutputFilename(outputDir,
                             muPedRootFileList[0],
                             "root",
                             muPedHistFile);

  if (readInHistsMuonPeds) {
    logStrm << __FILE__ << ": loading muon pedestal histograms: " << muPedHistFile << endl;
    muPed.loadHists(muPedHistFile);
    logStrm << __FILE__ << ": fitting muon pedestal histograms." << endl;
    muPed.fitHists();
  } else {
    unsigned nEventsMuonPeds = cfgFile.getVal<unsigned>("MUON_PEDS", 
                                                   "N_EVENTS", 
                                                   10000);

    // open new output histogram file
    logStrm << __FILE__ << ": opening muon pedestal output histogram file: " << muPedHistFile << endl;
    TFile outputHistFile(muPedHistFile.c_str(),
                         "RECREATE",
                         "Muon pedestals",
                         9);

    logStrm << __FILE__ << ": reading root event file(s) starting w/ " << muPedRootFileList[0] << endl;
    muPed.fillHists(nEventsMuonPeds, 
                      muPedRootFileList, 
                      roughPed);
    outputHistFile.Write();
    
    logStrm << __FILE__ << ": fitting muon pedestal histograms." << endl;
    muPed.fitHists();
  }

  logStrm << __FILE__ << ": writing muon pedestals: " << muPedTXTFile << endl;
  muPed.writeTXT(muPedTXTFile);
  return 0;
}
