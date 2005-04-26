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
  static const string _face_mnem[]= {"POS",
                                     "NEG"};

  const vector<string> FaceNum::MNEM(_face_mnem,_face_mnem + 
                                     sizeof(_face_mnem)/sizeof(string));

  static const string _diode_mnem[] = {"LARGE",
                                       "SMALL"};

  const vector<string> DiodeNum::MNEM(_diode_mnem,_diode_mnem + 
                                      sizeof(_diode_mnem)/sizeof(string));

  static const string _rng_mnem[] = {"LEX8",
                                     "LEX1",
                                     "HEX8",
                                     "HEX1"};

  const vector<string> RngNum::MNEM(_rng_mnem, _rng_mnem + 
                                    sizeof(_rng_mnem)/sizeof(string));

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
