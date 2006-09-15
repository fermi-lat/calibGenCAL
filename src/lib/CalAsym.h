#ifndef CalAsym_h
#define CalAsym_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/CalAsym.h,v 1.2 2006/06/22 21:50:22 fewtrell Exp $
/** @file
    @author Zachary Fewtrell
 */


// LOCAL INCLUDES
#include "CGCUtil.h"

// GLAST INCLUDES
#include "CalUtil/CalVec.h"
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalArray.h"

// EXTLIB INCLUDES
#include "TFile.h"
#include "TH2S.h"
#include "TSpline.h"

// STD INCLUDES
#include <string>
#include <memory>

using namespace std;
using namespace CalUtil;

/** \brief Reresents GLAST Cal crystal light asymmetry calibration constants.

    contains read & write methods to various file formats & code
    to calculate calibrations from digi ROOT event files

    @author Zachary Fewtrell
*/
class CalAsym {
 public:
  CalAsym();

  /// write asymmetry tables out to text file.
  void writeTXT(const string &filename) const;
  /// read asymmetry tables in from text file(s)
  void readTXT(const string &filename);
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
  float asym2pos(XtalIdx xtalIdx, 
                 DiodeNum diode,
                 float asym) const {
    if (!m_a2pSplines[diode][xtalIdx])
      return INVALID_ASYM;

    return m_a2pSplines[diode][xtalIdx]->Eval(asym);
  }


  /// uses pos2asym splines to convert xtal position to light asymmetry
  /// \note uses calibGenCAL internal xtal-pitch units w/ origin at negative 
  /// face.
  /// \return INVALID_ASYM on error
  float pos2asym(XtalIdx xtalIdx, 
                 DiodeNum diode,
                 float pos) const {
    TSpline3 *splinePtr = m_p2aSplines[diode][xtalIdx];
    if (!splinePtr)
      return INVALID_ASYM;

/*     cout << splinePtr->GetName()  */
/*          << " " << pos << " " << splinePtr->Eval(pos) */
/*          << endl; */

    return splinePtr->Eval(pos);
  }

  static const float CSI_LEN;
  static const short INVALID_ASYM = -5000;

  /// return longitudinal position (in mm) of a crystal center along the length 
  /// of an orthogonal crystal given a cystal column numnber
  static float xtalCenterPos(unsigned short col) {
    return (CSI_LEN/ColNum::N_VALS)*
      ((float)col + 0.5)  // calc for middle of segment
      - 0.5*CSI_LEN;     // put 0 in middle of xtal
  }

  const vector<float>& getPtsErr(XtalIdx xtalIdx, AsymType asymType) const 
    {return m_asymErr[asymType][xtalIdx];}

  const vector<float>& getPtsAsym(XtalIdx xtalIdx, AsymType asymType) const 
    {return m_asym[asymType][xtalIdx];}

  vector<float>& getPtsErr(XtalIdx xtalIdx, AsymType asymType)
    {return m_asymErr[asymType][xtalIdx];}

  vector<float>& getPtsAsym(XtalIdx xtalIdx, AsymType asymType)
    {return m_asym[asymType][xtalIdx];}

 private:

  /// collection of spline functions based on LEX8 vs LEX8 asymmetry 
  /// for calculating hit position in muon gain calibration (1 per xtal)
  CalVec<DiodeNum, CalArray<XtalIdx, TSpline3*> > m_a2pSplines; 
  /// collection of spline functions based on LEX8 vs LEX8 asymmetry 
  /// vs xtal position
  CalVec<DiodeNum, CalArray<XtalIdx, TSpline3*> > m_p2aSplines; 


  /// 2d vector N_ASYM_PTS lograt asymvals per xtal/asymType
  CalVec<AsymType, CalArray<XtalIdx, vector<float> > > m_asym; 
  /// corresponding error value
  CalVec<AsymType, CalArray<XtalIdx, vector<float> > > m_asymErr; 

};

#endif
