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
  else cfgPath = "../src/muonCalib_option.xml";
    
  McCfg cfg;
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
      cfg.ostrm << "--- End " << cfgPath << " ---" << endl;
    }

    cfg.ostrm << endl;
    output_env_banner(cfg.ostrm);
    cfg.ostrm << endl;
    
    MuonCalib appData(cfg);
    
    ////////////////////////////////
    // *** PHASE 1: PEDESTALS *** //
    ////////////////////////////////

    if (!cfg.readInPeds) {
      // PASS 1 - Rough Peds
      cfg.ostrm << "Calculating rough pedestals (LEX8) - " << cfg.nEvtRoughPed << " events" << endl;
      
      // open new histogram file
      if (cfg.genHistfiles) appData.openHistFile(cfg.pedHistFile);
      
      // retrieve rough pedestal data from events
      appData.fillRoughPedHists(cfg.nEvtRoughPed);
      
      // fit for rough pedestal results
      appData.fitRoughPedHists();

      // PASS 2 - Final Peds
      cfg.ostrm << "Calculating 4 range pedestals - " << cfg.nEvtPed << " events" << endl;
      
      // rewind input file to beginning
      appData.rewind();
      
      // retrieve 4-range pedestal data from events
      appData.fillPedHists(cfg.nEvtPed);
      
      // fit for new pedestal results
      appData.fitPedHists();             
      
      // output pedestal TXT file
      if (cfg.genTXT) appData.writePedsTXT(cfg.pedFileTXT);
      
      // output pedestal XML file
      if (cfg.genXML) appData.writePedsXML(cfg.pedFileXML,cfg.dtdPath);
      
      // output pedestal histogram ROOT file
      if (cfg.genHistfiles) appData.flushHists();
    } else {
      // ALTERNATE SHORTCUT TO CALCULATING PEDS
      string file_to_read = (cfg.inputPedFile.length()) ?
        cfg.inputPedFile :
        cfg.pedFileTXT;
      cfg.ostrm << "Reading pedestals from " << file_to_read << endl;
      appData.readCalPeds(file_to_read);
    }

    if (cfg.pedsOnly) return 0;
    
    ///////////////////////////////
    // *** PHASE 2: ASYMMETRY *** //
    ///////////////////////////////

    // LOAD INTNONLIN (USED IN ASYM AND MEVPERDAC)
    cfg.ostrm << "Reading integral nonlinearity from " << cfg.intNonlinFile << endl;
    appData.readIntNonlin(cfg.intNonlinFile);
    

    if (!cfg.readInAsym) {
      // PASS 3 - Asymmetry
      cfg.ostrm << "Calculating asymmetry - " << cfg.nEvtAsym << " events" << endl;
      
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
      if (cfg.genXML) appData.writeAsymXML(cfg.asymFileXML,cfg.dtdPath);
      
      // save asymmetry histogram file.
      if (cfg.genHistfiles) appData.flushHists();
    } else {
      // ALTERNATE SHORTCUT TO CALCULATING ASYM
      cfg.ostrm << "Reading asymmetry from " << cfg.asymFileLLTXT << " "
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
      cfg.ostrm << "Calculating MevPerDAC - " << cfg.nEvtMPD << " events" << endl;
      
      // open new histogram file
      if (cfg.genHistfiles) appData.openHistFile(cfg.mpdHistFile);      
      
      // retrieve MevPerDAC data from events.
      appData.fillMPDHists(cfg.nEvtMPD); // otherwise it's too many zeros
      
      // fit for MPD results
      appData.fitMPDHists();
      
      // write MPD TXT files.
      if (cfg.genTXT) appData.writeMPDTXT(cfg.largeMPDFileTXT, cfg.smallMPDFileTXT);
      
      // write MPD XML file
      if (cfg.genXML) appData.writeMPDXML(cfg.mpdFileXML, cfg.dtdPath);

      //write adc2nrg XML file
      appData.writeADC2NRGXML(cfg.adc2nrgFileXML);
      
      // save MPD histograms
      if (cfg.genHistfiles) appData.flushHists();
    }
    
  } catch (string s) {
    // generic exception handler...  all my exceptions are simple C++ strings
    cfg.ostrm << "MuonCalib:  exception thrown: " << s << endl;
    return -1;
  }
    
  return 0;
}
