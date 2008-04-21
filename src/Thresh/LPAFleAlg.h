#ifndef LPAFleAlg_h
#define LPAFleAlg_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Algs/LPAFleAlg.h,v 1.5 2008/01/22 19:40:59 fewtrell Exp $

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "src/lib/Algs/LPATrigAlg.h"

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

  /** \brief Algorithm class fill & fit FLE Trigger threshold histograms.
      @author Zachary Fewtrell
  */
  class LPAFleAlg : public LPATrigAlg {
  public:

    /// @param trigType which trigger to calibrate (FLE or FHE)
    /// @param face which xtal face to calibrate
    /// @param trigPattern which trigger channels are enabled
    /// @param expecteThresh expected trigger threshold in MeV
    /// @param safetyMargin cut when > 1 xtal has ene > thresh - margin (avoid ambiguous triggers)
    LPAFleAlg(const CalUtil::FaceNum face,
              const TriggerPattern trigPattern,
              const CalUtil::CalPed &peds,
              const CalUtil::ADC2NRG &adc2nrg,
              TrigHists &trigHists,
              const float expectedThresh=100,
              const float safetyMargin=50) :
      LPATrigAlg(trigPattern, peds, adc2nrg, trigHists, expectedThresh, safetyMargin),
      m_face(face)
    {}

    /// Fill histograms w/ nEvt event data
    /// @param nEntrries minimum number of entries in each histogram before quitting
    /// @param digiFileList list of digi files to process
    void fillHists(const unsigned nEntries,
                   const std::vector<std::string> &digiFileList);

  private:
    void processEvent(const DigiEvent &digiEvent);

    /// check that layer matches expected configuraiton
    bool checkLyr(const CalUtil::LyrIdx lyrIdx, const CalUtil::FaceNum faceNum);

    /// process all hits in single layer
    void processLyr(const CalUtil::LyrIdx lyrIdx, const CalUtil::FaceNum face);

    /// check that channel is enabled for given LAT configuration
    bool channelEnabled(const CalUtil::FaceIdx faceIdx);

    const CalUtil::FaceNum m_face;

  }; // class LPAFleAlg

}; // namespace calibGenCAL
#endif
