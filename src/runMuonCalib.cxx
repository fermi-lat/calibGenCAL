// LOCAL INCLUDES
#include "MuonCalib.h"
#include "McCfg.h"

// GLAST INCLUDES

// EXTLIB INCLUDES
#include "TSystem.h"

// STD INCLUDES
#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, char** argv) {
  string cfgPath;
  if(argc > 1) cfgPath = argv[1];
  else cfgPath = "../src/MuonCalib_option.xml";
    
  McCfg cfg;
  try {
    cfg.readCfgFile(cfgPath);

    // identify calibGenCAL package version
    cfg.ostr << "calibGenCAL CVS Tag: " << CGCUtil::CVS_TAG << endl;

    // insert quoted config file into log stream //
    { 
      string temp;
      ifstream cfgFile(cfgPath.c_str());
      cfg.ostr << "--- Begin cfg_file: " << cfgPath << " ---" << endl;
      while (cfgFile.good()) {
        getline(cfgFile, temp);
        if (cfgFile.fail()) continue; // bad get
        cfg.ostr << "> " << temp << endl;
      }
      cfg.ostr << "--- End " << cfgPath << " ---" << endl;
    }
  
    MuonCalib appData(cfg.rootFileList, 
                      cfg.instrument, 
                      cfg.towerList, 
                      cfg.ostr, 
                      cfg.timestamp,
                      cfg.hitThresh,
                      cfg.cellHorPitch,
                      cfg.cellVertPitch,
                      cfg.csiLength,
                      cfg.maxAsymLL,
                      cfg.maxAsymLS,
                      cfg.maxAsymSL,
                      cfg.maxAsymSS,
                      cfg.minAsymLL,
                      cfg.minAsymLS,
                      cfg.minAsymSL,
                      cfg.minAsymSS);
    
    ////////////////////////////////
    // *** PHASE 1: PEDESTALS *** //
    ////////////////////////////////

    if (!cfg.readInPeds) {
      // PASS 1 - Rough Peds
      cfg.ostr << "Calculating rough pedestals (LEX8) - " << cfg.nEvtRoughPed << " events" << endl;
      
      // open new histogram file
      if (cfg.genHistfiles) appData.openHistFile(cfg.pedHistFile);
      
      // retrieve rough pedestal data from events
      appData.fillRoughPedHists(cfg.nEvtRoughPed);
      
      // fit for rough pedestal results
      appData.fitRoughPedHists();

      // PASS 2 - Final Peds
      cfg.ostr << "Calculating 4 range pedestals - " << cfg.nEvtPed << " events" << endl;
      
      // rewind input file to beginning
      appData.rewind();
      
      // retrieve 4-range pedestal data from events
      appData.fillPedHists(cfg.nEvtPed);
      
      // fit for new pedestal results
      appData.fitPedHists();             
      
      // output pedestal TXT file
      if (cfg.genTXT) appData.writePedsTXT(cfg.pedFileTXT);
      
      // output pedestal XML file
      if (cfg.genXML) appData.writePedsXML(cfg.pedFileXML,cfg.dtdFile);
      
      // output pedestal histogram ROOT file
      if (cfg.genHistfiles) appData.flushHists();
    } else {
      // ALTERNATE SHORTCUT TO CALCULATING PEDS
      cfg.ostr << "Reading pedestals from " << cfg.pedFileTXT << endl;
      appData.readCalPeds(cfg.pedFileTXT);
    }

    // LOAD INTNONLIN
    cfg.ostr << "Reading integral nonlinearity from " << cfg.intNonlinFile << endl;
    appData.readIntNonlin(cfg.intNonlinFile);
    
    ///////////////////////////////
    // *** PHASE 2: ASYMMETRY *** //
    ///////////////////////////////

    if (!cfg.readInAsym) {
      // PASS 3 - Asymmetry
      cfg.ostr << "Calculating asymmetry - " << cfg.nEvtAsym << " events" << endl;
      
      // rewind input file
      appData.rewind();

      // open new histogram file
      if (cfg.genHistfiles) appData.openHistFile(cfg.asymHistFile);
      
      // retrieve asymmetry data from events
      appData.fillAsymHists(cfg.nEvtAsym, cfg.genOptAsymHists); // otherwise it's too many zeros
      
      // fit for asymmetry results
      appData.populateAsymArrays();

      // output asymmetry TXT file(s)
      if (cfg.genTXT) appData.writeAsymTXT(cfg.asymFileLLTXT,
                                           cfg.asymFileLSTXT,
                                           cfg.asymFileSLTXT,
                                           cfg.asymFileSSTXT);
      
      // output asymmetry XML file
      if (cfg.genXML) appData.writeAsymXML(cfg.asymFileXML,cfg.dtdFile);
      
      // save asymmetry histogram file.
      if (cfg.genHistfiles) appData.flushHists();
    } else {
      // ALTERNATE SHORTCUT TO CALCULATING ASYM
      cfg.ostr << "Reading asymmetry from " << cfg.asymFileLLTXT << " "
               << cfg.asymFileLSTXT << " "
               << cfg.asymFileSLTXT << " "
               << cfg.asymFileSSTXT << " "
               << endl;
      appData.readAsymTXT(cfg.asymFileLLTXT,
                          cfg.asymFileLSTXT,
                          cfg.asymFileSLTXT,
                          cfg.asymFileSSTXT);
    }

    //////////////////////////////////
    // *** PHASE 3: MEV_PER_DAC *** //
    //////////////////////////////////

    if (!cfg.skipMPD) {
      // rewind input file
      appData.rewind();
      cfg.ostr << "Calculating MevPerDAC - " << cfg.nEvtMPD << " events" << endl;
      
      // open new histogram file
      if (cfg.genHistfiles) appData.openHistFile(cfg.mpdHistFile);      
      
      // retrieve MevPerDAC data from events.
      appData.fillMPDHists(cfg.nEvtMPD); // otherwise it's too many zeros
      
      // fit for MPD results
      appData.fitMPDHists();
      
      // write MPD TXT files.
      if (cfg.genTXT) appData.writeMPDTXT(cfg.largeMPDFileTXT, cfg.smallMPDFileTXT);
      
      // write MPD XML file
      if (cfg.genXML) appData.writeMPDXML(cfg.mpdFileXML,cfg.dtdFile);
      
      // save MPD histograms
      if (cfg.genHistfiles) appData.flushHists();
    }
    
  } catch (string s) {
    // generic exception handler...  all my exceptions are simple C++ strings
    cfg.ostr << "MuonCalib:  exception thrown: " << s << endl;
    return -1;
  }
    
  return 0;
}
