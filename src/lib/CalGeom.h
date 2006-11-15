#ifndef CalGeom_H
#define CalGeom_H

//$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/CalGeom.h,v 1.2 2006/09/26 20:27:01 fewtrell Exp $

// LOCAL INCLUDES

// GLAST INCLUDES

// EXTLIB INCLUDES

// STD INCLUDES

/** @file CalGeom.h
    @author Zachary Fewtrell
    \brief Static Cal Geometry data for use when 
    Gleam facilities are not available.
*/

namespace CalGeom {
  static const float CalTopZ       = -48.12;  
  static const float CsILength     = 326.  ;
  static const float CsIHeight     = 19.9 ;
  static const float CsIWidth      = 26.7;
  static const float cellHorPitch  = 27.84;
  static const float cellVertPitch = 21.35;  
  static const float twrPitch      = 374.5 ;
  static const float twrGap        = twrPitch - 12.*cellHorPitch;

  /// vertical pitch (mm) of cal xtals
  static const float CELL_VERT_PITCH = 21.35;
  /// horizontal pitch (mm) of cal xtals
  static const float CELL_HOR_PITCH  = 27.84;

};

#endif // CalGeom_H
