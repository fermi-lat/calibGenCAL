#include <iostream>
#include <fstream>

#include "TSystem.h"

#include "muonCalib.h"
#include "mcCfg.h"

using namespace std;

int main(int argc, char** argv) {
  string cfgPath;
  if(argc > 1) cfgPath = argv[1];
  else cfgPath = "../src/muonCalib_option.xml";

  mcCfg cfg;
  if (cfg.readCfgFile(cfgPath) != 0) {
    cout << "Error reading config file: " << cfgPath << endl;
    return -1;
  }

  try {
    muonCalib appData(cfg.rootFileList, 
                      cfg.instrument, 
                      cfg.towerList, 
                      cfg.ostr, 
                      cfg.timestamp,
                      cfg.hitThresh,
                      cfg.cellHorPitch,
                      cfg.cellVertPitch,
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
      cfg.ostr << "Calculating MevPerDac - " << cfg.nEvtMPD << " events" << endl;
      
      // open new histogram file
      if (cfg.genHistfiles) appData.openHistFile(cfg.mpdHistFile);      
      
      // retrieve MevPerDac data from events.
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
    cfg.ostr << "muonCalib:  exception thrown: " << s << endl;
    return -1;
  }
    
  return 0;
}
