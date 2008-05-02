#ifndef IntNonlinAlg_h
#define IntNonlinAlg_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/CIDAC2ADC/IntNonlinAlg.h,v 1.4 2008/05/02 17:59:33 fewtrell Exp $

/** @file
    @author fewtrell
*/

// LOCAL INCLUDES
#include "src/lib/Specs/singlex16.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalVec.h"

// EXTLIB INCLUDES
#include "TObjArray.h"

// STD INCLUDES

class DigiEvent;
class CalDigi;
class TProfile;

namespace CalUtil {
  class CIDAC2ADC;
}

class TNtuple;

namespace calibGenCAL {


  /** \brief Algorithm class populates CIDAC2ADC calibration data
      object by analyzing calibGen singlex16 digi ROOT files.

      @author fewtrell
  */
  class IntNonlinAlg {
  public:
    /// @param sx16 spec describing LCI script procedure
    IntNonlinAlg(const singlex16 &sx16) :
      m_singlex16(sx16)
    {}

    /// process digi root event file
    /// \param diode specify whether to analyze HE or LE circuits
    void readRootData(const std::string &rootFileName,
                      CalUtil::CIDAC2ADC &adcMeans,
                      const CalUtil::DiodeNum diode,
                      const bool bcastMode);

    /// smooth raw adc means for use in offline spline calibration
    static void genSplinePts(const CalUtil::CIDAC2ADC &adcMeans,
                             CalUtil::CIDAC2ADC &cidac2adc);

  private:
    /// fill histograms w/ data from single event
    void processEvent(const DigiEvent &digiEvent);

    /// fill histograms w/ data from single CalDigi hit
    void processHit(const CalDigi &cdig);

    /// check that LCI configuration matches expected.
    bool checkLCICfg(const DigiEvent &digiEvent);

    /// apply smoothing algorithm to single ADC curve.
    static void smoothSpline(const vector<float> &curADC,
                             vector<float> &splineADC,
                             vector<float> &splineDAC,
                             const CalUtil::RngNum rng);

    /// store cfg & status data pertinent to current algorithm run
    struct AlgData {
      AlgData()  {
        init();
      }

      void init() {
        diode     = CalUtil::LRG_DIODE;
        bcastMode = true;
        adcMeans  = 0;
        adcHists = 0;
        m_fitResults = 0;
        initHists();
      }


      /// create one temporary histogram per adc channel.
      /// this histogram will be reused for each new CIDAC
      /// level.
      TObjArray *adcHists;

      /// profiles owned by current ROOT directory/m_histFile.
      CalUtil::CalVec<CalUtil::RngIdx, TProfile *> profiles;

      /// ntuple keeps track of fit results
      TNtuple *m_fitResults;

      /// create new histogram objects and accompanying TObjArray
      /// create TNtuple object for storing fit results
      void initHists();

      /// currently processing 1 of 2 diodes
      CalUtil::DiodeNum diode;

      /// broadcast mode data samples all 12 columns
      /// simultaneously.
      bool              bcastMode;

      /// fill in the mean values for each DAC setting here.
      CalUtil::CIDAC2ADC        *adcMeans;
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

      /// current CIDAC index
      unsigned short  testDAC;


    } eventData;

    const singlex16 &m_singlex16;
  };
}; // namespace calibGenCAL
#endif
