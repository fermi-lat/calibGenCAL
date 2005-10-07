// LOCAL INCLUDES
#include "CfCfg.h"
#include "CfRoot.h"
#include "CfData.h"

// GLAST INCLUDES

// EXTLIB INCLUDES

// STD INCLUDES
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

/** @file runCIFit.cxx
    @author Zachary Fewtrell
    \brief Generate GLAST Calorimeter integral non-linearity (ADC->CIDAC) calibration from singlex16 event files.
 */

using namespace std;
using namespace CalDefs;

int main(int argc, char **argv) {
  // Load xml config file
  string cfgPath;
  if(argc > 1) cfgPath = argv[1];
  else cfgPath = "../src/ciFit_option.xml";

  CfCfg cfg;
  try {
    cfg.readCfgFile(cfgPath);

    // insert quoted config file into log stream //
    { 
      string tmp;
      ifstream cfgFile(cfgPath.c_str());
      cfg.ostrm << "--- Begin cfg_file: " << cfgPath << " ---" << endl;
      while (cfgFile.good()) {
        getline(cfgFile, tmp);
        if (cfgFile.fail()) continue; // bad get
        cfg.ostrm << "> " << tmp << endl;
      }
      cfg.ostrm << "--- End " << cfgPath << " ---" << endl << endl;
    }

    cfg.ostrm << endl;
    output_env_banner(cfg.ostrm);
    cfg.ostrm << endl;
    
    
    CfData cfData(cfg, cfg.outputHistPath);
    

    // LE PASS
    {
      vector<string> digiFilenames;
      digiFilenames.push_back(cfg.rootFileLE);
      CfRoot cfRoot(digiFilenames, cfData, cfg, LARGE_DIODE);  
      cfRoot.EvtLoop(cfg.nPulsesPerRun);
    }

    // HE PASS
    {
      vector<string> digiFilenames;
      digiFilenames.push_back(cfg.rootFileHE);
      CfRoot cfRoot(digiFilenames,cfData,cfg, SMALL_DIODE);  
      cfRoot.EvtLoop(cfg.nPulsesPerRun);
    }

    cfData.FitData();
    // data_high_thresh.FitData();
    // data.corr_FLE_Thresh(data_high_thresh);
    //data.ReadSplinesTXT("../output/ciSplines.txt");
    if (cfg.genTXT) cfData.WriteSplinesTXT(cfg.outputTXTPath);
    if (cfg.genXML) cfData.WriteSplinesXML(cfg.outputXMLPath, cfg.dtdPath);
    // generate optional plots of each spline
    if (cfg.genHistfile) cfData.makeGraphs();
  } catch (string s) {
    // generic exception handler...  all my exceptions are simple C++ strings
    cfg.ostrm << "ciFit:  exception thrown: " << s << endl;
    return -1;
  }

  return 0;
}

