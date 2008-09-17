#ifndef MuonPedAlg_h
#define MuonPedAlg_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/Ped/MuonPedAlg.h,v 1.2 2008/04/23 16:54:51 fewtrell Exp $

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalVec.h"
#include "src/lib/Hists/PedHists.h"


// EXTLIB INCLUDES

// STD INCLUDES
#include <vector>
#include <string>

class TH1S;
class DigiEvent;
class CalDigi;
class TDirectory;

namespace CalUtil {
  class CalPed;
}

namespace calibGenCAL {
  /** \brief Algorithm class populates CalPed calibration object
      by analyzing digi ROOT event files.

      Supports extracting pedestals from the following trigger schemes
      1 - Event data only
      2 - Periodic trigger
      3 - External trigger


      @author Zachary Fewtrell
  */
  class MuonPedAlg {
  public:
    MuonPedAlg() {}

    /// which type of events should be filtered for
    /// pedestal processing?
    typedef enum {
      PASS_THROUGH,       ///< use full data stream
      PERIODIC_TRIGGER,   ///< scan for periodic trigger events in standard data stream
      EXTERNAL_TRIGGER    ///< scan for external trigger events
    } TRIGGER_CUT ;

    /// Fill MuonPedAlghist histograms w/ nEvt event data
    /// \param rootFilename.  input digi event file
    /// \param histFilename.  output root file for histograms.
    void fillHists(const unsigned nEntries,
                   const std::vector<std::string> &rootFileList,
                   const CalUtil::CalPed *roughPeds,
                   PedHists &pedHists,
                   const TRIGGER_CUT trigCut);
    
  private:
    /// process single crystal hit for pedestal data
    void     processHit(const CalDigi &calDigi);

    /// process single digi event for pedestal data
    void     processEvent(const DigiEvent &digiEvt);


    /// generate ROOT histogram name string.
    static string genHistName(const CalUtil::RngIdx rngIdx);

    /// store cfg & status data pertinent to current algorithm run
    class AlgData {
    private:
      void init() {
        roughPeds = 0;
        trigCut   = PERIODIC_TRIGGER;
      }

    public:
      AlgData() {
        init();
      }

      const CalUtil::CalPed *roughPeds;

      TRIGGER_CUT   trigCut;
      
      PedHists *pedHists;
    } algData;

    /// store data pertinent to current event
    class EventData {
    private:
      /// reset all member variables
      void init() {
        prev4Range = true;
        fourRange  = true;
        eventNum   = 0;
      }

    public:
      EventData() {
        init();
      }

      /// set member variables for next event.
      void next() {
        prev4Range = fourRange;

        // if mode is unknown, we always treat it as 4 range
        fourRange  = true;

        eventNum++;
      }

      bool     fourRange;

      unsigned eventNum;

      bool     prev4Range;
    } eventData;
  };

}; // namespace calibGenCAL
#endif
