// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/TwrHodoscope.h,v 1.1 2006/06/15 20:58:00 fewtrell Exp $
/** @file
    @author fewtrell
*/

#ifndef TwrHodoscope_h
#define TwrHodoscope_h

// LOCAL INCLUDES
#include "MuonPed.h"
#include "CIDAC2ADC.h"

// GLAST INCLUDES
#include "digiRootData/DigiEvent.h"
#include "CalUtil/CalArray.h"
#include "CalUtil/CalVec.h"
#include "CalUtil/CalDefs.h"

// EXTLIB INCLUDES

// STD INCLUDES

using namespace CalUtil;

/** \brief Accumulate crystal hits in single tower and summarize into information
    which can be used for track determination.
    @author fewtrell
*/
class TwrHodoscope {
 public:
  /// default ctor
  TwrHodoscope(const MuonPed &ped,
               const CIDAC2ADC &cidac2adc,
               ostream &ostrm = cout) :      
    adc_ped(tDiodeIdx::N_VALS),
    dac(tDiodeIdx::N_VALS),
    m_peds(ped),
    m_cidac2adc(cidac2adc),
    m_ostrm(ostrm)
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
  CalVec<tDiodeIdx, float> adc_ped; 

  /// cidac values (1 per diode)
  CalVec<tDiodeIdx, float> dac;

  // Hit summary histograms
  /// number of hits per layer
  CalArray<GCRCNum, unsigned short> perLyrX; 
  /// number of hits per layer
  CalArray<GCRCNum, unsigned short> perLyrY; 
  /// # of hits per X column
  CalArray<ColNum, unsigned short> perColX; 
  /// # of hits per Y column
  CalArray<ColNum, unsigned short> perColY; 

  // Hit lists
  /// list of X direction xtalId's which were hit 
  vector<XtalIdx> hitListX; 
  /// list of Y direction xtalId's which were hit
  vector<XtalIdx> hitListY; 

  // His summary 
  /// total # of hit xtals 
  unsigned int count;          
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

  /// true if track passed selection in x direction (clean vertical 4-in row)
  bool goodXTrack;    
  /// true if track passed selection in y direction (clean vertical 4-in row)
  bool goodYTrack;    

  /// reference to pedestal calibrations needed for some operations
  const MuonPed &m_peds;

  /// reference to cidac2adc calibs needed for some ops
  const CIDAC2ADC &m_cidac2adc;

  /// stream used for loggging
  ostream &m_ostrm;
  
  /// threshold (LEX8 ADCP + ADCN) for couniting a 'hit' xtal
  static const unsigned short hitThresh = 100;

};

#endif
