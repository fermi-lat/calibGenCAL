/** @file Generate MevPerDAC & Asym calibrations from Muon event files using
    Tracker recon (from svac) for track & hit information.

    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "lib/MuonCalibTkr.h"
#include "lib/SimpleIniFile.h"
#include "lib/CalPed.h"
#include "lib/CIDAC2ADC.h"
#include "lib/AsymHists.h"
#include "lib/MPDHists.h"
#include "lib/CalAsym.h"
#include "lib/CalMPD.h"
#include "lib/CGCUtil.h"
#include "lib/ADC2NRG.h"

// GLAST INCLUDES

// EXTLIB INCLUDES

// STD INCLUDES
#include <sstream>
#include <fstream>
#include <memory>

using namespace std;
using namespace CGCUtil;

const string usage_str("genMuonCalibTkr.exe cfg_file");

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

    // output dir
    string outputDir = cfgFile.getVal("GENERAL",
                                      "OUTPUT_DIR",
                                      string("./"));

    // input file(s)
    vector<string> digiFileList(cfgFile.getVector<string>("MUON_CALIB_TKR",
                                                          "DIGI_FILES",
                                                          ", "));
    if (digiFileList.size() < 1) {
      cout << __FILE__ << ": No input files specified" << endl;
      return -1;
    }

    vector<string> svacFileList(cfgFile.getVector<string>("MUON_CALIB_TKR",
                                                          "SVAC_FILES",
                                                          ", "));
    if (svacFileList.size() < 1) {
      cout << __FILE__ << ": No input files specified" << endl;
      return -1;
    }

    //-- SETUP LOG FILE --//
    /// multiplexing output streams
    /// simultaneously to cout and to logfile
    LogStream::addStream(cout);
    // generate logfile name
    string logfile =
      CGCUtil::genOutputFilename(outputDir,
                                 "muonCalib",
                                 digiFileList[0],
                                 "log.txt");
    ofstream tmpStrm(logfile.c_str());

    LogStream::addStream(tmpStrm);

    //-- LOG SOFTWARE VERSION INFO --//
    output_env_banner(LogStream::get());

    //-- RETRIEVE PEDESTALS
    // see if user has explictly specified input txt filename
    string pedTXTFile = cfgFile. getVal<string>("MUON_CALIB_TKR",
                                                "PED_TXT_FILE",
                                                "");

    if (!pedTXTFile.size()) {
      // retrieve original input root filename for pedestal process
      // (in order to generate associated 'output' txt filename)
      vector<string> pedRootFileList(cfgFile.getVector<string>("MUON_PEDS",
                                                               "ROOT_FILES",
                                                               ", "));
      if (pedRootFileList.size() < 1) {
        LogStream::get() << __FILE__ << ": No input files specified" << endl;
        return -1;
      }
      // txt output filename
      pedTXTFile =          CalPed::genFilename(outputDir,
                                                pedRootFileList[0],
                                                "txt");
    }

    CalPed peds;
    LogStream::get() << __FILE__ << ": reading in muon pedestal file: " << pedTXTFile << endl;
    peds.readTXT(pedTXTFile);

    //-- RETRIEVE CIDAC2ADC
    // see if user has explictly specified input txt filename
    string    dac2adcTXTFile = cfgFile. getVal<string>("MUON_CALIB_TKR",
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
    LogStream::get() << __FILE__ << ": reading in dac2adc txt file: " << dac2adcTXTFile << endl;
    dac2adc.readTXT(dac2adcTXTFile);

    LogStream::get() << __FILE__ << ": generating dac2adc splines: " << endl;
    dac2adc.genSplines();

    //-- MUON CALIB
    // output histogram file name
    string histFilename =
      CGCUtil::genOutputFilename(outputDir,
                                 "muonCalib",
                                 digiFileList[0],
                                 "root");

    // output txt file name
    string    asymTXTFile =
      CalAsym::genFilename(outputDir,
                           digiFileList[0],
                           "txt");

    string    mpdTXTFile  =
      CalMPD::genFilename(outputDir,
                          digiFileList[0],
                          "txt");

    unsigned  nEntries    = cfgFile. getVal<unsigned>("MUON_CALIB_TKR",
                                                      "ENTRIES_PER_HIST",
                                                      10000);

    // read in Hist file?
    bool      readInHists = cfgFile.getVal("MUON_CALIB_TKR",
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
        
    AsymHists asymHists;
    MPDHists  mpdHists(MPDHists::FitMethods::LANGAU);
    CalAsym   calAsym;
    CalMPD    calMPD;
    MuonCalibTkr tkrCalib(cfgFile,
                          peds,
                          dac2adc,
                          asymHists,
                          mpdHists);

    // used when creating histgrams
    if (readInHists) {
      // skip event file processing & read in histograms directly
      mpdHists.loadHists();
      asymHists.loadHists(*(histFile.get()));
    } else {
      LogStream::get() << __FILE__ << ": reading root event file(s) starting w/ "
                       << digiFileList[0] << endl;
      unsigned startEvent = cfgFile.getVal<unsigned>("MUON_CALIB_TKR",
                                                     "START_EVENT",
                                                     0);

      tkrCalib.fillHists(nEntries,
                         digiFileList,
                         svacFileList,
                         startEvent);
      mpdHists.trimHists();
      asymHists.trimHists();
      

      // Save file to disk before entering fit portion (saves time if i crash during debugging).
      //histFile->Write();
    }

    bool skipFitAsym = cfgFile.getVal<bool>("MUON_CALIB_TKR", "SKIP_ASYM_FIT", false);
    if (!skipFitAsym) {
      LogStream::get() << __FILE__ << ": fitting asymmetry histograms." << endl;
      asymHists.fitHists(calAsym);
    }

    bool skipFitMPD = cfgFile.getVal<bool>("MUON_CALIB_TKR", "SKIP_MPD_FIT", false);
    if (!skipFitMPD) {
      LogStream::get() << __FILE__ << ": fitting MeVPerDAC histograms." << endl;
      mpdHists.fitHists(calMPD);
    }

    LogStream::get() << __FILE__ << ": writing muon asymmetry: "
                     << asymTXTFile << endl;
    calAsym.writeTXT(asymTXTFile);

    LogStream::get() << __FILE__ << ": writing muon mevPerDAC: "
                     << mpdTXTFile << endl;
    calMPD.writeTXT(mpdTXTFile);

    LogStream::get() << __FILE__ << ": writing fit result tuple: "
                     << mpdTXTFile << endl;
    mpdHists.buildTuple();

    string adc2nrgFile =
      ADC2NRG::genFilename(outputDir,
                           digiFileList[0],
                           "txt");

    LogStream::get() << __FILE__ << ": writing muon adc2nrg: " << adc2nrgFile << endl;
    calAsym.genSplines();
    ADC2NRG::writeTXT(adc2nrgFile, calAsym, dac2adc, calMPD);

    LogStream::get() << __FILE__ << ": writing histogram file: "
                     << histFilename << endl;
    histFile->Write();
  } catch (exception e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
  }

  return 0;
}

