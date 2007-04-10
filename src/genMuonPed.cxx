// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/genMuonPed.cxx,v 1.15 2007/03/27 18:50:48 fewtrell Exp $

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "lib/CalibDataTypes/CalPed.h"
#include "lib/Algs/MuonPedAlg.h"
#include "lib/Util/CGCUtil.h"
#include "lib/Util/CfgMgr.h"

// GLAST INCLUDES

// EXTLIB INCLUDES

// STD INCLUDES
#include <iostream>
#include <string>
#include <fstream>

using namespace std;
using namespace CGCUtil;
using namespace CfgMgr;

class AppCfg {
public:
  AppCfg(const int argc,
         const char **argv) :
    cmdParser(path_remove_ext(__FILE__)),
    triggerCut("triggerCut",
               't',
               "event filter 'ALL'|'PERIODIC'|'EXTERNAL' (default='PERIODIC')",
               "PERIODIC"),
    entriesPerHist("entriesPerHist",
                   'e',
                   "quit after all histograms have > n entries",
                   1000),
    digiFilenames("digiFilenames",
                  "text file w/ newline delimited list of input digi ROOT files",
                  ""
                  ),
    outputBasename("outputBasename",
                   "all output files will use this basename + some_ext",
                   "")

  {
    cmdParser.registerArg(digiFilenames);
    cmdParser.registerArg(outputBasename);
    cmdParser.registerVar(triggerCut);
    cmdParser.registerVar(entriesPerHist);

    try {
      cmdParser.parseCmdLine(argc, argv);
    } catch (exception &e) {
      cout << e.what() << endl;
      cmdParser.printUsage();
      throw e;
    }


  }

  /// construct new parser
  CmdLineParser cmdParser;

  CmdOptVar<string> triggerCut;

  CmdOptVar<unsigned> entriesPerHist;

  CmdArg<string> digiFilenames;
  
  CmdArg<string> outputBasename;
};

int main(int argc,
         const char **argv) {
  // libCalibGenCAL will throw runtime_error
  try {
    AppCfg cfg(argc,argv);

    // input file(s)
    vector<string> rootFileList(getLinesFromFile(cfg.digiFilenames.getVal()));
    if (rootFileList.size() < 1) {
      cout << __FILE__ << ": No input files specified" << endl;
      return -1;
    }

    //-- SETUP LOG FILE --//
    /// multiplexing output streams
    /// simultaneously to cout and to logfile
    LogStream::addStream(cout);

    // generate logfile name
    string logfile(cfg.outputBasename.getVal() + ".log.txt");
    ofstream tmpStrm(logfile.c_str());

    LogStream::addStream(tmpStrm);

    //-- LOG SOFTWARE VERSION INFO --//
    output_env_banner(LogStream::get());

    //-- ROUGH PEDS --//
    // - LEX8 only include hits in histograms.
    MuonPedAlg muonRoughPed;
    CalPed  roughPed;

    // txt output filename
    string  roughPedTXTFile(cfg.outputBasename.getVal() + ".roughPeds.txt");
    // output histogram file
    string roughPedHistFileName(cfg.outputBasename.getVal() + ".roughPeds.root");

    // what type of cut?
    const string PASS_THROUGH_CUT("ALL");
    const string PERIODIC_TRIGGER_CUT("PERIODIC");
    const string EXTERNAL_TRIGGER_CUT("EXTERNAL");

    map<string, MuonPedAlg::TRIGGER_CUT> trigCutMap;
    trigCutMap[PASS_THROUGH_CUT]     = MuonPedAlg::PASS_THROUGH;
    trigCutMap[PERIODIC_TRIGGER_CUT] = MuonPedAlg::PERIODIC_TRIGGER;
    trigCutMap[EXTERNAL_TRIGGER_CUT] = MuonPedAlg::EXTERNAL_TRIGGER;
    string trigCutStr(cfg.triggerCut.getVal());
    if (trigCutMap.find(trigCutStr) == trigCutMap.end()) {
      LogStream::get() << __FILE__ << ": ERROR! Invalid trigger_cut string: " 
                       << trigCutStr << endl;
      return -1;
    }
    MuonPedAlg::TRIGGER_CUT trigCut     = trigCutMap[trigCutStr];

    unsigned nEntries(cfg.entriesPerHist.getVal());

    // open new output histogram file
    LogStream::get() << __FILE__ << ": opening output rough pedestal histogram file: " << roughPedHistFileName <<
      endl;
    TFile outputHistFile(roughPedHistFileName.c_str(),
                         "RECREATE",
                         "Muon rough pedestals",
                         9);

    LogStream::get() << __FILE__ << ": reading root event file(s) starting w/ " << rootFileList[0] << endl;
    muonRoughPed.fillHists(nEntries,
                           rootFileList,
                           NULL,
                           trigCut);
    muonRoughPed.trimHists();
    outputHistFile.Write();
    
    LogStream::get() << __FILE__ << ": fitting rough pedestal histograms." << endl;
    muonRoughPed.fitHists(roughPed);
    LogStream::get() << __FILE__ << ": writing rough pedestals: " << roughPedTXTFile << endl;
    roughPed.writeTXT(roughPedTXTFile);

    //-- MUON PEDS --//
    // - LEX8 only include hits in histograms.
    MuonPedAlg  muPed;
    CalPed   calPed;
    // txt output filename
    string   muPedTXTFile(cfg.outputBasename.getVal() + ".calPed.txt");
    // output histogram file
    string   muPedHistFileName(cfg.outputBasename.getVal() + ".calPed.root");

    // open new output histogram file
    LogStream::get() << __FILE__ << ": opening muon pedestal output histogram file: " << muPedHistFileName << endl;
    LogStream::get() << __FILE__ << ": reading root event file(s) starting w/ " << rootFileList[0] << endl;
    muPed.fillHists(nEntries,
                    rootFileList,
                    &roughPed,
                    trigCut);
    muPed.trimHists();
    outputHistFile.Write();
    
    LogStream::get() << __FILE__ << ": fitting muon pedestal histograms." << endl;
    muPed.fitHists(calPed);
    
    LogStream::get() << __FILE__ << ": writing muon pedestals: " << muPedTXTFile << endl;
    calPed.writeTXT(muPedTXTFile);
  } catch (exception &e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
  }
  return 0;
}

