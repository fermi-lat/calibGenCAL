#ifndef CalDefs_H
#define CalDefs_H 1

#include "commonRootData/idents/CalXtalId.h"
#include "idents/CalXtalId.h"

#include <string>
#include <vector>

using namespace std;

class CalDefs {
public:
  // STATICS / CONSTANTS 
  static const short N_LYRS = 8; ///< number of layers in one cal tower
  static const short N_COLS = 12; ///< number of columns in one cal layer
  static const short N_FACES = 2; ///< number of faces in one cal xtal
  static const short N_DIODES = 2; ///< number of diodes in one cal face
  static const short N_RNGS = 4; ///< number of adc ranges on one cal face
  static const short N_DIRS = 2; ///< number of 'directions' (X & Y axis)

  short rng2diode(short rng) {return rng/2;} ///< convert adc range # to diode #
  
  short lyr2Xlyr(short lyr) {return lyr/2;} ///< converts a layer #(0-7) to an X-layer # (0-3)
  short lyr2Ylyr(short lyr) {return lyr/2;} ///< converts a layer #(0-7) to a Y-layer # (0-3)
  //bool  isXlyr(short lyr) {return lyr%2==0;} ///< returns true if layer is an X-layer
  bool  isXlyr(short lyr);
  
  short diode2X8rng(short diode) {return diode*2;} ///< get the # for the X8 adc range belonging to given diode
  short diode2X1rng(short diode) {return diode*2 +1;} ///< get the # for the X1 adc range belonging to given diode

  static const vector<string> FACE_MNEM; ///< list of string names for xtal faces
  static const vector<string> RNG_MNEM; ///< statifc list of string names for adc ranges
  
  enum XtalDir {
    X_DIR = 0,
    Y_DIR
  }; ///< enumeration for the 2 xtal directions

  //////////////////////////////////
  // CAL GEOMETRY INDEX FUNCTIONS //
  //                              //
  // Allow contiguous array type  //
  // integer indexes for entire   //
  // instrument by xtal,diode,rng //
  // etc                          //
  //                              //
  // Used for all internal indexes//
  //////////////////////////////////

  // INDEX FUNCTIONS - contiguous indeces for arrays/histograms
  static const int MAX_XTAL_IDX = N_COLS*N_LYRS;   ///< Total # of xtals in current LAT
  static const int MAX_FACE_IDX = MAX_XTAL_IDX*N_FACES; ///< total # of xtal faces in current lat
  static const int MAX_DIODE_IDX = MAX_FACE_IDX*N_DIODES; ///< total # of xtal diodes in current lat
  static const int MAX_RNG_IDX = MAX_FACE_IDX*N_RNGS; ///< total # of possible adc range values in current lat
  
  /// generate LAT wide contiguous index value for a specific cal xtal
  int getNXtal(short lyr, short col) {return lyr*N_COLS + col;} 
  /// return layer # from given xtal index
  short nXtal2lyr(int nXtal) {return nXtal/N_COLS;}
  /// return column # from given xtal index
  short nXtal2col(int nXtal) {return nXtal%N_COLS;}
  /// appends unique id string for given xtal index to given string
  string &appendXtalStr(int nXtal, string &str);

  /// generate LAT wide contiguous index value for a specific cal face
  int getNFace(int nXtal, short face) {return nXtal*N_FACES + face;}
  /// generate LAT wide contiguous index value for a specific cal face
  int getNFace(short lyr, short col, short face) {return getNFace(getNXtal(lyr,col), face);}
  /// return face # from LAT wide face index
  int nFace2face(int nFace) {return nFace % N_FACES;}
  /// return LAT wide xtal index from LAT wide face index
  int nFace2nXtal(int nFace) {return nFace/N_FACES;}
  /// appends unique id string for given xtal_face index to given string
  string &appendFaceStr(int nXtal, string &str);

  /// generate LAT wide contiguous index value for a specific cal diode
  inline int getNDiode(int nFace, short diode) {return nFace*N_DIODES + diode;}
  /// generate LAT wide contiguous index value for a specific cal diode
  int getNDiode(int nXtal, short face, short diode) {return getNDiode(getNFace(nXtal,face),diode);}
  /// generate LAT wide contiguous index value for a specific cal diode
  int getNDiode(short lyr, short col, short face, short diode) {return getNDiode(getNFace(lyr,col,face),diode);}
  /// get diode # (0-1) from lat wide diode index
  int nDiode2diode(int nDiode) {return nDiode % N_DIODES;}
  /// get LAT wide xtal face index from LAT wide xtal diode index
  int nDiode2nFace(int nDiode) {return nDiode/N_DIODES;}
  /// appends unique id string for given diode index to given string
  string &appendDiodeStr(int nDiode, string &str);

  /// generate LAT wide contiguous index value for a specific adc range (including xtal face information)
  int getNRng(int nFace, short rng) {return nFace*N_RNGS + rng;}
  /// generate LAT wide contiguous index value for a specific adc range (including xtal face information)
  int getNRng(int nXtal, short face, short rng) {return getNRng(getNFace(nXtal,face),rng);}
  /// generate LAT wide contiguous index value for a specific adc range (including xtal face information)
  int getNRng(short lyr, short col, short face, short rng) {return getNRng(getNFace(lyr,col,face),rng);}
  /// generate adc range # from LAT wide adc range index
  int nRng2rng(int nRng) {return nRng % N_RNGS;}
  /// generate xtal face # from LAT wide adc range index
  int nRng2nFace(int nRng) {return nRng/N_RNGS;}
  /// appends unique id string for given adc_range index to given string
  string &appendRngStr(int nRng, string &str);

  // CalXtalId shorthand
  static const CalXtalId::XtalFace POS_FACE = CalXtalId::POS;
  static const CalXtalId::XtalFace NEG_FACE = CalXtalId::NEG;

  static const CalXtalId::AdcRange LEX8 = CalXtalId::LEX8;
  static const CalXtalId::AdcRange LEX1 = CalXtalId::LEX1;
  static const CalXtalId::AdcRange HEX8 = CalXtalId::HEX8;
  static const CalXtalId::AdcRange HEX1 = CalXtalId::HEX1;

  static const idents::CalXtalId::DiodeType LARGE_DIODE = idents::CalXtalId::LARGE;
  static const idents::CalXtalId::DiodeType SMALL_DIODE = idents::CalXtalId::SMALL;
}; // namespace caldefs

#endif // CalDefs_H
