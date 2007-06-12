#ifndef GCRFit_h
#define GCRFit_h

// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Hists/GCRHists.h,v 1.6 2007/06/07 17:45:43 fewtrell Exp $

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES

// GLAST INCLUDES

// EXTLIB INCLUDES

// STD INCLUDES
#include <string>

class TDirectory;

namespace calibGenCAL {
  class GCRHists;
  class CalMPD;

  /** \brief Responsible for fitting a collection of GCR histograms 
   */
  class GCRFit {
  public:
    /// fit GCR histograms & output fitting results to CalMPD obj
    /// \parm calMPD output calibration constants
    /// \parm writeFile location for output fit results tuple
    /// \parm tupleName output tuple fit results name
    static void fitHists(GCRHists &histCol,
                         CalMPD &calMPD,
                         TDirectory *const writeFile,
                         const std::string &tupleName="GCRFit"
                         );
  };

} // namespace calibGenCAL 

#endif
