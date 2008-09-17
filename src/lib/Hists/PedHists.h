#ifndef PedHists_h
#define PedHists_h

// $Header: //

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "HistVec.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"

// EXTLIB INCLUDES
#include "TH1S.h"

// STD INCLUDES

namespace CalUtil {
  class CalPed;
};


namespace calibGenCAL {

  /** \brief Store histograms required to generate Calorimeter
      Pedestal calibrations

      @author Z. Fewtrell

  */

  class PedHists : public HistVec<CalUtil::RngIdx, TH1S> {
  public:
    PedHists(TDirectory *writeDir=0,
             TDirectory *readDir=0) :
      HistVec<CalUtil::RngIdx, TH1S>("calPed",
                                     writeDir,
                                     readDir,
                                     1000,
                                     0.5,
                                     1000.5)
    {}

    /// Fit histograms & save results to pedestal collection object
    void     fitHists(CalUtil::CalPed &peds);

  };
  
} // namespace calibGenCAL
#endif // PedHists_h
