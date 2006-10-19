#ifndef MuonMPD_h
#define MuonMPD_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/MuonMPD.h,v 1.8 2006/10/03 21:09:26 fewtrell Exp $
/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "CGCUtil.h"
#include "TwrHodoscope.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalVec.h"

// EXTLIB INCLUDES
#include "TCanvas.h"
#include "TGraph.h"
#include "TF1.h"
#include "TH2S.h"


// STD INCLUDES
#include <iostream>

class CalPed;
class CIDAC2ADC;
class CalAsym;
class CalMPD;
class TH1S;
class TProfile;
class DigiEvent;

/** \brief Algorithm class generates CalMPD calibration data from digi ROOT
    event files
    
    @author Zachary Fewtrell
*/
class MuonMPD {
 public:
  MuonMPD(std::ostream &ostrm = cout);
   
  /// populate histograms from digi root event file
  void fillHists(unsigned nEntries,
                 const std::vector<std::string> &rootFileList, 
                 const CalPed &peds,
                 const CalAsym &asym,
                 const CIDAC2ADC &dac2adc); 
     
  /// fit histograms & save mean gain values to calMPD
  void fitHists(CalMPD &calMPD); 
       
  /// delete empty histograms
  /// \note useful for data w/ < 16 Cal modules.
  void trimHists();
         
  /// skip event processing & load histograms from previous analysis
  void loadHists(const std::string &filename);
           
  static void genOutputFilename(const std::string outputDir,
                                const std::string &inFilename,
                                const std::string &ext,
                                std::string &outFilename) {
    CGCUtil::genOutputFilename(outputDir,
                               "muonMPD",
                               inFilename,
                               ext,
                               outFilename);
  }
             
  /// output adc2nrg calibration data in TXT format.
  static void writeADC2NRG(const string &filename,
                           const CalAsym &asym,
                           const CIDAC2ADC &dac2adc,
                           const CalMPD &mpd);
               
 private:
  /// process a single event for histogram fill
  void processEvent(DigiEvent &digiEvent);

  /// process a single tower's data in single event for
  /// histogram fill
  void processTower(TwrHodoscope &hscope);
  
  /// hodoscopic event cut for X direction crystals
  bool passCutX(const TwrHodoscope &hscope);
  
  /// hodoscopic event cut for Y direction crystals
  bool passCutY(const TwrHodoscope &hscope);
                   
  /// allocate & create mpdmetry histograms & pointer arrays
  void initHists();
                     
  /// count min number of entries in all enable histograms
  unsigned getMinEntries();
                       
  string genHistName(const std::string &type,
                     CalUtil::XtalIdx xtalIdx);
                         
  /// used for loggin output
  std::ostream &m_ostrm;
                           
  /// # of bins in dacL2S profiles
  static const unsigned short N_L2S_PTS = 20;
  /// min LEDAC val for L2S fitting
  static const unsigned short L2S_MIN_LEDAC = 10;
  /// max LEDAC val for L2S fitting
  static const unsigned short L2S_MAX_LEDAC = 60;
                                 
  /// profile X=bigdiodedac Y=smdiodedac 1 per xtal
  CalUtil::CalVec<CalUtil::XtalIdx, TH1S*> m_dacL2SHists;  
                                   
  /// profile X=bigdiodedac Y=smdiodedac 1 per xtal (not used in main calib, 
  /// only for finding extra l2s slope info)
  CalUtil::CalVec<CalUtil::XtalIdx, TProfile*> m_dacL2SSlopeProfs;
  /// list of histograms of geometric mean for both ends on each xtal.
  CalUtil::CalVec<CalUtil::XtalIdx, TH1S*> m_dacLLHists; 
  /// contains sum of dacLLHists over all xtals
  TH1S *m_dacLLSumHist;
                                       
  /// most likely vertical muon energy deposition in Cal CsI crystal
  static const float MUON_ENERGY;
                                         
  struct AlgData {

    AlgData();

    void init(const CalAsym *calAsym=0) {
      nXEvents = 0;
      nYEvents = 0;
      nXtals   = 0;
      asym     = calAsym;
    }

    unsigned nXEvents;
    unsigned nYEvents;
    unsigned nXtals;

    TCanvas canvas;
    TGraph graph;
    TF1 lineFunc;
    TH2S viewHist;

    const CalAsym *asym;

  } algData;

  struct EventData {
    EventData() :
      hscopes(0),
         eventNum(0)
    {
    }

    ~EventData() {
      if (hscopes)
        delete hscopes;
    }
    
    /// reset all member variables
    void init(const CalPed &peds,
              const CIDAC2ADC &dac2adc) {
      eventNum = 0;
      hscopes = new CalUtil::CalVec<CalUtil::TwrNum, TwrHodoscope>
        (CalUtil::TwrNum::N_VALS, TwrHodoscope(peds, dac2adc));
      next();
    }

    /// rest all member variables that do not retain data
    /// from one event to next.
    void next() {
      // clear all hodoscopes
      if (hscopes)
        for (CalUtil::TwrNum twr; twr.isValid(); twr++)
          (*hscopes)[twr].clear();

    }

    /// need one hodo scope per tower
    CalUtil::CalVec<CalUtil::TwrNum, TwrHodoscope> *hscopes;

    unsigned eventNum;


  } eventData;
};

#endif
