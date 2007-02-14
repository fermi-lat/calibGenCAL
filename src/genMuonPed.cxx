// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/genMuonPed.cxx,v 1.12 2007/02/08 21:26:17 fewtrell Exp $

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "lib/MuonPed.h"
#include "lib/SimpleIniFile.h"
#include "lib/CalPed.h"
#include "lib/CGCUtil.h"

// GLAST INCLUDES

// EXTLIB INCLUDES

// STD INCLUDES
#include <iostream>
#include <string>
#include <fstream>

using namespace std;
using namespace CGCUtil;

const string usage_str("genMuonPed.exe cfg_file");

int main(int argc,
         char **argv) {
  // libCalibGenCAL will throw runtime_error
  try {
    //-- COMMAND LINE --//
    if (argc != 2) {
      cout << __FILE__ << ": Usage: " << usage_str << endl;
      return -1;
    }

    //-- CONFIG FILE --//
    SimpleIniFile cfgFile(argv[1]);

    // input files
    vector<string> rootFileList(cfgFile.getVector<string>("MUON_PEDS",
                                                          "ROOT_FILES",
                                                          ", "));
    if (rootFileList.size() < 1) {
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
      CalPed::genFilename(outputDir,
                          rootFileList[0],
                          "log.txt");
    ofstream tmpStrm(logfile.c_str());

    LogStream::addStream(tmpStrm);

    //-- LOG SOFTWARE VERSION INFO --//
    output_env_banner(LogStream::get());

    //-- ROUGH PEDS --//
    // - LEX8 only include hits in histograms.
    MuonPed muonRoughPed;
    CalPed  roughPed;

    // txt output filename
    string  roughPedTXTFile =
      CGCUtil::genOutputFilename(outputDir,
                                 "roughPeds",
                                 rootFileList[0],
                                 "txt");
    // output histogram file
    string roughPedHistFileName =
      CGCUtil::genOutputFilename(outputDir,
                                 "roughPeds",
                                 rootFileList[0],
                                 "root");

    // what type of cut?
    const string PASS_THROUGH_CUT("PASS_THROUGH");
    const string PERIODIC_TRIGGER_CUT("PERIODIC_TRIGGER");
    const string EXTERNAL_TRIGGER_CUT("EXTERNAL_TRIGGER");

    map<string, MuonPed::TRIGGER_CUT> trigCutMap;
    trigCutMap[PASS_THROUGH_CUT]     = MuonPed::PASS_THROUGH;
    trigCutMap[PERIODIC_TRIGGER_CUT] = MuonPed::PERIODIC_TRIGGER;
    trigCutMap[EXTERNAL_TRIGGER_CUT] = MuonPed::EXTERNAL_TRIGGER;
    string trigCutStr = cfgFile. getVal<string>("MUON_PEDS",
                                                "TRIGGER_CUT",
                                                "PERIODIC_TRIGGER");

    if (trigCutMap.find(trigCutStr) == trigCutMap.end()) {
      LogStream::get() << __FILE__ << ": ERROR! Invalid trigger_cut string: " << trigCutStr << endl;
      return -1;
    }
    MuonPed::TRIGGER_CUT trigCut     = trigCutMap[trigCutStr];

    unsigned nEntries    = cfgFile. getVal<unsigned>("MUON_PEDS",
                                                     "ENTRIES_PER_HIST",
                                                     1000);

    // read in Hist file?
    bool     readInHists = cfgFile.getVal("MUON_PEDS",
                                          "READ_IN_HISTS",
                                          false);

    if (!readInHists) {
      // open new output histogram file
      LogStream::get() << __FILE__ << ": opening output rough pedestal histogram file: " << roughPedHistFileName << endl;
      TFile outputHistFile(roughPedHistFileName.c_str(),
                           "RECREATE",
                           "Muon rough pedestals",
                           9);

      LogStream::get() << __FILE__ << ": reading root event file(s) starting w/ " << rootFileList[0] << endl;
      muonRoughPed.fillHists(nEntries,
                             rootFileList,
                             NULL,
                             trigCut);
      muonRoughPed.trimHists();
      outputHistFile.Write();

      LogStream::get() << __FILE__ << ": fitting rough pedestal histograms." << endl;
      muonRoughPed.fitHists(roughPed);
      LogStream::get() << __FILE__ << ": writing rough pedestals: " << roughPedTXTFile << endl;
      roughPed.writeTXT(roughPedTXTFile);
    }

    //-- MUON PEDS --//
    // - LEX8 only include hits in histograms.
    MuonPed  muPed;
    CalPed   calPed;
    // txt output filename
    string   muPedTXTFile  =
      CalPed::genFilename(outputDir,
                          rootFileList[0],
                          "txt");
    // output histogram file
    string   muPedHistFileName =
      CalPed::genFilename(outputDir,
                          rootFileList[0],
                          "root");

    if (readInHists) {
      TFile muPedHistFile(muPedHistFileName.c_str(),
                          "UPDATE");
      LogStream::get() << __FILE__ << ": loading muon pedestal histograms: " << muPedHistFileName << endl;
      muPed.loadHists(muPedHistFile);
      LogStream::get() << __FILE__ << ": fitting muon pedestal histograms." << endl;
      muPed.fitHists(calPed);
    } else {
      // open new output histogram file
      LogStream::get() << __FILE__ << ": opening muon pedestal output histogram file: " << muPedHistFileName << endl;
      TFile outputHistFile(muPedHistFileName.c_str(),
                           "RECREATE",
                           "Muon pedestals",
                           9);

      LogStream::get() << __FILE__ << ": reading root event file(s) starting w/ " << rootFileList[0] << endl;
      muPed.fillHists(nEntries,
                      rootFileList,
                      &roughPed,
                      trigCut);
      muPed.trimHists();
      outputHistFile.Write();

      LogStream::get() << __FILE__ << ": fitting muon pedestal histograms." << endl;
      muPed.fitHists(calPed);
    }

    LogStream::get() << __FILE__ << ": writing muon pedestals: " << muPedTXTFile << endl;
    calPed.writeTXT(muPedTXTFile);
  } catch (exception &e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
  }
  return 0;
}

