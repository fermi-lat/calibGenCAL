#ifndef NeighborXtalkAlg_h
#define NeighborXtalkAlg_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/CIDAC2ADC/NeighborXtalkAlg.h,v 1.2 2008/05/02 17:59:33 fewtrell Exp $

/** @file
    @author fewtrell
*/

// LOCAL INCLUDES
#include "src/lib/Specs/singlex16.h"

// GLAST INCLUDES
#include "CalUtil/SimpleCalCalib/NeighborXtalk.h"
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalVec.h"

// EXTLIB INCLUDES
#include "TObjArray.h"

// STD INCLUDES

class DigiEvent;
class CalDigi;

namespace calibGenCAL {

  /** \brief Algorithm class populates NeighborXtalk calibration data
      object by analyzing calibGen singlex16 digi ROOT files.

      @author fewtrell
  */
  class NeighborXtalkAlg {
  public:
    /// @param sx16 spec describing LCI script procedure
    /// \param altLoopScheme - enable alternate LCI loop scheme used in run #077015240
    NeighborXtalkAlg(const singlex16 &sx16,
                     const bool altLoopScheme=false) :
      eventData(altLoopScheme, sx16),
      m_singlex16(sx16)
    {}

    /// process digi root event file
    void readRootData(const std::string &rootFileName,
                      CalUtil::NeighborXtalk &xtalk);

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
        delete adcHists;
      }

      /// create one temporary histogram per adc channel.
      /// this histogram will be reused for each new CIDAC
      /// level.
      TObjArray                                   *adcHists;

      /// create new histogram objects and accompanying TObjArray
      void initHists();

      /// fill in the mean values for each DAC setting here.
	  CalUtil::NeighborXtalk        *xtalk;
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
      /// \param altLoopScheme - enable alternate LCI loop scheme used in run #077015240
      EventData(const bool altLoopScheme,
                const singlex16 &sx16) :
        m_altLoopScheme(altLoopScheme) ,
        m_singlex16(sx16)
      {
        init();
      }

      /// set internal object state to match next event id.
      void nextEvent();

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

      /// enable alternate LCI loop scheme used in run #077015240
      const bool m_altLoopScheme;

      const singlex16 &m_singlex16;
    } eventData;

    const singlex16 &m_singlex16;
  };

}; // namespace calibGenCAL
#endif
