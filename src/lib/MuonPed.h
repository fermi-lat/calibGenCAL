#ifndef MuonPed_h
#define MuonPed_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/MuonPed.h,v 1.8 2006/09/28 20:00:24 fewtrell Exp $
/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "CGCUtil.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalVec.h"

// EXTLIB INCLUDES

// STD INCLUDES

class CalPed;
class TH1S;
class DigiEvent;
class CalDigi;

/** \brief Algorithm class populates CalPed calibration object
    by analyzing digi ROOT event files. 

    Supports extracting pedestals from the following trigger schemes
    1 - Event data only 
    2 - Periodic trigger 
    3 - External trigger
    

@author Zachary Fewtrell
*/
class MuonPed {
 public:
  MuonPed(ostream &ostrm = cout);

  /// which type of events should be filtered for 
  /// pedestal processing?
  typedef enum {
    PASS_THROUGH, ///< use full data stream
    PERIODIC_TRIGGER, ///< scan for periodic trigger events in standard data strem
    EXTERNAL_TRIGGER ///< scan for external trigger events
  } TRIGGER_CUT ;

  /// Fill muonpedhist histograms w/ nEvt event data
  /// \param rootFilename.  input digi event file
  /// \param histFilename.  output root file for histograms.
  void fillHists(unsigned nEntries, 
                 const std::vector<std::string> &rootFileList, 
                 const CalPed *roughPeds,
                 TRIGGER_CUT trigCut); 

  /// Fit muonpedhist[]'s, assign means to m_calMuonPed
  void fitHists(CalPed &peds); 

  /// skip evenmt processing and load histograms from previous run
  void loadHists(const std::string &filename);

  /// delete empty histograms
  /// \note useful for data w/ < 16 Cal modules.
  void trimHists();


  static void genOutputFilename(const std::string outputDir,
                                const std::string &inFilename,
                                const std::string &ext,
                                std::string &outFilename) {
    CGCUtil::genOutputFilename(outputDir,
                               "muonPeds",
                               inFilename,
                               ext,
                               outFilename);
  }

 private:
  /// allocate & create muon pedestal histograms & pointer array
  void initHists(); 

  /// count min number of entries in all enable histograms
  unsigned getMinEntries();

  /// process single crystal hit for pedestal data
  void processHit(const CalDigi &calDigi);

  /// process single digi event for pedestal data
  void processEvent(DigiEvent &digiEvt);

  /// list of histograms for 'muon' pedestals
  CalUtil::CalVec<CalUtil::RngIdx, TH1S*> m_histograms; 

  /// used for log messages
  ostream &m_ostrm;

  /// generate ROOT histogram name string.
  static string genHistName(CalUtil::RngIdx rngIdx);

  /// store cfg & status data pertinent to current algorithm run
  struct AlgData {
    AlgData() {init();}

    void init() {
      roughPeds = 0;
      trigCut = PERIODIC_TRIGGER;
    }
    
	const CalPed *roughPeds;
    
	TRIGGER_CUT trigCut;
  } algData;

  /// store data pertinent to current event
  struct EventData{
    EventData() {init();}

    /// reset all member variables
	void init() {
      prev4Range = true;
      fourRange = true;
	  eventNum = 0;	  
    }

	/// set member variables for next event.
	void next() {
		prev4Range = fourRange;
		
		// if mode is unknown, we always treat it as 4 range
		fourRange = true;
		
		eventNum++;
	}

    bool fourRange;
		
	unsigned eventNum;
	
	bool prev4Range;
  } eventData;

};

#endif
