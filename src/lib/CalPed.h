#ifndef CalPed_h
#define CalPed_h

// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/CalPed.h,v 1.2 2006/09/26 18:57:24 fewtrell Exp $
/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalVec.h"

// STD INCLUDES

/** \brief \brief Represents GLAST Cal ADC pedestal calibrations

contains read & write methods to various file formats 

@author Zachary Fewtrell
*/
class CalPed {
 public:
  CalPed();
 
  /// write pedestals to columnar TXTfile
  void writeTXT(const std::string &filename) const; 

  /// read pedestals from columnar TXTfile
  void readTXT(const std::string &filename);
  
  float getPed(CalUtil::RngIdx rngIdx) const {return m_peds[rngIdx];}
  float getPedSig(CalUtil::RngIdx rngIdx) const {return m_pedSig[rngIdx];}

  void setPed(CalUtil::RngIdx rngIdx, float val) {m_peds[rngIdx] = val;}
  void setPedSig(CalUtil::RngIdx rngIdx, float val) {m_pedSig[rngIdx] = val;}

  static const short INVALID_PED = -5000;
 private:
  /// output pedestal data.
  CalUtil::CalVec<CalUtil::RngIdx, float> m_peds; 
  /// corresponding err values for m_calCalPed
  CalUtil::CalVec<CalUtil::RngIdx, float> m_pedSig; 
};

#endif
