#ifndef IntNonlin_h
#define IntNonlin_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/IntNonlin.h,v 1.3 2006/08/03 13:06:48 fewtrell Exp $
/** @file
    @author fewtrell
 */

// LOCAL INCLUDES
#include "CGCUtil.h"
#include "RootFileAnalysis.h"
#include "CIDAC2ADC.h"

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
class IntNonlin {
 public:
  IntNonlin(ostream &ostrm = cout);
 
  /// process digi root event file
  /// \param diode specify whether to analyze HE or LE circuits
  void readRootData(const string &rootFileName,
                    DiodeNum diode,
                    bool bcastMode); 
  
  /// smooth raw adc means for use in offline spline calibration
  void genSplinePts(CIDAC2ADC &cidac2adc); 

  /// write pre-smoothed values to txt file
  void writeADCMeans(const string &filename) const;

  void readADCMeans(const string &filename);

  /// retrieve mean adc value for given CI DAC level.
  /// \return INVALID_ADC on error.
  double getADCMean(RngIdx rngIdx, unsigned short dacIdx) const {
    if (m_adcMean[rngIdx].size() < dacIdx)
      return CIDAC2ADC::INVALID_ADC;
    return m_adcMean[rngIdx][dacIdx];
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


 private:
  /// store mean ADC for each channel / DAC setting
  CalVec<RngIdx, vector<float> >  m_adcMean;

  ostream &m_ostrm;


};

#endif
