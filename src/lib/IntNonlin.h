#ifndef IntNonlin_h
#define IntNonlin_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/IntNonlin.h,v 1.2 2006/09/19 13:19:32 fewtrell Exp $
/** @file
    @author fewtrell
*/

// LOCAL INCLUDES
#include "CGCUtil.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"


// EXTLIB INCLUDES

// STD INCLUDES

class CIDAC2ADC;


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
  void readRootData(const std::string &rootFileName,
                    CIDAC2ADC &adcMeans,
                    CalUtil::DiodeNum diode,
                    bool bcastMode); 
  
  /// smooth raw adc means for use in offline spline calibration
  void genSplinePts(CIDAC2ADC &adcMeans, CIDAC2ADC &cidac2adc); 

  static void genOutputFilename(const std::string outputDir,
                                const std::string &inFilename,
                                const std::string &ext,
                                std::string &outFilename) {
    CGCUtil::genOutputFilename(outputDir,
                               "cidac2adc",
                               inFilename,
                               ext,
                               outFilename);
  }


 private:
  std::ostream &m_ostrm;


};

#endif