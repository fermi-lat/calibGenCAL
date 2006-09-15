#ifndef CIDAC2ADC_h
#define CIDAC2ADC_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/CIDAC2ADC.h,v 1.3 2006/08/03 13:06:48 fewtrell Exp $
/** @file
    @author fewtrell
 */

// LOCAL INCLUDES
#include "CGCUtil.h"

// GLAST INCLUDES
#include "CalUtil/CalVec.h"
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalArray.h"

// EXTLIB INCLUDES
#include "TSpline.h"

// STD INCLUDES

using namespace std;
using namespace CalUtil;

/** \brief Represents GLAST Cal CIDAC (Charge Injection DAC) <-> ADC 
    calibration constants

    contains read & write methods to various file formats & code
    to calculate calibrations from digi ROOT event files

    @author fewtrell
*/
class CIDAC2ADC {
 public:
  CIDAC2ADC();

  /// write calibrations to txt file
  void writeTXT(const string &filename) const; 

  void readTXT(const string &filename);

  /// get series of charge injection DAC values used for given energy range in
  const vector<float>& getPtsDAC(RngIdx rngIdx) const 
    {return m_splinePtsDAC[rngIdx];}

  /// get series of measured ADC values for given channel
  const vector<float>& getPtsADC(RngIdx rngIdx) const 
    {return m_splinePtsADC[rngIdx];}

  /// get series of charge injection DAC values used for given energy range in
  vector<float>& getPtsDAC(RngIdx rngIdx)
    {return m_splinePtsDAC[rngIdx];}

  /// get series of measured ADC values for given channel
  vector<float>& getPtsADC(RngIdx rngIdx)
    {return m_splinePtsADC[rngIdx];}

  /// creates & populates INL splines from m_calIntNonlin;
  void genSplines(); 

  /// uses intNonlin to convert adc 2 dac for specified xtal/adc range
  float adc2dac(RngIdx rngIdx, float adc) const {
    if (!m_splinesADC2DAC[rngIdx])
      return INVALID_ADC;

    return m_splinesADC2DAC[rngIdx]->Eval(adc);
  }

  /// uses intNonlin to convert dac 2 adc for specified xtal/adc range
  float dac2adc(RngIdx rngIdx, float dac) const {
    if (!m_splinesDAC2ADC[rngIdx])
      return INVALID_ADC;

    return m_splinesDAC2ADC[rngIdx]->Eval(dac);
  }

  static const short INVALID_ADC = -5000;

  /// pedestal subtract spline point ADC by using value from first point
  void pedSubtractADCSplines();

 private:
  /// output ADC spline points
  CalVec<RngIdx, vector<float> >  m_splinePtsADC;
  /// output DAC spline points
  CalVec<RngIdx, vector<float> >  m_splinePtsDAC;

  CalVec<RngIdx, TSpline3*> m_splinesADC2DAC; 
  CalVec<RngIdx, TSpline3*> m_splinesDAC2ADC; 
};

#endif
