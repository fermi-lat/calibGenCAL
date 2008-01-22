#ifndef GCRFit_h
#define GCRFit_h

// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Hists/GCRFit.h,v 1.2 2007/06/13 22:42:12 fewtrell Exp $

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES

// GLAST INCLUDES

// EXTLIB INCLUDES

// STD INCLUDES
#include <string>

class TDirectory;

namespace CalUtil {
  class CalMPD;
}

namespace calibGenCAL {
  class GCRHists;

  /** \brief Collection of tools for fitting GCR calib histograms
   */
  namespace GCRFit {
    /// fit GCR histograms & output fitting results to CalMPD obj w/ simple Gaussian peakshape
    /// \parm calMPD output calibration constants
    /// \parm writeFile location for output fit results tuple
    /// \parm tupleName output tuple fit results name
    void gcrFitGaus(GCRHists &histCol,
			CalUtil::CalMPD &calMPD,
                    TDirectory *const writeFile,
                    const std::string &tupleName="GCRFitGauss"
                    );
  } // namespace GCRFit

} // namespace calibGenCAL 

#endif
