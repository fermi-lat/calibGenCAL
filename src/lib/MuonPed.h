#ifndef MuonPed_h
#define MuonPed_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/MuonPed.h,v 1.4 2006/09/19 18:49:27 fewtrell Exp $
/** @file
    @author Zachary Fewtrell
 */



// LOCAL INCLUDES
#include "CalPed.h"
#include "CGCUtil.h"
#include "RootFileAnalysis.h"

// GLAST INCLUDES
#include "CalUtil/CalVec.h"
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalArray.h"

// EXTLIB INCLUDES
#include "TH1S.h"
#include "TFile.h"

// STD INCLUDES


using namespace std;
using namespace CalUtil;

/** \brief \brief Represents GLAST Cal ADC pedestal calibrations

    contains read & write methods to various file formats & code
    to calculate calibrations from digi ROOT event files

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
                 const vector<string> &rootFileList, 
                 const CalPed *roughPeds,
                 TRIGGER_CUT trigCut); 

  /// Fit muonpedhist[]'s, assign means to m_calMuonPed
  void fitHists(CalPed &peds); 

  /// skip evenmt processing and load histograms from previous run
  void loadHists(const string &filename);

  static void genOutputFilename(const string outputDir,
                                const string &inFilename,
                                const string &ext,
                                string &outFilename) {
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
  CalVec<RngIdx, TH1S*> m_histograms; 

  /// used for log messages
  ostream &m_ostrm;

  /// generate ROOT histogram name string.
  static string genHistName(RngIdx rngIdx);

  /// store cfg & status data pertinent to current algorithm run
  struct {
    void clear() {memset(this, 0, sizeof(*this));}
    const CalPed *roughPeds;
    TRIGGER_CUT trigCut;
  } algData;

  /// store data pertinent to current event
  struct {
    void clear() {
      memset(this, 0, sizeof(*this));

      prev4Range = true;
      fourRange = true;
    }

    bool prev4Range;
    bool fourRange;
    unsigned evtNum;
  } evtData;

};

#endif
