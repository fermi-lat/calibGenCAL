#ifndef CIDAC2ADC_h
#define CIDAC2ADC_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/CIDAC2ADC.h,v 1.8 2007/01/05 17:25:33 fewtrell Exp $

/** @file
    @author fewtrell
*/

// LOCAL INCLUDES
#include "CGCUtil.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalVec.h"

// EXTLIB INCLUDES
#include "TSpline.h"

// STD INCLUDES

/** \brief Represents GLAST Cal CIDAC (Charge Injection DAC) <-> ADC
    calibration constants

    contains read & write methods to various file formats

    @author fewtrell
*/
class CIDAC2ADC {
 public:
  CIDAC2ADC();

  /// write calibrations to txt file
  void writeTXT(const std::string &filename) const;

  void readTXT(const std::string &filename);

  /// get series of charge injection DAC values used for given energy range in
  const std::vector<float> & getPtsDAC(CalUtil::RngIdx rngIdx) const
    {
      return m_splinePtsDAC[rngIdx];
    }

  /// get series of measured ADC values for given channel
  const std::vector<float> & getPtsADC(CalUtil::RngIdx rngIdx) const
    {
      return m_splinePtsADC[rngIdx];
    }

  /// get series of charge injection DAC values used for given energy range in
  std::vector<float> & getPtsDAC(CalUtil::RngIdx rngIdx)
    {
      return m_splinePtsDAC[rngIdx];
    }

  /// get series of measured ADC values for given channel
  std::vector<float> & getPtsADC(CalUtil::RngIdx rngIdx)
    {
      return m_splinePtsADC[rngIdx];
    }

  /// creates & populates INL splines from m_calIntNonlin;
  void genSplines();

  /// uses intNonlin to convert adc 2 dac for specified xtal/adc range
  /// \warning you _must_ call genSplines method before using.
  float adc2dac(CalUtil::RngIdx rngIdx,
                float adc) const {
    if (!m_splinesADC2DAC[rngIdx])
      return INVALID_ADC;

    return m_splinesADC2DAC[rngIdx]->Eval(adc);
  }

  /// uses intNonlin to convert dac 2 adc for specified xtal/adc range
  /// \warning you _must_ call genSplines method before using.
  float dac2adc(CalUtil::RngIdx rngIdx,
                float dac) const {
    if (!m_splinesDAC2ADC[rngIdx])
      return INVALID_ADC;

    return m_splinesDAC2ADC[rngIdx]->Eval(dac);
  }

  static const short INVALID_ADC;

  /// pedestal subtract spline point ADC by using value from first point
  void pedSubtractADCSplines();

  static std::string genFilename(const std::string outputDir,
                                 const std::string &inFilename,
                                 const std::string &ext) {
    return CGCUtil::genOutputFilename(outputDir,
                                      "cidac2adc",
                                      inFilename,
                                      ext);
  }

 private:
  /// output ADC spline points
  CalUtil::CalVec<CalUtil::RngIdx, vector<float> >  m_splinePtsADC;
  /// output DAC spline points
  CalUtil::CalVec<CalUtil::RngIdx, vector<float> >  m_splinePtsDAC;

  /// store optional ROOT spline objects for spline evaluation
  CalUtil::CalVec<CalUtil::RngIdx, TSpline3 *>      m_splinesADC2DAC;
  /// store optional ROOT spline objects for spline evaluation
  CalUtil::CalVec<CalUtil::RngIdx, TSpline3 *>      m_splinesDAC2ADC;
};

#endif
