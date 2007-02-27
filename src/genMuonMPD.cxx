// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/genMuonMPD.cxx,v 1.15 2007/02/14 16:11:37 fewtrell Exp $

/** @file Gen MevPerDAC calibrations from Muon event files using Cal Digi Hodoscope
    for track & hit information
    @author Zachary Fewtrell
 */

// LOCAL INCLUDES
#include "lib/MuonMPD.h"
#include "lib/SimpleIniFile.h"
#include "lib/CalPed.h"
#include "lib/CalAsym.h"
#include "lib/CIDAC2ADC.h"
#include "lib/CalMPD.h"
#include "lib/CGCUtil.h"
#include "lib/MPDHists.h"
#include "lib/ADC2NRG.h"

// GLAST INCLUDES

// EXTLIB INCLUDES

// STD INCLUDES
#include <iostream>
#include <string>
#include <climits>
#include <fstream>
#include <memory>

using namespace std;
using namespace CGCUtil;

const string usage_str("genMuonMPD.exe cfg_file");

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

    // input file(s)
    vector<string> rootFileList(cfgFile. getVector<string>("MUON_MPD",
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
      CalMPD::genFilename(outputDir,
                          rootFileList[0],
                          "log.txt");
    ofstream tmpStrm(logfile.c_str());

    LogStream::addStream(tmpStrm);

    //-- LOG SOFTWARE VERSION INFO --//
    output_env_banner(LogStream::get());

    //-- RETRIEVE PEDESTALS
    // see if user has explictly specified input txt filename
    string    pedTXTFile = cfgFile. getVal<string>("MUON_MPD",
                                                   "PED_TXT_FILE",
                                                   "");

    if (!pedTXTFile.size()) {
      // retrieve original input root filename for pedestal process
      // (in order to generate associated 'output' txt filename)
      vector<string> pedRootFileList(cfgFile. getVector<string>("MUON_PEDS",
                                                                "ROOT_FILES",
                                                                ", "));
      if (pedRootFileList.size() < 1) {
        LogStream::get() << __FILE__ << ": No input files specified" << endl;
        return -1;
      }

      // txt output filename
      pedTXTFile = CalPed::genFilename(outputDir,
                                       pedRootFileList[0],
                                       "txt");
    }

    CalPed    peds;
    LogStream::get() << __FILE__ << ": reading in muon pedestal file: " << pedTXTFile << endl;
    peds.readTXT(pedTXTFile);

    // first see if use has explicitly chosen a txt filename
    string    dac2adcTXTFile = cfgFile. getVal<string>("MUON_MPD",
                                                       "INL_TXT_FILE",
                                                       "");

    if (!dac2adcTXTFile.size()) {
      // retrieve original input root filename for cidac2adc process
      // (in order to generate 'output' txt filename)
      string rootFileLE = cfgFile.getVal("CIDAC2ADC",
                                         "LE_ROOT_FILE",
                                         string(""));
      if (rootFileLE.length() < 1) {
        LogStream::get() << __FILE__ << ": no LE root file specified" << endl;
        return -1;
      }
      // txt output filename
      dac2adcTXTFile = CIDAC2ADC::genFilename(outputDir,
                                              rootFileLE,
                                              "txt");
    }

    CIDAC2ADC dac2adc;
    LogStream::get() << __FILE__ << ": reading in cidac2adc txt file: " << dac2adcTXTFile << endl;
    dac2adc.readTXT(dac2adcTXTFile);
    LogStream::get() << __FILE__ << ": generating cidac2adc splines: " << endl;
    dac2adc.genSplines();

    //-- RETRIEVE ASYM
    // see if user has explictly specified input txt filename
    string  asymTXTFile = cfgFile.getVal("MUON_MPD",
                                         "ASYM_TXT_FILE",
                                         string(""));

    if (!asymTXTFile.size()) {
      // retrieve original input root filename for asymmetry process
      // (in order to generate associated 'output' txt filename)
      vector<string> asymRootFileList(cfgFile. getVector<string>("MUON_ASYM",
                                                                 "ROOT_FILES",
                                                                 ", "));
      if (asymRootFileList.size() < 1) {
        LogStream::get() << __FILE__ << ": No input files specified" << endl;
        return -1;
      }

      // txt output filename
      asymTXTFile = CalAsym::genFilename(outputDir,
                                         asymRootFileList[0],
                                         "txt");
    }

    CalAsym asym;
    LogStream::get() << __FILE__ << ": reading in muon asym file: " << asymTXTFile << endl;
    asym.readTXT(asymTXTFile);
    LogStream::get() << __FILE__ << ": building asymmetry splines: " << endl;
    asym.genSplines();

    //-- MUON MPD

    // output histogram file name
    string histFilename =
      CalMPD::genFilename(outputDir,
                          rootFileList[0],
                          "root");

    // output txt file name
    string   outputTXTFile =
      CalMPD::genFilename(outputDir,
                          rootFileList[0],
                          "txt");

    unsigned nEntries      = cfgFile. getVal<unsigned>("MUON_MPD",
                                                       "ENTRIES_PER_HIST",
                                                       3000);

    // read in Hist file?
    bool     readInHists   = cfgFile.getVal("MUON_MPD",
                                            "READ_IN_HISTS",
                                            false);

    ///////////////////////////////////////
    //-- OPEN HISTOGRAM FILE             //
    //   (either 'CREATE' or 'UPDATE') --//
    ///////////////////////////////////////

    auto_ptr<TFile> histFile;
    if (readInHists) {
      LogStream::get() << __FILE__ << ": opening input histogram file: "
                       << histFilename << endl;
      histFile.reset(new TFile(histFilename.c_str(), "UPDATE"));
    } else {
      // open file to save output histograms.
      LogStream::get() << __FILE__ << ": opening output histogram file: "
                       << histFilename << endl;
      histFile.reset(new TFile(histFilename.c_str(), "RECREATE", "CAL Muon Calib", 9));
    }

    MPDHists mpdHists(MPDHists::FitMethods::LANDAU);
    MuonMPD  muonMPD(peds,
                     dac2adc,
                     asym,
                     mpdHists);

    CalMPD calMPD;

    if (readInHists)
      mpdHists.loadHists();
    else {
      LogStream::get() << __FILE__ << ": reading root event file(s) starting w/ " << rootFileList[0] << endl;
      muonMPD.fillHists(nEntries,
                        rootFileList);
      mpdHists.trimHists();

      // Save file to disk before entering fit portion (saves time if i crash during debugging).
      //histFile->Write();
    }

    LogStream::get() << __FILE__ << ": fitting muon mpd histograms." << endl;
    mpdHists.fitHists(calMPD);

    LogStream::get() << __FILE__ << ": writing muon mpd: " << outputTXTFile << endl;
    calMPD.writeTXT(outputTXTFile);

    string adc2nrgFile =
      ADC2NRG::genFilename(outputDir,
                           rootFileList[0],
                           "txt");

    LogStream::get() << __FILE__ << ": writing muon adc2nrg: " << adc2nrgFile << endl;
    ADC2NRG::writeTXT(adc2nrgFile, asym, dac2adc, calMPD);

    LogStream::get() << __FILE__ << ": writing histogram file: " << histFilename << endl;
    histFile->Write();
  } catch (exception &e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
    return -1;
  }

  return 0;
}

