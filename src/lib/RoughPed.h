#ifndef RoughPed_h
#define RoughPed_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/RoughPed.h,v 1.2 2006/06/22 21:50:23 fewtrell Exp $
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

// STD INCLUDES

using namespace std;
using namespace CalUtil;

/** \brief Represents GLAST Cal LEX8 pedestals calibrations.  'rough' means
    that non-pedestal hits have not yet been cut.

    contains read & write methods to various file formats & code
    to calculate calibrations from digi ROOT event files
    @author Zachary Fewtrell
*/
class RoughPed {
 public:
  RoughPed();
 
  /// write rough LEX8 pedestals to simple columnar .txt file
  void writeTXT(const string &filename) const; 

  /// read in rough peds from txt file
  void readTXT(const string &filename);

  /// retrieve pedestal for specific channel
  float getPed(FaceIdx faceIdx) const {return m_peds[faceIdx];}
  /// retrieve pedestal sigma for specific channel
  float getPedSig(FaceIdx faceIdx) const {return m_pedSig[faceIdx];}

  /// retrieve pedestal for specific channel
  void setPed(FaceIdx faceIdx, float val) {m_peds[faceIdx] = val;}
  /// retrieve pedestal sigma for specific channel
  void setPedSig(FaceIdx faceIdx, float val) {m_pedSig[faceIdx] = val;}

  /// indicate unpopulated field
  static const short INVALID_PED = -5000;

 private:
  /// 'rough' first pass cal pedestals, all signals used (i.e. 'hits' and 'misses'), indexed by FaceIdx()
  CalVec<FaceIdx, float> m_peds; 
  /// corresponding err values for m_calRoughPed
  CalVec<FaceIdx, float> m_pedSig; 

};

#endif
