#ifndef LPATrigAlg_h
#define LPATrigAlg_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Algs/LPATrigAlg.h,v 1.5 2008/01/22 19:40:59 fewtrell Exp $

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "src/lib/Util/CalSignalArray.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalVec.h"

// EXTLIB INCLUDES

// STD INCLUDES
#include <vector>
#include <string>

class TH1F;
class DigiEvent;

namespace CalUtil {
  class CalPed;
  class ADC2NRG;
};

namespace calibGenCAL {
  class RootFileAnalysis;
  class TrigHists;

  /** \brief Abstract algorithm class fill & fit Trigger threshold histograms.
      @author Zachary Fewtrell
  */
  class LPATrigAlg {
  public:

    /// Specify which trigger channels have been enabled.
    typedef enum {
      EvenRowEvenCol, ///< even row (GCRC) even columns enabled
      EvenRowOddCol ///< even row (GCRC) odd columns enabled
    } TriggerPattern;

    /// return string repr of enum
    static std::string enum_str(const TriggerPattern trigPattern);

    /// @param trigType which trigger to calibrate (FLE or FHE)
    /// @param trigPattern which trigger channels are enabled
    /// @param expectedThresh intended trigger threshold in mev
    /// @param safetyMargin skip layers w/ > 1 channel where ene > thresh - margin (avoid ambiguous triggers)
    LPATrigAlg(const TriggerPattern trigPattern,
               const CalUtil::CalPed &peds,
               const CalUtil::ADC2NRG &adc2nrg,
               TrigHists &trigHists,
               const float expectedThresh,
               const float safetyMargin) :
      m_trigPattern(trigPattern),
      m_expectedThresh(expectedThresh),
      m_safetyMargin(safetyMargin),
      m_calPed(peds),
      m_trigHists(trigHists),
      eventData(peds, adc2nrg)
    {}

    virtual ~LPATrigAlg() {}

    /// Fill histograms w/ nEvt event data
    /// @param nEntrries minimum number of entries in each histogram before quitting
    /// @param digiFileList list of digi files to process
    virtual void fillHists(const unsigned nEntries,
                           const std::vector<std::string> &digiFileList) = 0;

  protected:
    /// setup needed branches for input ROOT files
    virtual void cfgBranches(RootFileAnalysis &rootFile);

    /// check that channel is enabled for given LAT configuration
    virtual bool channelEnabled(const CalUtil::FaceIdx faceIdx);

    /// fill eventData.m_diagTrigBits with diagnostic data from given event
    virtual void fillTrigBitArray(const DigiEvent &digiEvent);
    
    /// return list of possible trigger candidate crystals (mev > m_expectedThresh - m_safetyMargin) on given Cal lyr & face
    virtual std::vector<CalUtil::ColNum> countTrigCandidates(const CalUtil::LyrIdx lyrIdx,
                                                             const CalUtil::FaceNum face);

    /// process single digi event for pedestal data
    virtual void processEvent(const DigiEvent &digiEvt) = 0;

    /// check that layer matches expected configuraiton
    virtual bool checkLyr(const CalUtil::LyrIdx lyrIdx, const CalUtil::FaceNum face) = 0;

    /// process all hits in single layer
    virtual void processLyr(const CalUtil::LyrIdx lyrIdx, const CalUtil::FaceNum face) = 0;

    /// which trigger channels are enabled?
    const TriggerPattern m_trigPattern;

    /// expected trigger threshold
    const float m_expectedThresh;
    
    /// safety margin (for avoiding ambigous triggers) min MeV
    const float m_safetyMargin;
    
    /// pedestal calibration data
    const CalUtil::CalPed &m_calPed;

    // output histograms
    TrigHists &m_trigHists;
    
    /// store cfg & status data pertinent to current algorithm run
    struct EventData {
      EventData(const CalUtil::CalPed &peds,
                const CalUtil::ADC2NRG &adc2nrg) :
        m_calSignalArray(peds, adc2nrg)
      {
        init();
      }
      
      /// intialize to beginning of alg
      void init() {
        m_eventNum = 0;
        clear();
      }

      /// clear out arrays
      void clear();

      /// setup data for next event
      void nextEvent() {
        m_eventNum++;
        clear();
      }

      /// face signal array from cal tuple
      CalSignalArray m_calSignalArray;

      /// store trigger bits from diagnostic data, one trigger bit for
      /// each (tower,layer, face, diode) tuple
      CalUtil::CalVec<CalUtil::LyrIdx, 
                      CalUtil::CalVec<CalUtil::XtalDiode, bool> >  m_diagTrigBits;

      unsigned m_eventNum;
      
    } eventData;

    
  }; // class LPATrigAlg

}; // namespace calibGenCAL
#endif
