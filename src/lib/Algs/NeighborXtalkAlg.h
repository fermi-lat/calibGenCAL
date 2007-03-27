#ifndef NeighborXtalkAlg_h
#define NeighborXtalkAlg_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/NeighborXtalkAlg.h,v 1.2 2007/02/27 20:44:13 fewtrell Exp $

/** @file
    @author fewtrell
 */

// LOCAL INCLUDES
#include "../CalibDataTypes/NeighborXtalk.h"
#include "../Util/CGCUtil.h"


// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalVec.h"

// EXTLIB INCLUDES

// STD INCLUDES

class DigiEvent;
class CalDigi;

/** \brief Algorithm class populates NeighborXtalk calibration data
    object by analyzing calibGen singlex16 digi ROOT files.

    @author fewtrell
 */
class NeighborXtalkAlg {
public:
  NeighborXtalkAlg();

  /// process digi root event file
  void readRootData(const std::string &rootFileName,
                    NeighborXtalk &xtalk);

private:
  /// fill histograms w/ data from single event
  void processEvent(const DigiEvent &digiEvent);

  /// fill histograms w/ data from single CalDigi hit
  void processHit(const CalDigi &cdig);

  /// store cfg & status data pertinent to current algorithm run
  struct AlgData {
    AlgData()
    {
      init();
    }

    void init() {
      xtalk  = 0;
      initHists();
    }

    ~AlgData() {
      if (adcHists)
        delete adcHists;
    }

    /// create one temporary histogram per adc channel.
    /// this histogram will be reused for each new CIDAC
    /// level.
    TObjArray                                   *adcHists;

    /// create new histogram objects and accompanying TObjArray
    void initHists();

    /// fill in the mean values for each DAC setting here.
    NeighborXtalk        *xtalk;
  } algData;

  /// store data pertinent to current event
  struct EventData {
private:

    void            init() {
      eventNum = 0;
      iGoodEvt = 0;
      testCol  = 0;
      iSamp    = 0;
      testDAC  = 0;
    }

public:
    EventData() {
      init();
    }

    /// count events read from root file
    unsigned        eventNum;

    /// count 'good' events actually used from root file
    unsigned        iGoodEvt;

    /// which xtal column is singlex16 currently sampling?
    CalUtil::ColNum testCol;

    /// how many samples @ current setting?
    unsigned short  iSamp;

    // current CIDAC index
    unsigned short  testDAC;
  } eventData;
};

#endif
