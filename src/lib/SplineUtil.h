#ifndef SplineUtil_h
#define SplineUtil_h
// $Header $

/** @file collection of classes for polylines and associated spline functions
   @author fewtrell
 */

// LOCAL INCLUDES

// GLAST INCLUDES

// EXTLIB INCLUDES
#include "TSpline.h"

// STD INCLUDES
#include <vector>

namespace SplineUtil {
  typedef std::pair<float, float> Point2D;

  /// list of 2d points
  typedef std::vector<Point2D>    Polyline;

  /// \brief create new ROOT TSpline3 object from polyline point list
  /// \return New spline object on success, NULL on error (0 size array)
  TSpline3 * polyline2spline(const Polyline &pLine);

  /// \brief create new ROOT TSpline3 object from x & y vectors
  /// \return New spline object on success, NULL on error (0 size array)
  /// \note size of new spline will be min(x.size(), y.size())
  TSpline3 * vector2spline(const std::vector<float> &x,
                           const std::vector<float> &y,
                           const std::string &name);
};

#endif
