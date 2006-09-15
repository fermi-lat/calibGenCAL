#ifndef CalMPD_h
#define CalMPD_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/CalMPD.h,v 1.2 2006/06/22 21:50:23 fewtrell Exp $
/** @file
    @author Zachary Fewtrell
 */


// LOCAL INCLUDES
#include "CGCUtil.h"
#include "CalAsym.h"
#include "CIDAC2ADC.h"

// GLAST INCLUDES
#include "CalUtil/CalVec.h"
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalArray.h"


// EXTLIB INCLUDES


// STD INCLUDES
#include <string>
#include <memory>

using namespace std;
using namespace CalUtil;

/** \brief Represents GLAST Cal Optical gain calibration constants
    (MeV <-> CIDAC)

    contains read & write methods to various file formats & code
    to calculate calibrations from digi ROOT event files

    @author Zachary Fewtrell
*/
class CalMPD {
 public:
  CalMPD();

  void writeTXT(const string &filename) const;
  
  void readTXT(const string &filename);

  /// write ADC2MEV (4-range) factors to txt file
  void writeADC2NRG(const string &filename,
                    const CalAsym &asym,
                    const CIDAC2ADC &dac2adc);

  float getMPD(XtalIdx xtalIdx, DiodeNum diode) const {
    return m_mpd[diode][xtalIdx];
  }
  
  float getMPDErr(XtalIdx xtalIdx, DiodeNum diode) const {
    return m_mpdErr[diode][xtalIdx];
  }

  void setMPD(XtalIdx xtalIdx, DiodeNum diode, float val) {
    m_mpd[diode][xtalIdx] = val;
  }
  void setMPDErr(XtalIdx xtalIdx, DiodeNum diode, float val) {
    m_mpdErr[diode][xtalIdx] = val;
  }


  static const short INVALID_MPD = -5000;
  
 private:
  /// 2d vector N_MPD_PTS lograt mpdvals per xtal/mpdType
  CalVec<DiodeNum, CalArray<XtalIdx, float> > m_mpd; 
  /// corresponding error value
  CalVec<DiodeNum, CalArray<XtalIdx, float> > m_mpdErr; 

};

#endif
