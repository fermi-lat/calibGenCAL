/** @file CalDefs.cxx
    \brief Glast Calorimeter array code.
    $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/CalDefs.cxx,v 1.6 2006/01/13 17:25:58 fewtrell Exp $
*/

// LOCAL INCLUDES
#include "CalDefs.h"

// GLAST INCLUDES

// EXTLIB INCLUDES

// STD INCLUDES
#include <sstream>
#include <iomanip>
#include <stdexcept>

using namespace std;

//populate static arrays.

namespace CalDefs {
  const string FaceNum::m_str[]= {"POS",
                                         "NEG"};

  const string DiodeNum::m_str[] = {"LRG",
                                           "SM"};

  const string RngNum::m_str[] = {"LEX8",
                                         "LEX1",
                                         "HEX8",
                                         "HEX1"};

  const string AsymType::m_str[] = {"LL",
                                           "LS",
                                           "SL",
                                           "SS"};

  ostream& operator<< (ostream &strm, const XtalIdx &idx) {
    strm << "T" << setfill('0') << setw(2) << idx.getTwr();
    strm << "L"                 << setw(1) << idx.getLyr();
    strm << "C" << setfill('0') << setw(2) << idx.getCol();
    return strm;
  }
    
  ostream& operator<< (ostream &strm, const FaceIdx &idx) {
    strm << idx.getXtalIdx();
    strm << "F" << setw(1) << idx.getFace();
    return strm;
  }
    
  ostream& operator<< (ostream &strm, const DiodeIdx &idx) {
    strm << idx.getFaceIdx();
    strm << "D" << setw(1) << idx.getDiode();
    return strm;
  }
    
  ostream& operator<< (ostream &strm, const RngIdx &idx) {
    strm << idx.getFaceIdx();
    strm << "R" << setw(1) << idx.getRng();
    return strm;

  }

  ostream& operator<< (ostream &strm, const tXtalIdx &idx) {
    strm << "L"                 << setw(1) << idx.getLyr();
    strm << "C" << setfill('0') << setw(2) << idx.getCol();
    return strm;
  }
    
  ostream& operator<< (ostream &strm, const tFaceIdx &idx) {
    strm << idx.getTXtalIdx();
    strm << "F" << setw(1) << idx.getFace();
    return strm;
  }
    
  ostream& operator<< (ostream &strm, const tDiodeIdx &idx) {
    strm << idx.getTFaceIdx();
    strm << "D" << setw(1) << idx.getDiode();
    return strm;
  }
    
  ostream& operator<< (ostream &strm, const tRngIdx &idx) {
    strm << idx.getTFaceIdx();
    strm << "R" << setw(1) << idx.getRng();
    return strm;
  }


}
