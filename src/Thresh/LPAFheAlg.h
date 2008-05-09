#ifndef LPAFheAlg_h
#define LPAFheAlg_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/Thresh/LPAFheAlg.h,v 1.1 2008/04/21 20:43:13 fewtrell Exp $

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

  /** \brief Algorithm class fill & fit FHE Trigger threshold histograms.
      @author Zachary Fewtrell
  */
  class LPAFheAlg : public LPATrigAlg {
  public:

    /// @param trigType which trigger to calibrate (FHE or FHE)
    /// @param trigPattern which trigger channels are enabled
    /// @param intended trigger threshold in mev
    /// @param mev threshold for considering 'candidate' trigger channels (default=250 MeV)
    LPAFheAlg(const TriggerPattern trigPattern,
              const CalUtil::CalPed &peds,
              const CalUtil::ADC2NRG &adc2nrg,
              TrigHists &specHists,
              TrigHists &trigHists,
              const float expectedThresh=1000,
              const float safetyMargin=500) :
      LPATrigAlg(trigPattern, peds, adc2nrg, specHists, trigHists, expectedThresh, safetyMargin)
    {}

    /// Fill histograms w/ nEvt event data
    /// @param nEntrries minimum number of entries in each histogram before quitting
    /// @param digiFileList list of digi files to process
    void fillHists(const unsigned nEntries,
                   const std::vector<std::string> &digiFileList);

  private:
    void processEvent(const DigiEvent &digiEvent);

    /// check that layer matches expected configuraiton
    bool checkLyr(const CalUtil::LyrIdx lyrIdx, const CalUtil::FaceNum face);

    /// process all hits in single layer
    void processLyr(const CalUtil::LyrIdx lyrIdx, const CalUtil::FaceNum face);

  }; // class LPAFheAlg

}; // namespace calibGenCAL
#endif
