// $Header$
/** @file
    @author Zachary Fewtrell
 */

// LOCAL INCLUDES
#include "lib/MuonPed.h"
#include "lib/MuTrig.h"
#include "lib/CIDAC2ADC.h"
#include "lib/SimpleIniFile.h"

// GLAST INCLUDES

// EXTLIB INCLUDES

// STD INCLUDES
#include <iostream>
#include <string>
#include <climits>
#include <fstream>

const string usage_str("genMuonMPD.exe cfg_file");

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
  string rootFileCI = cfgFile.getVal("MU_TRIG", 
                                     "ROOTFILE_CI", string(""));
  if (rootFileCI.length() < 1) {
    cout << __FILE__ << ": no CI root file specified" << endl;
    return -1;
  }
  string rootFileEvenEven = cfgFile.getVal("MU_TRIG", 
                                           "ROOTFILE_EVEN_EVEN", string(""));
  if (rootFileEvenEven.length() < 1) {
    cout << __FILE__ << ": no even root file specified" << endl;
    return -1;
  }
  string rootFileEvenOdd = cfgFile.getVal("MU_TRIG", 
                                          "ROOTFILE_EVEN_ODD", string(""));
  if (rootFileEvenOdd.length() < 1) {
    cout << __FILE__ << ": no odd root file specified" << endl;
    return -1;
  }


  //-- SETUP LOG FILE --//
  /// multiplexing output streams 
  /// simultaneously to cout and to logfile
  CGCUtil::multiplexor_ostream logStrm;
  logStrm.getostreams().push_back(&cout);
  // generate logfile name
  string logfile;
  MuTrig::genOutputFilename(outputDir,
                            rootFileCI,
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
                    " ," );
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

  //-- CALCULATE TRIG EFFICIENCY --//
  // output histogram file name
  string histFilename;
  MuTrig::genOutputFilename(outputDir,
                            rootFileCI,
                            "root",
                            histFilename);
  TFile histFile(histFilename.c_str(), 
                 "RECREATE", 
                 "Cal MuTrig Analysis", 
                 9);

  MuTrig muTrig(logStrm);


  // reading charge injection file with diagnostic information (tack delay = 70)
  // input files
  muTrig.fillCIHists(rootFileCI);


  // trigger configuration A :  Even Rows Even Columns
  unsigned nEvents = cfgFile.getVal<unsigned>("MU_TRIG", 
                                              "N_EVENTS",
                                              ULONG_MAX); // process all events by default.
  bool calLOEnabled = cfgFile.getVal<bool>("MU_TRIG",
                                           "CAL_LO_ENABLED",
                                           false);

  muTrig.fillMuonHists(MuTrig::EVEN_ROW_EVEN_COL,
                       rootFileEvenEven,
                       nEvents,
                       peds,
                       calLOEnabled);


  //  trigger configuration B :   Even Rows Odd Columns
  muTrig.fillMuonHists(MuTrig::EVEN_ROW_ODD_COL,
                       rootFileEvenOdd,
                       nEvents,
                       peds,
                       calLOEnabled);

  muTrig.fitData(peds);

  // generate txt output name
  string outputTXTFile;
  MuTrig::genOutputFilename(outputDir,
                            rootFileCI,
                            "txt",
                            outputTXTFile);
  muTrig.writeTXT(outputTXTFile);

  histFile.Write();
  histFile.Close();
  

  return 0;
}
