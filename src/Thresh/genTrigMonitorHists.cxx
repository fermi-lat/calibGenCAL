// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/Thresh/genFLEHists.cxx,v 1.1 2008/04/21 20:43:14 fewtrell Exp $

/** @file
    @author Z. Fewtrell
    
    Try to find good Cal thresholds from standard LAT configuration data
    Bascially histogram most energetic crystal in each tower with trigger bit set
*/

// LOCAL INCLUDES
#include "src/lib/Hists/TrigHists.h"
#include "src/lib/Util/CfgMgr.h"
#include "src/lib/Util/CGCUtil.h"
#include "src/lib/Util/RootFileAnalysis.h"
#include "src/lib/Util/CalSignalArray.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalVec.h"
#include "CalUtil/SimpleCalCalib/CalPed.h"
#include "CalUtil/SimpleCalCalib/ADC2NRG.h"
#include "CalUtil/bit_util.h"
#include "digiRootData/DigiEvent.h"
#include "digiRootData/Gem.h"

// EXTLIB INCLUDES
#include "TFile.h"
#include "TH2S.h"

// STD INCLUDES
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

using namespace CalUtil;
using namespace calibGenCAL;
using namespace CfgMgr;
using namespace std;

/// Manage application configuration parameters
class AppCfg {
public:
  AppCfg(const int argc,
         const char **argv) :
    cmdParser(path_remove_ext(__FILE__)),
    digiFile("digiFile",
             "input Digi ROOT file",
             ""),
    pedFilename("pedFilename",
                "text file with pedestal calibration data",
                ""),
    adc2nrgFilename("muSlopeFilename",
                    "text file with muSlope (adc2mev) calibration data",
                    ""),
    outputBasename("outputBasename",
                   "all output files will use this basename + some_ext",
                   ""),
    help("help",
         'h',
         "print usage info")
  {
    cmdParser.registerArg(digiFile);
    cmdParser.registerArg(pedFilename);
    cmdParser.registerArg(adc2nrgFilename);
    cmdParser.registerArg(outputBasename);
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

  CmdArg<string> digiFile;

  CmdArg<string> pedFilename;

  CmdArg<string> adc2nrgFilename;
  
  CmdArg<string> outputBasename;

  /// print usage string
  CmdSwitch help;

};

namespace {
  static const float FLE_HIST_MIN = 0;
  static const float FLE_HIST_MAX = 300;
  static const float FHE_HIST_MIN = 0;
  static const float FHE_HIST_MAX = 3000;
  static const unsigned N_EVENTS_STATUS = 1000;
  static const unsigned short N_HIST_BINS = 100;

}
                                                  

int main(const int argc, const char **argv) {
  // libCalibGenCAL will throw runtime_error
  try {
    AppCfg cfg(argc,argv);

    // input file(s)
    const vector<string> digiFileList(1,cfg.digiFile.getVal());
    if (digiFileList.size() < 1) {
      cout << __FILE__ << ": No input files specified" << endl;
      return -1;
    }

    //-- SETUP LOG FILE --//
    /// multiplexing output streams
    /// simultaneously to cout and to logfile
    LogStrm::addStream(cout);

    // generate logfile name
    const string logfile(cfg.outputBasename.getVal() + ".fle_hist.log.txt");
    ofstream tmpStrm(logfile.c_str());
    LogStrm::addStream(tmpStrm);

    //-- LOG SOFTWARE VERSION INFO --//
    output_env_banner(LogStrm::get());
    LogStrm::get() << endl;
    cfg.cmdParser.printStatus(LogStrm::get());
    LogStrm::get() << endl;

    /// open ROOT event file
    LogStrm::get() << __FILE__ << ": opening input Digi ROOT file. " << cfg.digiFile.getVal() << endl;
    RootFileAnalysis rootFile(0,             // mc
                              &digiFileList);
    // enable only needed branches in root file
    rootFile.getDigiChain()->SetBranchStatus("*", 0);
    rootFile.getDigiChain()->SetBranchStatus("m_calDigiCloneCol");
    rootFile.getDigiChain()->SetBranchStatus("m_gem");

    /// load up previous calibrations
    CalPed calPed;
    LogStrm::get() << __FILE__ << ": calib file: " << cfg.pedFilename.getVal() << endl;
    calPed.readTXT(cfg.pedFilename.getVal());
    /// load up previous calibrations
    ADC2NRG adc2nrg;
    LogStrm::get() << __FILE__ << ": calib file: " << cfg.adc2nrgFilename.getVal() << endl;
    adc2nrg.readTXT(cfg.adc2nrgFilename.getVal());
    CalSignalArray calSignalArray(calPed, adc2nrg);

    // open new output histogram file
    // output histogram file
    const string histfilePath(cfg.outputBasename.getVal() 
                              + ".cal_thr_monitor.root");
    LogStrm::get() << __FILE__ << ": opening output histogram file: " << histfilePath << endl;
    TFile histfile(histfilePath.c_str(),
                   "RECREATE");


    /// GENERATE OUTPUT HISTOGRAMS
    TrigHists fleHists("fleHist",
                       &histfile, 0,
                       N_HIST_BINS, FLE_HIST_MIN ,FLE_HIST_MAX);
    TrigHists fheHists("fheHist",
                       &histfile, 0,
                       N_HIST_BINS, FHE_HIST_MIN, FHE_HIST_MAX);

    /// histogram all FLE channels in Cal
    TH2S *const calFLEHist = new TH2S("calFLEHist",
                                      "calFLEHist",
                                      FaceIdx::N_VALS, 0, FaceIdx::N_VALS+1,
                                      N_HIST_BINS,FLE_HIST_MIN,FLE_HIST_MAX);
      

    /// histogram all FHE channels in Cal
    TH2S *const calFHEHist = new TH2S("calFHEHist",
                                      "calFHEHist",
                                      FaceIdx::N_VALS, 0, FaceIdx::N_VALS+1,
                                      N_HIST_BINS,FHE_HIST_MIN,FHE_HIST_MAX);
    
    // EVENT LOOP
    const unsigned nEvents = rootFile.getEntries();
    LogStrm::get() << "Processing " << nEvents << " events." << endl;
    for (unsigned nEvt = 0;
         nEvt < nEvents;
         nEvt++) {
    
      /// load next event
      rootFile.getEvent(nEvt);

      // status print out
      if (nEvt % N_EVENTS_STATUS == 0)
        cerr << nEvt << endl;

      /// retrieve DigiEvent
      DigiEvent const*const digiEvent = rootFile.getDigiEvent();
      if (!digiEvent) {
        LogStrm::get() << __FILE__ << ": Unable to read DigiEvent: " << nEvt  << endl;
        continue;
      }

      /// get Trigger bits
      const Gem &gem = digiEvent->getGem();
      const UShort_t GemCalLeVector = gem.getCalLeVector();
      const UShort_t GemCalHeVector = gem.getCalHeVector();

      // skip if there's no trigger
      if (GemCalLeVector==0 && GemCalHeVector==0)
        continue;

      // fill array with signal levels for each channel
      calSignalArray.clear();
      calSignalArray.fillArray(*digiEvent);

      // TWR LOOP
      for (TwrNum twr; twr.isValid(); twr++) {
        // get indices for 1st & last channel in LAT
        const FaceIdx twrBegin(twr,LyrNum(0),0,FaceNum(0));
        const FaceIdx twrEnd(twr.val()+1,LyrNum(0),0,FaceNum(0));

        typedef CalSignalArray::FaceSignalArray FSArray;
        const FSArray &faceSignal(calSignalArray.getFaceSignalArray());

        /// check for FLE trigger in current tower
        if (check_bit(GemCalLeVector, twr.val())) {
          const size_t fleChannelIdx = max_element(faceSignal.begin() + twrBegin.val(),
                                                   faceSignal.begin() + twrEnd.val()) - faceSignal.begin();
          const FaceIdx faceIdx(fleChannelIdx);
          const float mev = faceSignal[faceIdx];

          if (between(FLE_HIST_MIN,mev,FLE_HIST_MAX)) {
            /// fill this channel's histogram
            fleHists.produceHist(faceIdx).Fill(mev);
            /// fill cal-wide histogram
            calFLEHist->Fill(faceIdx.val(), mev);
          }
          
        }
        
        /// check for FHE trigger in current tower
        if (check_bit(GemCalHeVector, twr.val())) { 
          const size_t fheChannelIdx = max_element(faceSignal.begin()+twrBegin.val(),
                                                   faceSignal.begin()+twrEnd.val()) - faceSignal.begin();
          const FaceIdx faceIdx(fheChannelIdx);
          const float mev = faceSignal[faceIdx];


          if (between(FHE_HIST_MIN,mev,FHE_HIST_MAX)) {
            /// fill this channel's histogram
            fheHists.produceHist(faceIdx).Fill(mev);
            /// fill cal-wide histogram
            calFHEHist->Fill(faceIdx.val(), mev);
          }
        }
      }
    }


    histfile.Write();
    histfile.Close();

  } catch (exception &e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
    return -1;
  }

  return 0;
}
