#ifndef ADC2NRG_H
#define ADC2NRG_H

// $Header: //

/** @file
        @author Zachary Fewtrell
 */

// LOCAL INCLUDES
#include "CGCUtil.h"

// GLAST INCLUDES

// EXTLIB INCLUDES

// STD INCLUDES
#include <ostream>
#include <string>

class CalMPD;
class CalAsym;
class CIDAC2ADC;

/** \brief class for manipulating Cal ADC2NRG data
 */

class ADC2NRG {
public:
  static void        writeTXT(const std::string &filename,
                              const CalAsym &asym,
                              const CIDAC2ADC &dac2adc,
                              const CalMPD &calMPD);

  static std::string genFilename(const std::string outputDir,
                                 const std::string &inFilename,
                                 const std::string &ext) {
    return CGCUtil::genOutputFilename(outputDir,
                                      "adc2nrg",
                                      inFilename,
                                      ext);
  }
};

#endif // ADC2NRG_H
