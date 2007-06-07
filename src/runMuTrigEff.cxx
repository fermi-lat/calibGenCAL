// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/runMuTrigEff.cxx,v 1.34 2007/05/25 21:06:47 fewtrell Exp $

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "lib/CalibDataTypes/CalPed.h"
#include "lib/Algs/MuTrigAlg.h"
#include "lib/CalibDataTypes/CIDAC2ADC.h"
#include "lib/Util/SimpleIniFile.h"
#include "lib/Util/CGCUtil.h"

// GLAST INCLUDES

// EXTLIB INCLUDES

// STD INCLUDES
#include <iostream>
#include <string>
#include <climits>
#include <fstream>

using namespace std;
using namespace CGCUtil;
using namespace calibGenCAL;

int main(int argc,
         char **argv) {
  // libCalibGenCAL will throw runtime_error
  try {
    // input files
    string rootFileCI       = cfgFile.getVal("MU_TRIG",
                                             "ROOTFILE_CI",
                                             string(""));  // allow for run w/out CI info

    string rootFileEvenEven = cfgFile.getVal("MU_TRIG",
                                             "ROOTFILE_EVEN_EVEN",
                                             string(""));
    if (rootFileEvenEven.length() < 1) {
      cout << __FILE__ << ": no even root file specified" << endl;
      return -1;
    }
    string rootFileEvenOdd = cfgFile.getVal("MU_TRIG",
                                            "ROOTFILE_EVEN_ODD",
                                            string(""));
    if (rootFileEvenOdd.length() < 1) {
      cout << __FILE__ << ": no odd root file specified" << endl;
      return -1;
    }

    //-- SETUP LOG FILE --//
    /// multiplexing output streams
    /// simultaneously to cout and to logfile
    LogStrm::addStream(cout);
    // generate logfile name
    string logfile(cfg.outputBasename.getVal() + ".log.txt");
    ofstream tmpStrm(logfile.c_str());

    LogStrm::addStream(tmpStrm);

    //-- LOG SOFTWARE VERSION INFO --//
    output_env_banner(LogStrm::get());

    //-- RETRIEVE PEDESTALS
    // retrieve original input root filename for pedestal process
    // (in order to generate associated 'output' txt filename)
    vector<string> pedRootFileList(cfgFile. getVector<string>("MUON_PEDS",
                                                              "ROOT_FILES",
                                                              " ," ));
    if (pedRootFileList.size() < 1) {
      LogStrm::get() << __FILE__ << ": No input files specified" << endl;
      return -1;
    }

    CalPed peds;
    LogStrm::get() << __FILE__ << ": reading in muon pedestal file: " << pedTXTFile << endl;
    peds.readTXT(pedTXTFile);

    //-- CALCULATE TRIG EFFICIENCY --//
    // output histogram file name
    string histFilename(cfg.outputBasename.getVal() + ".root");
    TFile histFile(histFilename.c_str(),
                   "RECREATE",
                   "Cal MuTrig Analysis");

    MuTrigAlg muTrig;

    // reading charge injection file with diagnostic information (tack delay = 70)
    // input files
    if (rootFileCI.length() > 0)
      muTrig.fillCIHists(rootFileCI);

    // trigger configuration A :  Even Rows Even Columns
    unsigned nEvents      = cfgFile. getVal<unsigned>("MU_TRIG",
                                                      "N_EVENTS",
                                                      ULONG_MAX); // process all events by default.

    bool     calLOEnabled = cfgFile. getVal<bool>("MU_TRIG",
                                                  "CAL_LO_ENABLED",
                                                  false);

    muTrig.fillMuonHists(MuTrigAlg::EVEN_ROW_EVEN_COL,
                         rootFileEvenEven,
                         nEvents,
                         peds,
                         calLOEnabled);

    //  trigger configuration B :   Even Rows Odd Columns
    muTrig.fillMuonHists(MuTrigAlg::EVEN_ROW_ODD_COL,
                         rootFileEvenOdd,
                         nEvents,
                         peds,
                         calLOEnabled);

    muTrig.fitData(peds);

    // generate txt output name
    string outputTXTFile(cfg.outputBasename.getVal() + ".txt");
    muTrig.writeTXT(outputTXTFile);

    histFile.Write();
    histFile.Close();
  } catch (exception &e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
    return -1;
  }
  return 0;
}

