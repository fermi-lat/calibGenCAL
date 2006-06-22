#ifndef MuonAsym_h
#define MuonAsym_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/MuonAsym.h,v 1.1 2006/06/15 20:58:00 fewtrell Exp $
/** @file
    @author Zachary Fewtrell
 */


// LOCAL INCLUDES
#include "MuonPed.h"
#include "CIDAC2ADC.h"
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
class MuonAsym {
 public:
  MuonAsym(ostream &ostrm = cout);

  /// populate asymmetry profiles w/ nEvt worth of data.
  void fillHists(unsigned nEntries,
                 const vector<string> &rootFileList, 
                 const MuonPed &peds,
                 const CIDAC2ADC &dac2adc); 
  /// load mean values from asymmetry profiles into m_asym*** arrays
  void fitHists(); 
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

  static void genOutputFilename(const string outputDir,
                                const string &inFilename,
                                const string &ext,
                                string &outFilename) {
    CGCUtil::genOutputFilename(outputDir,
                               "muonAsym",
                               inFilename,
                               ext,
                               outFilename);
  }


  /// load histograms from ROOT output of previous run
  void loadHists(const string &filename);

  /// print histogram summary info to output stream
  void summarizeHists(ostream &ostrm=cout);


 private:

  /// allocate & create asymmetry histograms & pointer arrays
  void initHists();

  /// count min number of entries in all enable histograms
  unsigned getMinEntries();

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

  /// list of histograms for muon asymmetry
  CalVec<AsymType, CalArray<XtalIdx, TH2S*> > m_histograms; 

  string genHistName(AsymType asymType, XtalIdx xtalIdx);

  //-- Asymmetry 2 Pos conversion --//
  static const short INVALID_ASYM = -5000;
  static const float CSI_LEN;
  
  /// return longitudinal position (in mm) of a crystal center along the length of an 
  /// orthogonal crystal given a cystal column numnber
  static float xtalCenterPos(short col) {
    return (CSI_LEN/ColNum::N_VALS)*
      ((float)col + 0.5)  // calc for middle of segment
      - 0.5*CSI_LEN;     // put 0 in middle of xtal
  }

  
  ostream &m_ostrm;


};

#endif
