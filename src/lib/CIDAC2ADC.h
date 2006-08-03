#ifndef CIDAC2ADC_h
#define CIDAC2ADC_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/CIDAC2ADC.h,v 1.2 2006/06/22 21:50:22 fewtrell Exp $
/** @file
    @author fewtrell
 */

// LOCAL INCLUDES
#include "CGCUtil.h"
#include "RootFileAnalysis.h"

// GLAST INCLUDES
#include "CalUtil/CalVec.h"
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalArray.h"

// EXTLIB INCLUDES
#include "TFile.h"
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
  CIDAC2ADC(ostream &ostrm = cout);
 
  /// process digi root event file
  /// \param diode specify whether to analyze HE or LE circuits
  void readRootData(const string &rootFileName,
                    DiodeNum diode,
                    bool bcastMode); 
  
  /// smooth raw adc means for use in offline spline calibration
  void genSplinePts(); 
  
  /// write calibrations to txt file
  void writeTXT(const string &filename) const; 

  /// write pre-smoothed values to txt file
  void writeADCMeans(const string &filename) const;

  void readADCMeans(const string &filename);
  
  void readTXT(const string &filename);

  void makeGraphs(TFile &histFile);

  /// get series of charge injection DAC values used for given energy range in
  const vector<float>& getSplineDAC(RngIdx rngIdx) const 
    {return m_splinePtsDAC[rngIdx];}

  /// get series of measured ADC values for given channel
  const vector<float>& getSplineADC(RngIdx rngIdx) const 
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

  static void genOutputFilename(const string outputDir,
                                const string &inFilename,
                                const string &ext,
                                string &outFilename) {
    CGCUtil::genOutputFilename(outputDir,
                               "cidac2adc",
                               inFilename,
                               ext,
                               outFilename);
  }

  /// retrieve mean adc value for given CI DAC level.
  /// \return INVALID_ADC on error.
  double getADCMean(RngIdx rngIdx, unsigned short dacIdx) const {
    if (m_adcMean[rngIdx].size() < dacIdx)
      return INVALID_ADC;
    return m_adcMean[rngIdx][dacIdx];
  }

 private:
  /// store mean ADC for each channel / DAC setting
  CalVec<RngIdx, vector<float> >  m_adcMean;
  /// output ADC spline points
  CalVec<RngIdx, vector<float> >  m_splinePtsADC;
  /// output DAC spline points
  CalVec<RngIdx, vector<float> >  m_splinePtsDAC;

  CalVec<RngIdx, TSpline3*> m_splinesADC2DAC; 
  CalVec<RngIdx, TSpline3*> m_splinesDAC2ADC; 


  /// pedestal subtract spline point ADC by using value from first point
  void pedSubtractADCSplines();

  ostream &m_ostrm;

  static const short INVALID_ADC = -5000;

};

#endif
