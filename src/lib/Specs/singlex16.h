#ifndef singlex16_h
#define singlex16_h

// $Header: $

/** @file Specification of online 'singlex16' LCI script data
    @author fewtrell
*/

// LOCAL INCLUDES

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"

// EXTLIB INCLUDES

// STD INCLUDES

namespace calibGenCAL {

  class singlex16 {
  public:
    singlex16( unsigned nPulsesPerDAC=100) :
      nPulsesPerDAC(nPulsesPerDAC) 
    {}

    /// sequence if charge injection DAC test values
    static const float CIDAC_TEST_VALS[];

    /// number of CIDAC values tested
    static const unsigned short N_CIDAC_VALS;

    /// n pulses (events) per CIDAC val
    unsigned short nPulsesPerDAC;

    /// n total pulsees per xtal (or column)
    unsigned nPulsesPerXtal() const {return N_CIDAC_VALS * nPulsesPerDAC;}

    /// total number of pulses in broadcast mode singlex16
    unsigned totalPulsesBCAST() const {return nPulsesPerXtal();}

    /// total number of pulses in column-wise singlex16
    unsigned totalPulsesCOLWISE() const {return nPulsesPerXtal()*CalUtil::ColNum::N_VALS;}

    /// maximum CIDAC integer value
    static const unsigned short MAX_DAC = 4095;

    /// maximum ADC integer value
    static const unsigned short MAX_ADC = 4095;
  };

}; // namespace calibGenCAL
#endif
