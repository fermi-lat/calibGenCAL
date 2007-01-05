// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/genCIDAC2ADC.cxx,v 1.9 2007/01/04 23:23:00 fewtrell Exp $

/** @file Gen CIDAC2ADC calibrations from singlex16 charge injection event files
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "lib/IntNonlin.h"
#include "lib/CIDAC2ADC.h"
#include "lib/SimpleIniFile.h"
#include "lib/CGCUtil.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"

// EXTLIB INCLUDES

// STD INCLUDES
#include <iostream>
#include <string>
#include <fstream>

using namespace std;
using namespace CalUtil;
using namespace CGCUtil;

const string usage_str("genCIDAC2ADC.exe cfg_file");

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
    string outputDir  = cfgFile.getVal("GENERAL",
                                       "OUTPUT_DIR",
                                       string("./"));

    // input files
    string rootFileLE = cfgFile.getVal("CIDAC2ADC",
                                       "LE_ROOT_FILE", string(""));
    string rootFileHE = cfgFile.getVal("CIDAC2ADC",
                                       "HE_ROOT_FILE", string(""));

    // i can process 1 or 2 files, but not none
    if (rootFileLE.length() == 0 && rootFileHE.length() == 0) {
      cout << __FILE__ << ": no input files specified." << endl;
      return -1;
    }

    const  string &outputBaseName = (rootFileLE.length()) ?
      rootFileLE : rootFileHE;

    //-- SETUP LOG FILE --//
    /// multiplexing output streams
    /// simultaneously to cout and to logfile
    LogStream::addStream(cout);
    // generate logfile name
    string logfile =
      CIDAC2ADC::genFilename(outputDir,
                             outputBaseName,
                             "log.txt");
    ofstream tmpStrm(logfile.c_str());

    LogStream::addStream(tmpStrm);

    //-- LOG SOFTWARE VERSION INFO --//
    output_env_banner(LogStream::get());

    // broadcast mode
    bool      bcastMode     = cfgFile.getVal("CIDAC2ADC",
                                             "BCAST_MODE",
                                             true);

    // txt output filename
    string    outputTXTFile =
      CIDAC2ADC::genFilename(outputDir,
                             outputBaseName,
                             "txt");

    CIDAC2ADC adcMeans;
    CIDAC2ADC cidac2adc;
    IntNonlin intNonlin;

    bool      readADCMeans = cfgFile.getVal("CIDAC2ADC",
                                            "READ_ADC_MEANS",
                                            false);

    string    adcMeanFile  =
      CIDAC2ADC::genFilename(outputDir,
                             outputBaseName,
                             "adcmean.txt");

    if (readADCMeans) {
      LogStream::get() << __FILE__ << ": reading in adc means from previous event processing: "
                       << adcMeanFile << endl;
      adcMeans.readTXT(adcMeanFile);
    } else {
      if (rootFileLE.length()) {
        LogStream::get() << __FILE__ << ": reading LE calibGen event file: " << rootFileLE << endl;
        intNonlin.readRootData(rootFileLE, adcMeans, LRG_DIODE, bcastMode);
      }

      if (rootFileHE.length()) {
        LogStream::get() << __FILE__ << ": reading HE calibGen event file: " << rootFileHE << endl;
        intNonlin.readRootData(rootFileHE, adcMeans, SM_DIODE,  bcastMode);
      }

      LogStream::get() << __FILE__ << ": saving adc means to txt file: "
                       << adcMeanFile << endl;
      adcMeans.writeTXT(adcMeanFile);
    }

    LogStream::get() << __FILE__ << ": generating smoothed spline points: " << rootFileHE << endl;
    intNonlin.genSplinePts(adcMeans, cidac2adc);

    LogStream::get() << __FILE__ << ": writing smoothed spline points: " << outputTXTFile << endl;
    cidac2adc.writeTXT(outputTXTFile);
  } catch (exception e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
  }

  return 0;
}

