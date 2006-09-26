#ifndef CalAsym_h
#define CalAsym_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/CalAsym.h,v 1.1 2006/09/15 15:02:10 fewtrell Exp $
/** @file
    @author Zachary Fewtrell
*/


// LOCAL INCLUDES
#include "CalGeom.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalVec.h"
#include "CalUtil/CalArray.h"

// EXTLIB INCLUDES
#include "TSpline.h"

// STD INCLUDES


/** \brief Reresents GLAST Cal crystal light asymmetry calibration constants.

contains read & write methods to various file formats & code
to calculate calibrations from digi ROOT event files

@author Zachary Fewtrell
*/
class CalAsym {
 public:
  CalAsym();

  /// write asymmetry tables out to text file.
  void writeTXT(const std::string &filename) const;
  /// read asymmetry tables in from text file(s)
  void readTXT(const std::string &filename);
  /// # of points per xtal for asymmetry type data.  
  /// 1 point for the center of each orthogonal xtal 
  /// excluding the two outermost xtals

  static const unsigned short N_ASYM_PTS = 10;

  /// creates & populates INL splines from m_asym
  void buildSplines(); 

  /// uses asym2pos splines to convert asymmetry value to xtal position for 
  /// energy centroid
  /// \note uses calibGenCAL internal xtal-pitch units w/ origin at negative 
  /// face.
  /// \return INVALID_ASYM on error
  float asym2pos(CalUtil::XtalIdx xtalIdx, 
                 CalUtil::DiodeNum diode,
                 float asym) const {
    if (!m_a2pSplines[diode][xtalIdx])
      return INVALID_ASYM;

    return m_a2pSplines[diode][xtalIdx]->Eval(asym);
  }


  /// uses pos2asym splines to convert xtal position to light asymmetry
  /// \note uses calibGenCAL internal xtal-pitch units w/ origin at negative 
  /// face.
  /// \return INVALID_ASYM on error
  float pos2asym(CalUtil::XtalIdx xtalIdx, 
                 CalUtil::DiodeNum diode,
                 float pos) const {
    TSpline3 *splinePtr = m_p2aSplines[diode][xtalIdx];
    if (!splinePtr)
      return INVALID_ASYM;

    /*     cout << splinePtr->GetName()  */
    /*          << " " << pos << " " << splinePtr->Eval(pos) */
    /*          << endl; */

    return splinePtr->Eval(pos);
  }

  static const short INVALID_ASYM = -5000;

  /// return longitudinal position (in mm) of a crystal center along the length 
  /// of an orthogonal crystal given a cystal column numnber
  static float xtalCenterPos(unsigned short col) {
    return (CalGeom::CsILength/CalUtil::ColNum::N_VALS)*
      ((float)col + 0.5)  // calc for middle of segment
      - 0.5*CalGeom::CsILength;     // put 0 in middle of xtal
  }

  const std::vector<float>& getPtsErr(CalUtil::XtalIdx xtalIdx, CalUtil::AsymType asymType) const 
    {return m_asymErr[asymType][xtalIdx];}

  const std::vector<float>& getPtsAsym(CalUtil::XtalIdx xtalIdx, CalUtil::AsymType asymType) const 
    {return m_asym[asymType][xtalIdx];}

  std::vector<float>& getPtsErr(CalUtil::XtalIdx xtalIdx, CalUtil::AsymType asymType)
    {return m_asymErr[asymType][xtalIdx];}

  std::vector<float>& getPtsAsym(CalUtil::XtalIdx xtalIdx, CalUtil::AsymType asymType)
    {return m_asym[asymType][xtalIdx];}

 private:

  /// collection of spline functions based on LEX8 vs LEX8 asymmetry 
  /// for calculating hit position in muon gain calibration (1 per xtal)
  CalUtil::CalVec<CalUtil::DiodeNum, CalUtil::CalArray<CalUtil::XtalIdx, TSpline3*> > m_a2pSplines; 
  /// collection of spline functions based on LEX8 vs LEX8 asymmetry 
  /// vs xtal position
  CalUtil::CalVec<CalUtil::DiodeNum, CalUtil::CalArray<CalUtil::XtalIdx, TSpline3*> > m_p2aSplines; 


  /// 2d vector N_ASYM_PTS lograt asymvals per xtal/asymType
  CalUtil::CalVec<CalUtil::AsymType, CalUtil::CalArray<CalUtil::XtalIdx, std::vector<float> > > m_asym; 
  /// corresponding error value
  CalUtil::CalVec<CalUtil::AsymType, CalUtil::CalArray<CalUtil::XtalIdx, std::vector<float> > > m_asymErr; 

};

#endif
