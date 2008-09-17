#ifndef CalResponse_H
#define CalResponse_H

//$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Specs/CalResponse.h,v 1.4 2008/04/18 21:46:26 chehtman Exp $

// LOCAL INCLUDES=

// GLAST INCLUDES

// EXTLIB INCLUDES

// STD INCLUDES

/** @file CalResponse.h
    @author Zachary Fewtrell
    \brief GLAST Calorimeter CsI response coefficients.
*/

namespace calibGenCAL {

  namespace CalResponse {
    /// Cal is usually in either FLIGHT_GAIN (hegain=15) or MUON_GAIN
    typedef enum {
      MUON_GAIN,
      FLIGHT_GAIN
    } CAL_GAIN_INTENT;

    /// most probable energy deposit for vertical muon MIP travelling through crystal.
    static const float CsIMuonPeak = 11.2;
    
    /// nominal most probable energy deposit for ion of given Z travelling veritcally through CsI crystal
    inline float nominalZPeakMeV(const unsigned short Z) {
      return CsIMuonPeak*Z*Z;
    }

    /// nominal MeVPerDAC coefficients (flight gain)
    static const float nominalMPD[CalUtil::DiodeNum::N_VALS] = {.36, 21};
    
  }

}; // namespace calibGenCAL
#endif // CalResponse_H
