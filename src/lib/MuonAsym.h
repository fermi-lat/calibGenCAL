#ifndef MuonAsym_h
#define MuonAsym_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/MuonAsym.h,v 1.10 2007/01/05 17:25:34 fewtrell Exp $

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
class DigiEvent;
class AsymHists;

/** \brief Algorithm class populates CalAsym calibration data with values extracted
    from Muon collection digi ROOT event files

    @author Zachary Fewtrell
 */
class MuonAsym {
public:
  MuonAsym(const CalPed &ped,
           const CIDAC2ADC &dac2adc,
           AsymHists &asymHists);

  /// populate asymmetry profiles w/ nEvt worth of data.
  void        fillHists(unsigned nEntries,
                        const vector<string> &rootFileList);

private:
  /// process a single event for histogram fill
  void        processEvent(DigiEvent &digiEvent);

  /// process a single tower's data in single event for
  /// histogram fill
  void        processTower(TwrHodoscope &hscope);

  /// hodoscopic event cut for X direction xtals
  bool        passCutX(const TwrHodoscope &hscope);

  /// hodoscopic event cut for Y direction xtals
  bool        passCutY(const TwrHodoscope &hscope);

  class AlgData {
private:
    void init() {
      nGoodDirs = 0;
      nXDirs    = 0;
      nYDirs    = 0;
      nHits     = 0;
      nBadHits  = 0;
    }

public:
    AlgData() {
      init();
    }

    unsigned nGoodDirs  ;   // count total # of events used
    unsigned nXDirs     ;
    unsigned nYDirs     ;
    long     nHits      ;   // count total # of xtals measured
    unsigned nBadHits   ;
  } algData;

  class EventData {
private:
    /// reset all member variables
    void init() {
      eventNum = 0;
      next();
    }

public:
    EventData(const CalPed &peds,
              const CIDAC2ADC &dac2adc) :
      hscopes(CalUtil::TwrNum::N_VALS,
              TwrHodoscope(peds, dac2adc)),
      eventNum(0)
    {
    }

    /// rest all member variables that do not retain data
    /// from one event to next.
    void next() {
      // clear all hodoscopes
      for (CalUtil::TwrNum twr; twr.isValid(); twr++)
        hscopes[twr].clear();
    }

    /// need one hodo scope per tower
    CalUtil::CalVec<CalUtil::TwrNum, TwrHodoscope> hscopes;

    unsigned eventNum;
  } eventData;

  /// histograms to fill
  AsymHists &m_asymHists;
};

#endif
