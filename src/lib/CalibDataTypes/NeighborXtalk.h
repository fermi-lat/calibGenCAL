#ifndef NeighborXtalk_h
#define NeighborXtalk_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/CalibDataTypes/NeighborXtalk.h,v 1.3 2007/04/10 14:51:01 fewtrell Exp $

/** @file
    @author fewtrell
*/

// LOCAL INCLUDES
#include "../Util/SplineUtil.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalVec.h"

// EXTLIB INCLUDES

// STD INCLUDES

namespace calibGenCAL {

  /** \brief Represents electronic crosstalk in adc units upon each Cal crystal
      by neighboring crystals.

      contains read & write methods to various file formats

      @author fewtrell
  */
  class NeighborXtalk {
  public:
    NeighborXtalk() {
    }

    /// write calibrations to txt file
    void writeTXT(const std::string &filename) const;

    /// write ROOT file w/ one tuple per xtalk spline, also throw in a TGraph per spline
    void writeTuples(const std::string &filename) const;

    /// \brief retrieve 2d points for crosstalk spline function (x=cidac, y=adc)
    /// for given source_channel, destination_channel pair
    /// \return NULL if no such spline exists.
    /// \param dest destination Cal adc channel
    /// \param source source Cal adc channel of xtalk
    const SplineUtil::Polyline *getPts(const CalUtil::DiodeIdx dest,
                                       const CalUtil::DiodeIdx source) const;

    /// \brief retrieve 2d points for crosstalk spline function (x=cidac, y=adc)
    /// for given source_channel, destination_channel pair
    /// \return NULL if no such spline exists.
    /// \param dest destination Cal adc channel
    /// \param source source Cal adc channel of xtalk
    SplineUtil::Polyline * getPts(const CalUtil::DiodeIdx dest,
                                  const CalUtil::DiodeIdx source);

    /// \brief set single spline point for given src / dest adc channel
    void                   setPoint(const CalUtil::DiodeIdx dest,
                                    const CalUtil::DiodeIdx source,
                                    const float dac,
                                    const float adc);

    /// for each xtalk spline, subtract spline[0].adc from all points in spline.
    void pedSubtractADC();

  private:
    /// associate cidac2adc splines from other channels w/ a single adc channel
    typedef std::map<CalUtil::DiodeIdx, SplineUtil::Polyline> ChannelSplineMap;

    /// associate a single adc channel w/ a  ChannelSplineMap
    typedef std::map<CalUtil::DiodeIdx, ChannelSplineMap>     XtalkMap;

    /// \brief ADC Spline points array stored in 3 dims [destChan][sourceChan][pointIdx]
    XtalkMap m_xtalkMap;
  };

}; // namespace calibGenCAL
#endif
