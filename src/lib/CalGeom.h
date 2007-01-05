#ifndef CalGeom_H
#define CalGeom_H

//$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/CalGeom.h,v 1.4 2007/01/04 23:23:00 fewtrell Exp $

// LOCAL INCLUDES=

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"

// EXTLIB INCLUDES
#include "CLHEP/Geometry/Vector3D.h"

// STD INCLUDES

/** @file CalGeom.h
    @author Zachary Fewtrell
    \brief Static Cal Geometry data for use when
    Gleam facilities are not available.
*/

namespace CalGeom {
  /// z pos of top surface of Layer 0 Cal Xtal
  static const float CalTopZ       = -48.12;
  static const float CsILength     = 326.  ;
  static const float CsIHeight     = 19.9 ;
  static const float CsIWidth      = 26.7;
  static const float cellHorPitch  = 27.84;
  static const float cellVertPitch = 21.35;
  static const float twrPitch      = 374.5 ;
  static const float twrGap        = twrPitch - 12.*cellHorPitch;
  /// z pos of z-center of Layer 0 Cal Xtal
  static const float lyr0ZCtr      = -58.07;

  typedef Vector3D<float> Vec3D;

  /// return global LAT Z coordinate for z-center of given cal layer
  inline float lyrCtrZ(CalUtil::LyrNum lyr) {
    return lyr0ZCtr - cellVertPitch*lyr.val();
  }

  /// invalid Xtal Idx
  static const CalUtil::XtalIdx INVALID_XTAL(0xFFFF);

  /// convert 3D vector into Cal xtal
  /// \return INVALID_XTAL if point is not w/ in crystal
  /// boundaries.
  ///
  /// \note returns INVALID_XTAL if point is in inter-xtal gaps
  CalUtil::XtalIdx pos2Xtal(const Vec3D &pos);

  /// return
  Vec3D            xtalCtrPos(CalUtil::XtalIdx xtalIdx);
};

#endif // CalGeom_H
