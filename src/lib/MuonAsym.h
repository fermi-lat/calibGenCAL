#ifndef MuonAsym_h
#define MuonAsym_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/MuonAsym.h,v 1.6 2006/09/28 20:00:24 fewtrell Exp $
/** @file
    @author Zachary Fewtrell
*/


// LOCAL INCLUDES
#include "CGCUtil.h"
#include "TwrHodoscope.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalVec.h"
#include "CalUtil/CalArray.h"


// EXTLIB INCLUDES

// STD INCLUDES

class CalAsym;
class CalPed;
class CIDAC2ADC;
class TwrHodoscope;
class TH2S;
class DigiEvent;

/** \brief Algorithm class populates CalAsym calibration data with values extracted
    from Muon collection digi ROOT event files

    @author Zachary Fewtrell
*/
class MuonAsym {
 public:
  MuonAsym(ostream &ostrm = cout);

  /// populate asymmetry profiles w/ nEvt worth of data.
  void fillHists(unsigned nEntries,
                 const vector<string> &rootFileList, 
                 const CalPed &peds,
                 const CIDAC2ADC &dac2adc); 

  /// load mean values from asymmetry profiles into m_asym*** arrays
  void fitHists(CalAsym &calAsym); 

  /// load histograms from ROOT output of previous run
  void loadHists(const string &filename);

  /// print histogram summary info to output stream
  void summarizeHists(ostream &ostrm=cout);

  /// delete empty histograms
  /// \note useful for data w/ < 16 Cal modules.
  void trimHists();

  static void genOutputFilename(const string outputDir,
                                const string &inFilename,
                                const string &ext,
                                string &outFilename) {
    CGCUtil::genOutputFilename(outputDir,
                               "muonAsym",
                               inFilename,
                               ext,
                               outFilename);
  }


 private:
  /// process a single event for histogram fill
  void processEvent(DigiEvent &digiEvent);

  /// process a single tower's data in single event for
  /// histogram fill
  void processTower(TwrHodoscope &hscope);

  /// hodoscopic event cut for X direction xtals
  bool passCutX(const TwrHodoscope &hscope);
  /// hodoscopic event cut for Y direction xtals
  bool passCutY(const TwrHodoscope &hscope);

  /// allocate & create asymmetry histograms & pointer arrays
  void initHists();

  /// count min number of entries in all enable histograms
  unsigned getMinEntries();

  /// list of histograms for muon asymmetry
  CalUtil::CalVec<CalUtil::AsymType, CalUtil::CalArray<CalUtil::XtalIdx, TH2S*> > m_histograms; 

  std::string genHistName(CalUtil::AsymType asymType, CalUtil::XtalIdx xtalIdx);

  /// used for logging output
  ostream &m_ostrm;

  struct AlgData {
    AlgData() {init();}
    void init() {
      nGoodDirs = 0;
      nXDirs = 0;
      nYDirs = 0;
      nHits = 0;
      nBadHits = 0;
    }

    unsigned  nGoodDirs  ; // count total # of events used
    unsigned  nXDirs     ;
    unsigned  nYDirs     ;
    long nHits      ; // count total # of xtals measured
    unsigned  nBadHits   ;



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
      for (CalUtil::TwrNum twr; twr.isValid(); twr++)
        if (hscopes)
          (*hscopes)[twr].clear();

    }

    /// need one hodo scope per tower
    CalUtil::CalVec<CalUtil::TwrNum, TwrHodoscope> *hscopes;

    unsigned eventNum;


  } eventData;

};

#endif
