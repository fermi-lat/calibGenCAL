#include <sstream>
#include <iomanip>

#include "CalDefs.h"

using namespace std;

//populate static arrays.

static const string _face_mnem[]= {"POS",
"NEG"};

const vector<string> CalDefs::FACE_MNEM(_face_mnem,_face_mnem+sizeof(_face_mnem)/sizeof(string));

static const string _rng_mnem[] = {"LEX8",
"LEX1",
"HEX8",
"HEX1"};

const vector<string> CalDefs::RNG_MNEM(_rng_mnem, _rng_mnem+sizeof(_rng_mnem)/sizeof(string));

string &CalDefs::appendXtalStr(int nXtal, string &str) {
   ostringstream tmpStrm;
   tmpStrm << "L" << setfill('0') << setw(2) << nXtal2lyr(nXtal)
      << "C" << setw(1) << nXtal2col(nXtal);
   return str += tmpStrm.str();
   }

string &CalDefs::appendFaceStr(int nFace, string &str) {
   appendXtalStr(
      nFace2nXtal(nFace),str);

   ostringstream tmpStrm;
   tmpStrm << "F" << setw(1) << nFace2face(nFace);

   return str += tmpStrm.str();
   }

string &CalDefs::appendDiodeStr(int nDiode, string &str) {
   appendFaceStr(
      nDiode2nFace(nDiode),str);

   ostringstream tmpStrm;
   tmpStrm << "D" << setw(1) << nDiode2diode(nDiode);

   return str += tmpStrm.str();
   }

string &CalDefs::appendRngStr(int nRng, string &str) {
   appendFaceStr(
      nRng2nFace(nRng),str);

   ostringstream tmpStrm;
   tmpStrm << "D" << setw(1) << nRng2rng(nRng);

   return str += tmpStrm.str();
   }

bool  CalDefs::isXlyr(short lyr) {return lyr%2==0;} ///< returns true if layer is an X-layer

