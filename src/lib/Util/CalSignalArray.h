// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Util/CalSignalArray.h,v 1.1 2008/04/21 20:31:30 fewtrell Exp $

/** @file
    @author fewtrell
*/

#ifndef CalSignalArray_h
#define CalSignalArray_h

// LOCAL INCLUDES

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalVec.h"

// EXTLIB INCLUDES

// STD INCLUDES

class DigiEvent;
class CalDigi;

namespace CalUtil {
  class CalPed;
  class ADC2NRG;
}

namespace calibGenCAL {


  /** \brief Build array of Cal signal responses for each channel in cal

  @author fewtrell
  */
  class CalSignalArray {
  public:
    CalSignalArray(const CalUtil::CalPed &ped,
                   const CalUtil::ADC2NRG &adc2nrg) :
      m_peds(ped),
      m_adc2nrg(adc2nrg)
    {
      clear();
    }
    
    /// 'zero-out' all members
    void clear() {
      fill(m_faceSignal.begin(),
           m_faceSignal.end(),
           0);
      fill(m_adcPed.begin(),
           m_adcPed.end(),
           0);
      fill(m_adcRng.begin(),
           m_adcRng.end(),
           CalUtil::LEX8);
    }

    /// populate array with data from new event.
    void fillArray(const DigiEvent &digiEvent);

    /// get face signal (MeV) for given xtal face
    float getFaceSignal(const CalUtil::FaceIdx faceIdx) const {
      return m_faceSignal[faceIdx];
    }

    /// get pedestal subtracted adc value for given xtal face
    float getAdcPed(const CalUtil::FaceIdx faceIdx) const {
      return m_adcPed[faceIdx];
    }

    /// get adc range for given xtal face
    CalUtil::RngNum getAdcRng(const CalUtil::FaceIdx faceIdx) const {
      return m_adcRng[faceIdx];
    }

    /// represent one float value per xtal face
    typedef CalUtil::CalVec<CalUtil::FaceIdx, float> FaceSignalArray;

    const FaceSignalArray &getFaceSignalArray() {return m_faceSignal;}
    
  private:

    /// add new xtal hit to event summary data
    void addHit(const CalDigi &calDigi);

    /// store face signal value for each xtal face
    FaceSignalArray m_faceSignal;
    /// pedestal subtracted adc ranges
    FaceSignalArray m_adcPed;
    /// adc rng to go w/ m_adcPed
    CalUtil::CalVec<CalUtil::FaceIdx, CalUtil::RngNum> m_adcRng;

    const CalUtil::CalPed &m_peds;
    const CalUtil::ADC2NRG &m_adc2nrg;
  };

}; // namespace calibGenCAL
#endif
