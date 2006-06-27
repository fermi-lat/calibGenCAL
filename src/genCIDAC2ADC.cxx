// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/genCIDAC2ADC.cxx,v 1.2 2006/06/22 21:50:22 fewtrell Exp $
/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "lib/CIDAC2ADC.h"
#include "lib/SimpleIniFile.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"

// EXTLIB INCLUDES

// STD INCLUDES
#include <iostream>
#include <string>
#include <fstream>

using namespace std;
using namespace CalUtil;

const string usage_str("genCIDAC2ADC.exe cfg_file");

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

    // input files
    string rootFileLE = cfgFile.getVal("CIDAC2ADC", 
                                       "LE_ROOT_FILE", string(""));
    if (rootFileLE.length() < 1) {
      cout << __FILE__ << ": no LE root file specified" << endl;
      return -1;
    }
    string rootFileHE = cfgFile.getVal("CIDAC2ADC", 
                                       "HE_ROOT_FILE", string(""));
    if (rootFileHE.length() < 1) {
      cout << __FILE__ << ": no HE root file specified" << endl;
      return -1;
    }


    //-- SETUP LOG FILE --//
    /// multiplexing output streams 
    /// simultaneously to cout and to logfile
    CGCUtil::multiplexor_ostream logStrm;
    logStrm.getostreams().push_back(&cout);
    // generate logfile name
    string logfile;
    CIDAC2ADC::genOutputFilename(outputDir,
                                 rootFileLE,
                                 "log.txt",
                                 logfile);
    ofstream tmpStrm(logfile.c_str());
    logStrm.getostreams().push_back(&tmpStrm);

    //-- LOG SOFTWARE VERSION INFO --//
    output_env_banner(logStrm);





    // broadcast mode
    bool bcastMode = cfgFile.getVal("CIDAC2ADC",
                                    "BCAST_MODE",
                                    true);

    // txt output filename
    string outputTXTFile;
    CIDAC2ADC::genOutputFilename(outputDir,
                                 rootFileLE,
                                 "txt",
                                 outputTXTFile);

    // output histogram file
    string outputHistFile;
    CIDAC2ADC::genOutputFilename(outputDir,
                                 rootFileLE,
                                 "root",
                                 outputHistFile);




    CIDAC2ADC cidac2adc(logStrm);

    bool readADCMeans = cfgFile.getVal("CIDAC2ADC",
                                       "READ_ADC_MEANS",
                                       false);
  
    string adcMeanFile;
    CIDAC2ADC::genOutputFilename(outputDir,
                                 rootFileLE,
                                 "adcmean.txt",
                                 adcMeanFile);
                               
    // open file to save output histograms.
    logStrm << __FILE__ << ": opening cidac2adc histogram file: " << outputHistFile << endl;
    TFile histFile(outputHistFile.c_str(), "RECREATE", "CAL CIDAC2ADC", 9);

    if (readADCMeans) {
      logStrm << __FILE__ << ": reading in adc means from previous event processing: "
              << adcMeanFile << endl;
      cidac2adc.readADCMeans(adcMeanFile);
    } else {
      logStrm << __FILE__ << ": reading LE calibGen event file: " << rootFileLE << endl;
      cidac2adc.readRootData(rootFileLE, LRG_DIODE, bcastMode);
      logStrm << __FILE__ << ": reading HE calibGen event file: " << rootFileHE << endl;
      cidac2adc.readRootData(rootFileHE, SM_DIODE,  bcastMode);
      logStrm << __FILE__ << ": saving adc means to txt file: " 
              << adcMeanFile << endl;
      cidac2adc.writeADCMeans(adcMeanFile);
    }


  
    logStrm << __FILE__ << ": generating smoothed spline points: " << rootFileHE << endl;
    cidac2adc.genSplinePts();
  
    cidac2adc.makeGraphs(histFile);

    logStrm << __FILE__ << ": writing smoothed spline points: " << outputTXTFile << endl;
    cidac2adc.writeTXT(outputTXTFile);
    histFile.Write();
  } catch (exception e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
  }
        
  return 0;
}
