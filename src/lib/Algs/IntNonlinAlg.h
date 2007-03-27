#ifndef IntNonlinAlg_h
#define IntNonlinAlg_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/IntNonlinAlg.h,v 1.2 2007/02/27 20:44:13 fewtrell Exp $

/** @file
    @author fewtrell
 */

// LOCAL INCLUDES
#include "../Util/CGCUtil.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalVec.h"

// EXTLIB INCLUDES
#include "TObjArray.h"

// STD INCLUDES

class CIDAC2ADC;
class DigiEvent;
class CalDigi;
class TProfile;

/** \brief Algorithm class populates CIDAC2ADC calibration data
    object by analyzing calibGen singlex16 digi ROOT files.

    @author fewtrell
 */
class IntNonlinAlg {
public:
  IntNonlinAlg();

  /// process digi root event file
  /// \param diode specify whether to analyze HE or LE circuits
  void readRootData(const std::string &rootFileName,
                    CIDAC2ADC &adcMeans,
                    CalUtil::DiodeNum diode,
                    bool bcastMode);

  /// smooth raw adc means for use in offline spline calibration
  void genSplinePts(CIDAC2ADC &adcMeans,
                    CIDAC2ADC &cidac2adc);

private:
  /// fill histograms w/ data from single event
  void processEvent(const DigiEvent &digiEvent);

  /// fill histograms w/ data from single CalDigi hit
  void processHit(const CalDigi &cdig);

  /// apply smoothing algorithm to single ADC curve.
  void smoothSpline(const vector<float> &curADC,
                    vector<float> &splineADC,
                    vector<float> &splineDAC,
                    CalUtil::RngNum rng);

  /// store cfg & status data pertinent to current algorithm run
  struct AlgData {
    AlgData() :
      profiles(CalUtil::RngIdx::N_VALS, 0) {
      init();
    }

    void init() {
      diode     = CalUtil::LRG_DIODE;
      bcastMode = true;
      adcMeans  = 0;
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

    /// profiles owned by current ROOT directory/m_histFile.
    CalUtil::CalVec<CalUtil::RngIdx, TProfile *> profiles;

    /// create new histogram objects and accompanying TObjArray
    void initHists();

    /// currently processing 1 of 2 diodes
    CalUtil::DiodeNum diode;

    /// broadcast mode data samples all 12 columns
    /// simultaneously.
    bool              bcastMode;

    /// fill in the mean values for each DAC setting here.
    CIDAC2ADC        *adcMeans;
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
