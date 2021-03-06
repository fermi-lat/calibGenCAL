// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/Ped/genMuonPed.cxx,v 1.5 2008/09/17 20:32:45 fewtrell Exp $

/** @file
    @author Zachary Fewtrell

    extract Cal pedestals from LPA data (periodic trigger, external trigger, or non zero suppressed particle data)
*/

// LOCAL INCLUDES
#include "src/lib/Algs/MuonPedAlg.h"
#include "src/lib/Util/CfgMgr.h"
#include "src/lib/Util/CGCUtil.h"
#include "src/lib/Util/string_util.h"
#include "src/lib/Util/stl_util.h"
#include "src/lib/Hists/PedHists.h"

// GLAST INCLUDES
#include "CalUtil/SimpleCalCalib/CalPed.h"

// EXTLIB INCLUDES
#include "TFile.h"

// STD INCLUDES
#include <iostream>
#include <string>
#include <fstream>

using namespace std;
using namespace calibGenCAL;
using namespace CfgMgr;
using namespace CalUtil;

/// Manage application configuration parameters
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
                   ""),
    help("help",
         'h',
         "print usage info")
  {
    cmdParser.registerArg(digiFilenames);
    cmdParser.registerArg(outputBasename);
    cmdParser.registerVar(triggerCut);
    cmdParser.registerVar(entriesPerHist);
    cmdParser.registerSwitch(help);


    try {
      cmdParser.parseCmdLine(argc, argv);
    } catch (exception &e) {
      // ignore invalid commandline if user asked for help.
      if (!help.getVal())
        cout << e.what() << endl;
      cmdParser.printUsage();
      exit(-1);
    }


  }

  /// construct new parser
  CmdLineParser cmdParser;

  CmdOptVar<string> triggerCut;

  CmdOptVar<unsigned> entriesPerHist;

  CmdArg<string> digiFilenames;
  
  CmdArg<string> outputBasename;

  /// print usage string
  CmdSwitch help;

};

int main(int argc,
         const char **argv) {
  // libCalibGenCAL will throw runtime_error
  try {
    AppCfg cfg(argc,argv);

    // input file(s)
    vector<string> digiFileList(getLinesFromFile(cfg.digiFilenames.getVal().c_str()));
    if (digiFileList.size() < 1) {
      cout << __FILE__ << ": No input files specified" << endl;
      return -1;
    }

    //-- SETUP LOG FILE --//
    /// multiplexing output streams
    /// simultaneously to cout and to logfile
    LogStrm::addStream(cout);

    // generate logfile name
    const string logfile(cfg.outputBasename.getVal() + ".log.txt");
    ofstream tmpStrm(logfile.c_str());

    LogStrm::addStream(tmpStrm);

    //-- LOG SOFTWARE VERSION INFO --//
    output_env_banner(LogStrm::get());
    LogStrm::get() << endl;
    cfg.cmdParser.printStatus(LogStrm::get());
    LogStrm::get() << endl;

    //-- ROUGH PEDS --//
    // - LEX8 only include hits in histograms.
    MuonPedAlg roughPedAlg;
    CalPed  roughPed;

    // txt output filename
    const string  roughPedTXTFile(cfg.outputBasename.getVal() + ".roughPeds.txt");
    // output histogram file
    const string roughPedHistFileName(cfg.outputBasename.getVal() + ".roughPeds.root");

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
      LogStrm::get() << __FILE__ << ": ERROR! Invalid trigger_cut string: " 
                     << trigCutStr << endl;
      return -1;
    }
    MuonPedAlg::TRIGGER_CUT trigCut     = trigCutMap[trigCutStr];

    const unsigned nEntries(cfg.entriesPerHist.getVal());

    // open new output histogram file
    LogStrm::get() << __FILE__ << ": opening output rough pedestal histogram file: " << roughPedHistFileName <<
      endl;
    TFile roughpedHistfile(roughPedHistFileName.c_str(),
                           "RECREATE",
                           "Muon rough pedestals");
    PedHists roughPedHists(&roughpedHistfile);


    LogStrm::get() << __FILE__ << ": reading root event file(s) starting w/ " << digiFileList[0] << endl;
    roughPedAlg.fillHists(nEntries,
                          digiFileList,
                          NULL,
                          roughPedHists,
                          trigCut);
    roughPedHists.trimHists();

    
    LogStrm::get() << __FILE__ << ": fitting rough pedestal histograms." << endl;
    roughPedHists.fitHists(roughPed);
    LogStrm::get() << __FILE__ << ": writing rough pedestals: " << roughPedTXTFile << endl;
    roughPed.writeTXT(roughPedTXTFile);
    roughpedHistfile.Write();
    roughpedHistfile.Close();

    //-- MUON PEDS --//
    // - LEX8 only include hits in histograms.
    MuonPedAlg  calPedAlg;
    CalPed   calPed;

    // txt output filename
    const string   calPedTXTFile(cfg.outputBasename.getVal() + ".txt");
    // output histogram file
    const string   calPedHistFileName(cfg.outputBasename.getVal() + ".root");

    // open new output histogram file
    LogStrm::get() << __FILE__ << ": opening pedestal output histogram file: " << calPedHistFileName << endl;
    TFile mupedHistfile(calPedHistFileName.c_str(),
                        "RECREATE",
                        "pedestals");
    PedHists calPedHists(&mupedHistfile);
    
    LogStrm::get() << __FILE__ << ": reading root event file(s) starting w/ " << digiFileList[0] << endl;
    calPedAlg.fillHists(nEntries,
                        digiFileList,
                        &roughPed,
                        calPedHists,
                        trigCut);
    calPedHists.trimHists();
    
    LogStrm::get() << __FILE__ << ": fitting pedestal histograms." << endl;
    calPedHists.fitHists(calPed);
    
    LogStrm::get() << __FILE__ << ": writing pedestals: " << calPedTXTFile << endl;
    calPed.writeTXT(calPedTXTFile);

    mupedHistfile.Write();
    mupedHistfile.Close();

    LogStrm::get() << __FILE__ << ": Successfully completed." << endl;
  } catch (exception &e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
    return -1;
  }

  return 0;
}

