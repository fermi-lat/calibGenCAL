#ifndef TrigHists_h
#define TrigHists_h

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



namespace calibGenCAL {

  /** \brief Store histograms required to generate Calorimeter
      Trigger calibrations

      @author Z. Fewtrell

  */

  typedef HistVec<CalUtil::FaceIdx, TH1S> TrigHists;

}; // namespace calibGenCAL
#endif // TrigHists_h
