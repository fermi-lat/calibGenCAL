// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/CalHodoscope.h,v 1.3 2007/02/27 20:44:13 fewtrell Exp $

/** @file
    @author fewtrell
 */

#ifndef CalHodoscope_h
#define CalHodoscope_h

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

/** \brief Accumulate crystal hits in Cal summarize into
    information which can be used for track determination.
    @author fewtrell
 */
class CalHodoscope {
public:
  CalHodoscope(const CalPed &ped,
               const CIDAC2ADC &cidac2adc) :
    adc_ped(CalUtil::DiodeIdx::N_VALS),
    dac(CalUtil::DiodeIdx::N_VALS),
    bestRng(CalUtil::FaceIdx::N_VALS),
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
  CalUtil::CalVec<CalUtil::DiodeIdx, float>          adc_ped;

  /// cidac values (1 per diode)
  CalUtil::CalVec<CalUtil::DiodeIdx, float>          dac;

  /// best range selection
  CalUtil::CalVec<CalUtil::FaceIdx, CalUtil::RngNum> bestRng;

  // Hit summary histograms
  /// number of hits per layer
  CalUtil::CalArray<CalUtil::LyrNum, unsigned short> perLyr;

  // Hit lists
  /// list of xtalId's which were hit
  std::vector<CalUtil::XtalIdx>                      hitList;

  // His summary
  /// total # of hit xtals
  unsigned count;
  /// total # of layers hit
  unsigned short nLyrs;
  /// max # of hits in any layer
  unsigned short maxPerLyr;

  /// reference to pedestal calibrations needed for some operations
  const CalPed &m_peds;

  /// reference to cidac2adc calibs needed for some ops
  const CIDAC2ADC &m_cidac2adc;

  /// threshold (LEX8 ADCP + ADCN) for couniting a 'hit' xtal
  static const unsigned short hitThresh = 100;
};

#endif
