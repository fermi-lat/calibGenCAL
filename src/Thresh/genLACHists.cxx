// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/Thresh/genLACHists.cxx,v 1.1 2008/04/21 20:43:14 fewtrell Exp $

/** @file
    @author Zachary Fewtrell

    Generate LAC threshold histograms for each crystal face from zero suppressed data.
    Also save pedestal histograms in case of pedestal drift.
*/


// LOCAL INCLUDES
#include "src/lib/Util/CfgMgr.h"
#include "src/lib/Util/string_util.h"
#include "src/lib/Util/CGCUtil.h"
#include "src/lib/Util/RootFileAnalysis.h"
#include "src/lib/Util/CalSignalArray.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalVec.h"
#include "CalUtil/SimpleCalCalib/CalPed.h"
#include "CalUtil/SimpleCalCalib/ADC2NRG.h"
#include "digiRootData/Gem.h"
#include "digiRootData/DigiEvent.h"

// EXTLIB INCLUDES
#include "TFile.h"
#include "TTree.h"
#include "TH2F.h"
#include "TH1I.h"
#include "TF1.h"

// STD INCLUDES
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <fstream>

using namespace std;
using namespace CfgMgr;
using namespace calibGenCAL;
using namespace CalUtil;
                                                  
/// Manage application configuraiton parameters
class AppCfg {
public:
  AppCfg(const int argc,
         const char **argv) :
    cmdParser(path_remove_ext(__FILE__)),
    faceName("faceName",
             "process either \"POS_FACE\" or \"NEG_FACE\" data",
             ""),
    digiFilenames("digiFilenames",
                  "text file w/ newline delimited list of input digi ROOT files",
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
    nEvts("numEvents",
          "number of events to process",
          0),
    help("help",
         'h',
         "print usage info")
  {
    cmdParser.registerArg(faceName);
    cmdParser.registerArg(digiFilenames);
    cmdParser.registerArg(pedFilename);
    cmdParser.registerArg(adc2nrgFilename);
    cmdParser.registerArg(outputBasename);
    cmdParser.registerArg(nEvts);
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

  /// current alg processes only one face @ a time
  CmdArg<string> faceName;

  CmdArg<string> digiFilenames;
  
  CmdArg<string> pedFilename;

  CmdArg<string> adc2nrgFilename;
  
  CmdArg<string> outputBasename;

  CmdArg<unsigned> nEvts;

  /// print usage string
  CmdSwitch help;

};


int main(const int argc, const char **argv) {

  // libCalibGenCAL will throw runtime_error
  try {
    AppCfg cfg(argc,argv);

    // get crystal face from cmdline
    FaceNum face;
    if (cfg.faceName.getVal() == "POS")
      face = POS_FACE;
    else if (cfg.faceName.getVal() == "NEG")
      face = NEG_FACE;
    else {
      cout << "Invalid crystal face string: " << cfg.faceName.getVal() << endl;
      return -1;
    }
      

    //-- SETUP LOG FILE --//
    /// multiplexing output streams
    /// simultaneously to cout and to logfile
    LogStrm::addStream(cout);

    // generate logfile name
    const string logfile(cfg.outputBasename.getVal() + ".lac_hist.log.txt");
    ofstream tmpStrm(logfile.c_str());
    LogStrm::addStream(tmpStrm);

    // generate output ROOT filename
    const string outputPath(cfg.outputBasename.getVal() + ".lac_hist.root");

    // open input files
    // input file(s)
    vector<string> digiFileList(getLinesFromFile(cfg.digiFilenames.getVal()));
    if (digiFileList.size() < 1) {
      cout << __FILE__ << ": No input files specified" << endl;
      return -1;
    }

    /// load up previous calibrations
    CalPed calPed;
    calPed.readTXT(cfg.pedFilename.getVal());
    /// load up previous calibrations
    ADC2NRG adc2nrg;
    adc2nrg.readTXT(cfg.adc2nrgFilename.getVal());


    RootFileAnalysis rootFile(0,
                              &digiFileList,
                              0);

    // open output files
    TFile output(outputPath.c_str(),"RECREATE");

    // ENABLE / REGISTER TUPLE BRANCHES
    rootFile.getDigiChain()->SetBranchStatus("*", 0);
    rootFile.getDigiChain()->SetBranchStatus("m_calDigiCloneCol");
    rootFile.getDigiChain()->SetBranchStatus("m_summary");
    rootFile.getDigiChain()->SetBranchStatus("m_gem");

    CalVec<XtalIdx, TH1I*> hadc;
    CalVec<XtalIdx, TH1I*> hped;

    for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
      const FaceIdx faceIdx(xtalIdx, face);

      ostringstream hadcname;
      hadcname << "hadc_" << faceIdx.toStr();
      hadc[xtalIdx] = new TH1I(hadcname.str().c_str(),hadcname.str().c_str(),60,0,300);

      ostringstream hpedname;
      hpedname << "hped_" << faceIdx.toStr();
      hped[xtalIdx] = new TH1I(hpedname.str().c_str(),hpedname.str().c_str(),200,-100,100);
    }

    /// store cal signal levels for each channel
    CalSignalArray calSignalArray(calPed, adc2nrg);

    // EVENT LOOP
    const unsigned  nEvents = min<unsigned>(rootFile.getEntries(), cfg.nEvts.getVal());
    LogStrm::get() << __FILE__ << ": Processing: " << nEvents << " events." << endl;

    for (unsigned nEvt = 0;
         nEvt < nEvents;
         nEvt++) {

      // read new event
      rootFile.getEvent(nEvt);

      // read in cal digis
      calSignalArray.clear();
      
      DigiEvent const*const digiEvent = rootFile.getDigiEvent();
      if (!digiEvent) {
        LogStrm::get() << __FILE__ << ": Unable to read DigiEvent " << nEvt  << endl;
        continue;
      }

      calSignalArray.fillArray(*digiEvent);

      //-- retrieve trigger data
      const Gem &gem =digiEvent->getGem();
      const unsigned gemConditionsWord = gem.getConditionSummary();
      
      // status print out
      if (nEvt % 1000 == 0)
        LogStrm::get() << nEvt << endl;
      
      CalVec<FaceNum, float> ene;
      CalVec<FaceNum, float> adc;
      CalVec<FaceNum, RngNum> rng;

      for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
        for (FaceNum tmpFace; tmpFace.isValid(); tmpFace++) {
          const FaceIdx faceIdx(xtalIdx, tmpFace);
          ene[tmpFace] = calSignalArray.getFaceSignal(faceIdx);
          adc[tmpFace] = calSignalArray.getAdcPed(faceIdx);
          rng[tmpFace] = calSignalArray.getAdcRng(faceIdx);
        }

        /// avoid periodic triggers (pedestals)
        if(!gem.getPeriodicSet()){
          if(rng[POS_FACE] == LEX8 && rng[NEG_FACE] == LEX8 && 
             adc[POS_FACE]>3 && adc[NEG_FACE]>3 && 
             adc[POS_FACE]<350 && adc[NEG_FACE]<350) {
            /// skip direct deposit hits by comparing asymmetry between xtal faces
            const FaceNum oppFace(face.oppositeFace());
            /// 'healthy assymmetry maxes around 2:1, so we'll cut anything above 3:1
            if (ene[face] / ene[oppFace] < 3)
              hadc[xtalIdx]->Fill(adc[face]); 
          }
        }
        
        /// select periodic (only) triggers for pedestals
        if(gemConditionsWord == enums::PERIODIC)
          hped[xtalIdx]->Fill(adc[face]); 
      }
    }
   
    output.Write();
    output.Close();

  } catch (exception &e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
    return -1;
  }

  return 0;
}

