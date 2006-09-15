#ifndef CalPed_h
#define CalPed_h

// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/CalPed.h,v 1.2 2006/06/22 21:50:23 fewtrell Exp $
/** @file
    @author Zachary Fewtrell
 */

// LOCAL INCLUDES
#include "CGCUtil.h"

// GLAST INCLUDES
#include "CalUtil/CalVec.h"
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalArray.h"


// STD INCLUDES

using namespace std;
using namespace CalUtil;

/** \brief \brief Represents GLAST Cal ADC pedestal calibrations

    contains read & write methods to various file formats & code
    to calculate calibrations from digi ROOT event files

    @author Zachary Fewtrell
*/
class CalPed {
 public:
  CalPed();
 
  /// write muon LEX8 pedestals to simple columnar .txt file
  void writeTXT(const string &filename) const; 

  void readTXT(const string &filename);
  
  float getPed(RngIdx rngIdx) const {return m_peds[rngIdx];}
  float getPedSig(RngIdx rngIdx) const {return m_pedSig[rngIdx];}

  void setPed(RngIdx rngIdx, float val) {m_peds[rngIdx] = val;}
  void setPedSig(RngIdx rngIdx, float val) {m_pedSig[rngIdx] = val;}

  static const short INVALID_PED = -5000;
 private:
  /// output pedestal data.
  CalVec<RngIdx, float> m_peds; 
  /// corresponding err values for m_calCalPed
  CalVec<RngIdx, float> m_pedSig; 
};

#endif
