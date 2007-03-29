// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/genMuonAsym.cxx,v 1.15 2007/03/27 18:50:48 fewtrell Exp $

/** @file generate Light Asymmetry calibrations from Muon event filesusing Cal Digi Hodoscope
    for track & hit information

    @author Zachary Fewtrell
 */

// LOCAL INCLUDES
#include "lib/CalibDataTypes/CalPed.h"
#include "lib/CalibDataTypes/CIDAC2ADC.h"
#include "lib/CalibDataTypes/CalAsym.h"
#include "lib/Hists/AsymHists.h"
#include "lib/Algs/MuonAsymAlg.h"
#include "lib/Util/SimpleIniFile.h"
#include "lib/Util/CGCUtil.h"

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

const string usage_str("genMuonAsym.exe cfg_file");

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
    vector<string> rootFileList(cfgFile. getVector<string>("MUON_ASYM",
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
      CalAsym::genFilename(outputDir,
                           rootFileList[0],
                           "log.txt");
    ofstream tmpStrm(logfile.c_str());

    LogStream::addStream(tmpStrm);

    //-- LOG SOFTWARE VERSION INFO --//
    output_env_banner(LogStream::get());

    //-- RETRIEVE PEDESTALS
    // see if user has explictly specified input txt filename
    string pedTXTFile = cfgFile. getVal<string>("MUON_MPD",
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

    CalPed peds;
    LogStream::get() << __FILE__ << ": reading in muon pedestal file: "
                     << pedTXTFile << endl;
    peds.readTXT(pedTXTFile);

    //-- RETRIEVE CIDAC2ADC
    // see if user has explictly specified input txt filename
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
    LogStream::get() << __FILE__ << ": reading in dac2adc txt file: "
                     << dac2adcTXTFile << endl;
    dac2adc.readTXT(dac2adcTXTFile);

    LogStream::get() << __FILE__ << ": generating dac2adc splines: " << endl;
    dac2adc.genSplines();

    //-- MUON ASYM
    // output histogram file name
    string histFilename =
      CalAsym::genFilename(outputDir,
                           rootFileList[0],
                           "root");

    // output txt file name
    string    outputTXTFile =
      CalAsym::genFilename(outputDir,
                           rootFileList[0],
                           "txt");

    unsigned  nEntries      = cfgFile. getVal<unsigned>("MUON_ASYM",
                                                        "ENTRIES_PER_HIST",
                                                        10000);

    // read in Hist file?
    bool      readInHists   = cfgFile.getVal("MUON_ASYM",
                                             "READ_IN_HISTS",
                                             false);

    AsymHists asymHists;
    MuonAsymAlg muonAsym(peds,
                      dac2adc,
                      asymHists);

    CalAsym   calAsym;

    // used when creating histgrams
    auto_ptr<TFile> histFile;
    if (readInHists) {
      // skip event file processing & read in histograms directly
      LogStream::get() << __FILE__ << ": opening input histogram file: "
                       << histFilename << endl;
      histFile.reset(new TFile(histFilename.c_str(),
                               "UPDATE"));
      asymHists.loadHists(*(histFile.get()));
    } else {
      // open file to save output histograms.
      LogStream::get() << __FILE__ << ": opening output histogram file: "
                       << histFilename << endl;
      histFile.reset(new TFile(histFilename.c_str(),
                               "RECREATE",
                               "CAL Muon Asymmetry",
                               9));

      LogStream::get() << __FILE__ << ": reading root event file(s) starting w/ "
                       << rootFileList[0] << endl;
      muonAsym.fillHists(nEntries,
                         rootFileList);
      asymHists.trimHists();

      // Save file to disk before entering fit portion (saves time if i crash during debugging).
      //histFile->Write();
    }

    asymHists.summarizeHists(LogStream::get());

    LogStream::get() << __FILE__ << ": fitting muon asymmmetry histograms." << endl;
    asymHists.fitHists(calAsym);

    LogStream::get() << __FILE__ << ": writing muon asymmetry: "
                     << outputTXTFile << endl;
    calAsym.writeTXT(outputTXTFile);
    LogStream::get() << __FILE__ << ": writing histogram file: "
                     << histFilename << endl;
    histFile->Write();
  } catch (exception &e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
  }

  return 0;
}

