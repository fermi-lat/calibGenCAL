// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/Thresh/genAliveHists.cxx,v 1.1 2008/06/25 15:15:40 chehtman Exp $

/** @file
    @author Zachary Fewtrell

    fill Trigger threshold histograms from LPA histograms.
*/

// LOCAL INCLUDES
#include "src/lib/Util/RootFileAnalysis.h"
// #include "LPAFleAlg.h"
#include "src/lib/Util/CfgMgr.h"
#include "src/lib/Util/CGCUtil.h"
#include "src/lib/Util/string_util.h"
#include "src/lib/Hists/TrigHists.h"
#include "src/lib/Util/stl_util.h"
#include "src/lib/Util/CalSignalArray.h"

// GLAST INCLUDES
#include "CalUtil/SimpleCalCalib/CalPed.h"
#include "CalUtil/SimpleCalCalib/ADC2NRG.h"
#include "digiRootData/Gem.h"
#include "digiRootData/DigiEvent.h"
#include "enums/GemConditionSummary.h"

// EXTLIB INCLUDES
#include "TFile.h"
#include "TH2.h"

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
    entriesPerHist("entriesPerHist",
                   'e',
                   "quit after all histograms have > n entries",
                   1000),
    help("help",
         'h',
         "print usage info")
  {
    cmdParser.registerArg(digiFilenames);
    cmdParser.registerArg(pedFilename);
    cmdParser.registerArg(adc2nrgFilename);
    cmdParser.registerArg(outputBasename);
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

  CmdArg<string> digiFilenames;

  CmdArg<string> pedFilename;

  CmdArg<string> adc2nrgFilename;
  
  CmdArg<string> outputBasename;

  CmdOptVar<unsigned> entriesPerHist;
  
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
    const string logfile(cfg.outputBasename.getVal() + ".alive_hist.log.txt");
    ofstream tmpStrm(logfile.c_str());
    LogStrm::addStream(tmpStrm);

    //-- LOG SOFTWARE VERSION INFO --//
    output_env_banner(LogStrm::get());
    LogStrm::get() << endl;
    cfg.cmdParser.printStatus(LogStrm::get());
    LogStrm::get() << endl;


    /// load up previous calibrations
    CalPed calPed;
    LogStrm::get() << __FILE__ << ": calib file: " << cfg.pedFilename.getVal() << endl;
    calPed.readTXT(cfg.pedFilename.getVal());
    /// load up previous calibrations
    ADC2NRG adc2nrg;
    LogStrm::get() << __FILE__ << ": calib file: " << cfg.adc2nrgFilename.getVal() << endl;
    adc2nrg.readTXT(cfg.adc2nrgFilename.getVal());

    // open new output histogram file
    // output histogram file
    const string histfilePath(cfg.outputBasename.getVal() 
                              + ".Alive"
                              + ".root");
    LogStrm::get() << __FILE__ << ": opening output histogram file: " << histfilePath << endl;
    TFile histfile(histfilePath.c_str(),
                   "RECREATE",
                   (string("Alive")
                    + "_hists").c_str());

    /// store alogrithm histograms
    TrigHists lex8Hists("lex8Hist",
                        &histfile, 0,
                        200, 0, 4000);
    TrigHists hex8Hists("hex8Hist",
                        &histfile, 0,
                        200, 0, 4000);

    HistVec<CalUtil::XtalIdx, TH1S> p2nlex8Hists("p2nlex8Hist",
                        &histfile, 0,
                        100, 0, 2);
    HistVec<CalUtil::XtalIdx, TH1S> p2nhex8Hists("p2nhex8Hist",
                        &histfile, 0,
                        100, 0, 2);


      for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++){
	lex8Hists.produceHist(faceIdx);
	hex8Hists.produceHist(faceIdx);
      }
      for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++){
	p2nlex8Hists.produceHist(xtalIdx);
	p2nhex8Hists.produceHist(xtalIdx);

      }

    /// store cfg & status data pertinent to current algorithm run
    struct EventData {
      EventData(const CalUtil::CalPed &peds,
                const CalUtil::ADC2NRG &adc2nrg) :
        m_calSignalArray(peds, adc2nrg)
      {
        init();
      }
      
      /// intialize to beginning of alg
      void init() {
        m_eventNum = 0;
        clear();
      }

      /// clear out arrays
      void clear(){
	m_calSignalArray.clear();
      }

      /// setup data for next event
      void nextEvent() {
        m_eventNum++;
        clear();
      }

      /// face signal array from cal tuple
      CalSignalArray m_calSignalArray;

      unsigned m_eventNum;
      
    } eventData(calPed,adc2nrg);






    const unsigned nEntries(cfg.entriesPerHist.getVal());


    LogStrm::get() << __FILE__ << ": reading root event file(s) starting w/ " << digiFileList[0] << endl;
    
    //DEBUG



    //    aliveAlg.fillHists(nEntries,
    //                      digiFileList);


    /////////////////////////////////////////
    /// Open ROOT Event File  ///////////////
    /////////////////////////////////////////
    RootFileAnalysis rootFile(0,             // mc
                              &digiFileList);

    // configure active branches from input ROOT trees
    //    cfgBranches(rootFile);
    rootFile.getDigiChain()->SetBranchStatus("*", 0);
    rootFile.getDigiChain()->SetBranchStatus("m_calDigiCloneCol"); 
    rootFile.getDigiChain()->SetBranchStatus("m_summary");
    rootFile.getDigiChain()->SetBranchStatus("m_gem");
        
    //    rootFile.getDigiChain()->SetBranchStatus("m_calDiagnosticCloneCol");


    const unsigned nEvents = rootFile.getEntries();
    //    const unsigned nEvents = 100000;
    LogStrm::get() << __FILE__ << ": Processing: " << nEvents << " events." << endl;

    /////////////////////////////////////////
    /// Event Loop //////////////////////////
    /////////////////////////////////////////
    for (eventData.init(); eventData.m_eventNum < nEvents; eventData.nextEvent()) {
      /////////////////////////////////////////
      /// Load new event //////////////////////
      /////////////////////////////////////////
      if (eventData.m_eventNum % 2000 == 0) {
        // quit if we have enough entries in each histogram
        const unsigned currentMin = lex8Hists.getMinEntries();
        if (currentMin >= nEntries) break;
        LogStrm::get() << "Event: " << eventData.m_eventNum
                       << " min entries per histogram: " << currentMin
                       << endl;
        LogStrm::get().flush();
      }

      if (!rootFile.getEvent(eventData.m_eventNum)) {
        LogStrm::get() << "Warning, event " << eventData.m_eventNum << " not read." << endl;
        continue;
      }

      DigiEvent const*const digiEvent = rootFile.getDigiEvent();
      if (!digiEvent) {
        LogStrm::get() << __FILE__ << ": Unable to read DigiEvent " << eventData.m_eventNum  << endl;
        continue;
      }

      //-- retrieve trigger data
      const Gem *gem = 0;
      unsigned   gemConditionsWord = 0;

      gem = &(digiEvent->getGem());
      gemConditionsWord = gem->getConditionSummary();
      const float gemDeltaEventTime = gem->getDeltaEventTime()*0.05;
      if((gemConditionsWord &32) !=0  ||  gemDeltaEventTime<70)continue;
      //      if((gemConditionsWord &32) !=0)continue;

      /// load up faceSignal in mev from all digis
      //      eventData.m_calSignalArray.fillArray(*digiEvent);


      const TClonesArray *calDigiCol = digiEvent->getCalDigiCol();
      if (!calDigiCol) {
	LogStrm::get() << "no calDigiCol found for event#" << eventData.m_eventNum << endl;
	continue;
      }

      TIter calDigiIter(calDigiCol);

      const CalDigi      *pCalDigi = 0;

      while ((pCalDigi = dynamic_cast<CalDigi *>(calDigiIter.Next()))){

	const CalDigi &calDigi = *pCalDigi;
	//-- XtalId --//
	const idents::CalXtalId id(calDigi.getPackedId()); 

	const XtalIdx xtalIdx(id);

	const unsigned nRO = calDigi.getNumReadouts();

	for (unsigned short n = 0; n < nRO; n++) {
	  const CalXtalReadout &readout = *calDigi.getXtalReadout(n);
	  short unsigned rngRo[2];
	  float adcPedRo[2];
	  
	  for (FaceNum face; face.isValid(); face++) {
	    // check that we are in the expected readout mode
	    const RngNum rng(readout.getRange((CalXtalId::XtalFace)face.val()));
	    const unsigned short adc(readout.getAdc((CalXtalId::XtalFace)face.val()));
	    const RngIdx rngIdx(xtalIdx,face,rng);
	    const FaceIdx faceIdx(xtalIdx,face);
	      float adcPed = adc-calPed.getPed(rngIdx);
	      short unsigned pn = face.val();
	      rngRo[pn]=rng.val();
	      adcPedRo[pn]=adcPed;
	

	    if(rng == CalUtil::LEX8){
	      lex8Hists.produceHist(faceIdx).Fill(adcPed);
	    }
	    if(rng == CalUtil::HEX8){
	      hex8Hists.produceHist(faceIdx).Fill(adcPed);
	    }

	  }
	  if(rngRo[0] == rngRo[1] && adcPedRo[1]>30){
	    if(rngRo[0] == 0){
	      p2nlex8Hists.produceHist(xtalIdx).Fill(adcPedRo[0]/adcPedRo[1]);
	    }
	    if(rngRo[0] == 2){
	      p2nhex8Hists.produceHist(xtalIdx).Fill(adcPedRo[0]/adcPedRo[1]);	    
	    }
	  }	
	}
      }
    }


	TH2F* hnlex8[16];
	TH2F* hnhex8[16];
	TH2F* havlex8[16];
	TH2F* havhex8[16];
	TH2F* hp2nlex8[16];
	TH2F* hp2nhex8[16];
	for (int twr=0; twr<16; twr++){
		ostringstream hnlex8name;
		hnlex8name << "hnlex8_twr"<< twr;
		ostringstream hnlex8title;
		hnlex8title << "tower " << twr;
		hnlex8[twr] = new TH2F(hnlex8name.str().c_str(), hnlex8title.str().c_str(),24,0,12,8,0,8);
		//		hnlex8[twr]->SetMaximum(1000);
		hnlex8[twr]->SetMinimum(0);
		hnlex8[twr]->SetXTitle("col+0.5*face");
		hnlex8[twr]->SetYTitle("layer");


		ostringstream hnhex8name;
		hnhex8name << "hnhex8_twr"<< twr;
		ostringstream hnhex8title;
		hnhex8title << "tower " << twr;
		hnhex8[twr] = new TH2F(hnhex8name.str().c_str(), hnhex8title.str().c_str(),24,0,12,8,0,8);
		//		hnhex8[twr]->SetMaximum(1000);
	        hnhex8[twr]->SetMinimum(0);
		hnhex8[twr]->SetXTitle("col+0.5*face");
		hnhex8[twr]->SetYTitle("layer");

		ostringstream havhex8name;
		havhex8name << "havhex8_twr"<< twr;
		ostringstream havhex8title;
		havhex8title << "tower " << twr;
		havhex8[twr] = new TH2F(havhex8name.str().c_str(), havhex8title.str().c_str(),24,0,12,8,0,8);
		//		havhex8[twr]->SetMaximum(1000);
	        havhex8[twr]->SetMinimum(0);
		havhex8[twr]->SetXTitle("col+0.5*face");
		havhex8[twr]->SetYTitle("layer");

		ostringstream havlex8name;
		havlex8name << "havlex8_twr"<< twr;
		ostringstream havlex8title;
		havlex8title << "tower " << twr;
		havlex8[twr] = new TH2F(havlex8name.str().c_str(), havlex8title.str().c_str(),24,0,12,8,0,8);
		//		havlex8[twr]->SetMaximum(1000);
	        havlex8[twr]->SetMinimum(0);
		havlex8[twr]->SetXTitle("col+0.5*face");
		havlex8[twr]->SetYTitle("layer");

		ostringstream hp2nlex8name;
		hp2nlex8name << "hp2nlex8_twr"<< twr;
		ostringstream hp2nlex8title;
		hp2nlex8title << "tower " << twr;
		hp2nlex8[twr] = new TH2F(hp2nlex8name.str().c_str(), hp2nlex8title.str().c_str(),12,0,12,8,0,8);
     		hp2nlex8[twr]->SetMaximum(2);
	        hp2nlex8[twr]->SetMinimum(0);
		hp2nlex8[twr]->SetXTitle("col");
		hp2nlex8[twr]->SetYTitle("layer");

		ostringstream hp2nhex8name;
		hp2nhex8name << "hp2nhex8_twr"<< twr;
		ostringstream hp2nhex8title;
		hp2nhex8title << "tower " << twr;
		hp2nhex8[twr] = new TH2F(hp2nhex8name.str().c_str(), hp2nhex8title.str().c_str(),12,0,12,8,0,8);
     		hp2nhex8[twr]->SetMaximum(2);
	        hp2nhex8[twr]->SetMinimum(0);
		hp2nhex8[twr]->SetXTitle("col");
		hp2nhex8[twr]->SetYTitle("layer");

	}	

	float maxnlex8=0;
	float maxnhex8=0;
	float maxavlex8=0;
	float maxavhex8=0;

      for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++){
	float p2nlex8 = p2nlex8Hists.produceHist(xtalIdx).GetMean();
	float p2nhex8 = p2nhex8Hists.produceHist(xtalIdx).GetMean();
	unsigned short twr = xtalIdx.getTwr().val();
	unsigned short lyr = xtalIdx.getLyr().val();
	unsigned short col = xtalIdx.getCol().val();
	(hp2nlex8[twr])->Fill(col,lyr,p2nlex8);
	(hp2nhex8[twr])->Fill(col,lyr,p2nhex8);	
      }
      for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++){
	int nlex8 = (int)lex8Hists.produceHist(faceIdx).GetEntries();
	int nhex8 = (int)hex8Hists.produceHist(faceIdx).GetEntries();
	float avlex8 = lex8Hists.produceHist(faceIdx).GetMean();
	float avhex8 = hex8Hists.produceHist(faceIdx).GetMean();
	if(maxnlex8 < nlex8)maxnlex8=nlex8;
	if(maxnhex8 < nhex8)maxnhex8=nhex8;
	if(maxavlex8 < avlex8)maxavlex8=avlex8;
	if(maxavhex8 < avhex8)maxavhex8=avhex8;

	unsigned short twr = faceIdx.getTwr().val();
	unsigned short lyr = faceIdx.getLyr().val();
	unsigned short col = faceIdx.getCol().val();
	unsigned short face = faceIdx.getFace().val();

	(hnlex8[twr])->Fill(col+face*0.5,lyr,nlex8);
	(hnhex8[twr])->Fill(col+face*0.5,lyr,nhex8);
	(havlex8[twr])->Fill(col+face*0.5,lyr,avlex8);
	(havhex8[twr])->Fill(col+face*0.5,lyr,avhex8);

	//	LogStrm::get() << " xtalFace = " << faceIdx.toStr() << "  nlex8=" << nlex8 << "  nhex8=" << nhex8 << endl;


      }
	for (int twr=0; twr<16; twr++){

		hnhex8[twr]->SetMaximum(maxnhex8*1.5);
		hnlex8[twr]->SetMaximum(maxnlex8*1.5);
		havhex8[twr]->SetMaximum(maxavhex8*1.5);
		havlex8[twr]->SetMaximum(maxavlex8*1.5);
	}


    LogStrm::get() << __FILE__ << ": Writing output ROOT file." << endl;
    histfile.Write();
    histfile.Close();

    LogStrm::get() << __FILE__ << ": Successfully completed." << endl;

  } catch (exception &e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
    return -1;
  }
  return 0;
}

