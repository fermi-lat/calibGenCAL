#ifndef CalMPD_h
#define CalMPD_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/CalibDataTypes/CalMPD.h,v 1.3 2007/05/25 21:06:47 fewtrell Exp $

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalArray.h"
#include "CalUtil/CalVec.h"

// EXTLIB INCLUDES

// STD INCLUDES

namespace calibGenCAL {

  class CalAsym;
  class CIDAC2ADC;

  /** \brief Represents GLAST Cal Optical gain calibration constants
      (MeV <-> CIDAC)

      contains read & write methods to various file formats

      @author Zachary Fewtrell
  */
  class CalMPD {
  public:
    CalMPD();

    void writeTXT(const std::string &filename) const;

    void readTXT(const std::string &filename);

    float getMPD(const CalUtil::XtalIdx xtalIdx,
                 const CalUtil::DiodeNum diode) const {
      return m_mpd[diode][xtalIdx];
    }

    float getMPDErr(const CalUtil::XtalIdx xtalIdx,
                    const CalUtil::DiodeNum diode) const {
      return m_mpdErr[diode][xtalIdx];
    }

    void setMPD(const CalUtil::XtalIdx xtalIdx,
                const CalUtil::DiodeNum diode,
                float val) {
      m_mpd[diode][xtalIdx] = val;
    }

    void setMPDErr(const CalUtil::XtalIdx xtalIdx,
                   const CalUtil::DiodeNum diode,
                   const float val) {
      m_mpdErr[diode][xtalIdx] = val;
    }

    static const short INVALID_MPD;

  private:
    /// 2d vector N_MPD_PTS lograt mpdvals per xtal/mpdType
    CalUtil::CalVec<CalUtil::DiodeNum, CalUtil::CalArray<CalUtil::XtalIdx, float> > m_mpd;
    /// corresponding error value
    CalUtil::CalVec<CalUtil::DiodeNum, CalUtil::CalArray<CalUtil::XtalIdx, float> > m_mpdErr;
  };

}; // namespace calibGenCAL
#endif
