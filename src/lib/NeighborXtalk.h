#ifndef NeighborXtalk_h
#define NeighborXtalk_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/NeighborXtalk.h,v 1.1 2007/02/26 23:20:30 fewtrell Exp $

/** @file
    @author fewtrell
*/

// LOCAL INCLUDES
#include "SplineUtil.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalVec.h"

// EXTLIB INCLUDES

// STD INCLUDES

/** \brief Represents electronic crosstalk in adc units upon each Cal crystal
    by neighboring crystals.

    contains read & write methods to various file formats

    @author fewtrell
*/
class NeighborXtalk {
 public:
  NeighborXtalk() {}

  /// write calibrations to txt file
  void writeTXT(const std::string &filename) const;

  /// \brief retrieve 2d points for crosstalk spline function (x=cidac, y=adc)
  /// for given source_channel, destination_channel pair
  /// \return NULL if no such spline exists.
  /// \param dest destination Cal adc channel
  /// \param source source Cal adc channel of xtalk
  const SplineUtil::Polyline *getPts(CalUtil::RngIdx dest,
                                     CalUtil::RngIdx source) const;
    
  
  /// \brief retrieve 2d points for crosstalk spline function (x=cidac, y=adc)
  /// for given source_channel, destination_channel pair
  /// \return NULL if no such spline exists.
  /// \param dest destination Cal adc channel
  /// \param source source Cal adc channel of xtalk
  SplineUtil::Polyline *getPts(CalUtil::RngIdx dest,
                               CalUtil::RngIdx source);
 private:
  /// associate some number of cidac2adc splines w/
  typedef std::map<CalUtil::RngIdx, SplineUtil::Polyline> ChannelSplineMap;

  /// \brief ADC Spline points array stored in 3 dims [destChan][sourceChan][pointIdx]
  CalUtil::CalVec<CalUtil::RngIdx, 
    ChannelSplineMap >  m_splinePts;
};

#endif
