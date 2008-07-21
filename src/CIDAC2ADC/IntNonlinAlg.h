#ifndef IntNonlinAlg_h
#define IntNonlinAlg_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/CIDAC2ADC/IntNonlinAlg.h,v 1.7 2008/05/14 18:39:46 fewtrell Exp $

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
class TGraph;

namespace CalUtil {
  class CIDAC2ADC;
}

class TNtuple;
class TTree;

namespace calibGenCAL {


  /** \brief Algorithm class populates CIDAC2ADC calibration data
      object by analyzing calibGen singlex16 digi ROOT files.

      @author fewtrell
  */
  class IntNonlinAlg {
  public:
    /// @param sx16 spec describing LCI script procedure
    IntNonlinAlg(const singlex16 &sx16,
                 const bool hugeTuple=false);

    /// process digi root event file
    /// \param diode specify whether to analyze HE or LE circuits
    void readRootData(const std::string &rootFileName,
                      CalUtil::CIDAC2ADC &adcMeans,
                      const CalUtil::DiodeNum diode,
                      const bool bcastMode);

  private:
    /// fill histograms w/ data from single event
    void processEvent(const DigiEvent &digiEvent);

    /// fill histograms w/ data from single CalDigi hit
    void processHit(const CalDigi &cdig);

    /// check that LCI configuration matches expected.
    bool checkLCICfg(const DigiEvent &digiEvent);

    /// store cfg & status data pertinent to current algorithm run
    struct AlgData {
      AlgData()  {
        init();
      }

      /// one time initialization
      void init() {
        diode     = CalUtil::LRG_DIODE;
        bcastMode = true;
        adcMeans  = 0;
        adcHists = 0;
        initHists();
      }


      /// create one temporary histogram per adc channel.
      /// this histogram will be reused for each new CIDAC
      /// level.
      TObjArray *adcHists;

      /// profiles owned by current ROOT directory/m_histFile.
      CalUtil::CalVec<CalUtil::RngIdx, TProfile *> profiles;

      /// graph all samples for given dac setting & channel
      CalUtil::CalVec<CalUtil::RngIdx, TGraph *> noiseGraphs;

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
    
    /// ntuple keeps track of fit results
    TNtuple *m_fitResults;

    /// keep track of every single readout for noise studies (optional)
    TTree *m_hugeTuple;

    struct HugeTupleData {
      /// channel index
      unsigned short rngIdx;
      /// event sequence number (skips invalid events)
      unsigned short goodEvt;
      /// current charge injection value
      unsigned short cidac;
      /// adc readout value
      unsigned short adc;
    } m_tupleData;
  };
}; // namespace calibGenCAL
#endif
