#ifndef CalAsym_h
#define CalAsym_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/CalAsym.h,v 1.8 2007/02/27 20:44:13 fewtrell Exp $

/** @file
    @author Zachary Fewtrell
 */

// LOCAL INCLUDES
#include "../Specs/CalGeom.h"
#include "../Util/CGCUtil.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalVec.h"
#include "CalUtil/CalArray.h"

// EXTLIB INCLUDES
#include "TSpline.h"

// STD INCLUDES

/** \brief Reresents GLAST Cal crystal light asymmetry calibration constants.

   contains read & write methods to various file formats

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

  /// creates & populates Asymmetry splines from m_asym
  void genSplines();

  /// uses asym2pos splines to convert asymmetry value to xtal position for
  /// energy centroid
  /// \note uses calibGenCAL internal xtal-pitch units w/ origin at negative
  /// face.
  /// \return INVALID_ASYM on error
  /// \warning you _must_ call genSplines method before using.
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
  /// \warning you _must_ call genSplines method before using.
  float pos2asym(CalUtil::XtalIdx xtalIdx,
                 CalUtil::DiodeNum diode,
                 float pos) const {
    TSpline3 *splinePtr = m_p2aSplines[diode][xtalIdx];


    if (!splinePtr)
      return INVALID_ASYM;

    return splinePtr->Eval(pos);
  }

  /// represent invalid value in dat arrays
  static const short INVALID_ASYM = -5000;

  /// \brief return the center position of a given segment along
  /// a crystal in mm from center of xtal.
  ///
  /// \note xtals are divided along length into 12 equal segments
  /// these segments can be though of as matching the widths of the
  /// 12 orthogonal crystals.
  static float xtalSegmentPos(unsigned short col) {
    return (CalGeom::CsILength/CalUtil::ColNum::N_VALS)*
           ((float)col + 0.5)        // calc for middle of segment
           - 0.5*CalGeom::CsILength; // put 0 in middle of xtal
  }

  /// \brief return xtal segment # from longitudinal position
  /// along xtal in mm from center
  ///
  /// \note see note on xtaSegmentPos() method for definition
  /// of segment numbering
  static unsigned short pos2xtalSegment(float mmFromCtr) {
    float mmFromEnd = mmFromCtr + CalGeom::CsILength/2;


    return (unsigned short)(mmFromEnd*12/CalGeom::CsILength);
  }

  const std::vector<float> & getPtsErr(CalUtil::XtalIdx xtalIdx,
                                       CalUtil::AsymType asymType) const
  {
    return m_asymErr[asymType][xtalIdx];
  }

  const std::vector<float> & getPtsAsym(CalUtil::XtalIdx xtalIdx,
                                        CalUtil::AsymType asymType) const
  {
    return m_asym[asymType][xtalIdx];
  }

  std::vector<float> & getPtsErr(CalUtil::XtalIdx xtalIdx,
                                 CalUtil::AsymType asymType)
  {
    return m_asymErr[asymType][xtalIdx];
  }

  std::vector<float> & getPtsAsym(CalUtil::XtalIdx xtalIdx,
                                  CalUtil::AsymType asymType)
  {
    return m_asym[asymType][xtalIdx];
  }

  /// gen output filenames
  static std::string genFilename(const std::string &outputDir,
                                 const std::string &inFilename,
                                 const std::string &ext) {
    return CGCUtil::genOutputFilename(outputDir,
                                      "calAsym",
                                      inFilename,
                                      ext);
  }

private:

  /// collection of spline functions based on LEX8 vs LEX8 asymmetry
  /// for calculating hit position in muon gain calibration (1 per xtal)
  CalUtil::CalVec<CalUtil::DiodeNum, CalUtil::CalArray<CalUtil::XtalIdx,
                                                       TSpline3 *> >
  m_a2pSplines;

  /// collection of spline functions based on LEX8 vs LEX8 asymmetry
  /// vs xtal position
  CalUtil::CalVec<CalUtil::DiodeNum, CalUtil::CalArray<CalUtil::XtalIdx,
                                                       TSpline3 *> >
  m_p2aSplines;

  /// 2d vector N_ASYM_PTS lograt asymvals per xtal/asymType
  CalUtil::CalVec<CalUtil::AsymType, CalUtil::CalArray<CalUtil::XtalIdx,
                                                       std::vector<float> > >
  m_asym;
  /// corresponding error value
  CalUtil::CalVec<CalUtil::AsymType, CalUtil::CalArray<CalUtil::XtalIdx,
                                                       std::vector<float> > >
  m_asymErr;
};

#endif