// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Util/TwrHodoscope.h,v 1.1 2007/03/27 18:50:51 fewtrell Exp $

/** @file
    @author fewtrell
*/

#ifndef TwrHodoscope_h
#define TwrHodoscope_h

// LOCAL INCLUDES

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalVec.h"
#include "CalUtil/CalArray.h"

// EXTLIB INCLUDES

// STD INCLUDES
#include <vector>

class CalPed;
class CIDAC2ADC;
class CalDigi;

/** \brief Accumulate crystal hits in single tower and summarize into
    information which can be used for track determination.
    @author fewtrell
*/
class TwrHodoscope {
 public:
  /// default ctor
  TwrHodoscope(const CalPed &ped,
               const CIDAC2ADC &cidac2adc) :
    adc_ped(CalUtil::tDiodeIdx::N_VALS),
    dac(CalUtil::tDiodeIdx::N_VALS),
    m_peds(ped),
    m_cidac2adc(cidac2adc)
    {
      clear();
    }

  /// 'zero-out' all members
  void clear();

  /// add new xtal hit to event summary data
  void addHit(const CalDigi &calDigi);

  /// summarize all hits added since last clear()
  void summarizeEvent();

  /// pedestal subtracted adc values 1 per diode
  CalUtil::CalVec<CalUtil::tDiodeIdx, float>          adc_ped;

  /// cidac values (1 per diode)
  CalUtil::CalVec<CalUtil::tDiodeIdx, float>          dac;

  // Hit summary histograms
  /// number of hits per X layer
  CalUtil::CalArray<CalUtil::GCRCNum, unsigned short> perLyrX;
  /// number of hits per Y layer
  CalUtil::CalArray<CalUtil::GCRCNum, unsigned short> perLyrY;
  /// # of hits per X column
  CalUtil::CalArray<CalUtil::ColNum, unsigned short>  perColX;
  /// # of hits per Y column
  CalUtil::CalArray<CalUtil::ColNum, unsigned short>  perColY;

  // Hit lists
  /// list of X direction xtalId's which were hit
  std::vector<CalUtil::XtalIdx>                       hitListX;
  /// list of Y direction xtalId's which were hit
  std::vector<CalUtil::XtalIdx>                       hitListY;

  // His summary
  /// total # of hit xtals
  unsigned count;
  /// total # of x layers hit
  unsigned short nLyrsX;
  /// total # of y layers hit
  unsigned short nLyrsY;
  /// total # of x columns hit
  unsigned short nColsX;
  /// total # of y columns hit
  unsigned short nColsY;
  /// max # of hits in any layer
  unsigned short maxPerLyr;
  /// max # of hits in any x layer
  unsigned short maxPerLyrX;
  ///  max # of hits in any y layer
  unsigned short maxPerLyrY;
  /// first hit X col (will be only hit col in good X track)
  unsigned short firstColX;
  /// fisrt hit Y col (will be only hit col in good Y track)
  unsigned short firstColY;

  /// reference to pedestal calibrations needed for some operations
  const CalPed &m_peds;

  /// reference to cidac2adc calibs needed for some ops
  const CIDAC2ADC &m_cidac2adc;

  /// threshold (LEX8 ADCP + ADCN) for couniting a 'hit' xtal
  static const unsigned short hitThresh = 100;
};

#endif
