#ifndef IntNonlin_h
#define IntNonlin_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/IntNonlin.h,v 1.1 2006/09/15 15:02:10 fewtrell Exp $
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
                    CIDAC2ADC &adcMeans,
                    DiodeNum diode,
                    bool bcastMode); 
  
  /// smooth raw adc means for use in offline spline calibration
  void genSplinePts(CIDAC2ADC &adcMeans, CIDAC2ADC &cidac2adc); 

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
  ostream &m_ostrm;


};

#endif
