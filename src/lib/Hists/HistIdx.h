#ifndef IdxPath_h
#define IdxPath_h

// LOCAL INCLUDES
#include "src/lib/Util/string_util.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"


// EXTLIB INCLUDES

// STD INCLUDES
#include <string>
#include <sstream>

/** @file CalUtil::CalDefs inspired classes for indexing calibGenCAL histograms

 */

namespace calibGenCAL {

  /// convert index to filesystem type path ('/' separated, bu no '/' prefix or suffix)
  template <typename IdxType> std::string toPath(const IdxType &);

  inline std::string toPath(const CalUtil::RngNum &rng) {
    return rng.toStr();
  }

  inline std::string toPath(const CalUtil::DiodeNum &diode) {
    return diode.toStr();
  }

  inline std::string toPath(const CalUtil::FaceNum &face) {
    return face.toStr();
  }

  inline std::string toPath(const CalUtil::TwrNum &twr) {
    return twr.toStr();
  }

  inline std::string toPath(const CalUtil::LyrNum &lyr) {
    return lyr.toStr();
  }

  inline std::string toPath(const CalUtil::ColNum &col) {
    return col.toStr();
  }

  inline std::string toPath(const CalUtil::AsymType &asymType) {
    return asymType.toStr();
  }

  inline std::string toPath(const CalUtil::XtalIdx &xtalIdx) {
    return "T" + toPath(xtalIdx.getTwr()) + "/" +
      "L" + toPath(xtalIdx.getLyr()) + "/" +
      "C" + toPath(xtalIdx.getCol());
  }

  inline std::string toPath(const CalUtil::FaceIdx &faceIdx) {
    return toPath(faceIdx.getXtalIdx()) + "/" +
      toPath(faceIdx.getFace());
    
  }

  inline std::string toPath(const CalUtil::RngIdx &rngIdx) {
    return toPath(rngIdx.getFaceIdx()) + "/" +
      toPath(rngIdx.getRng());
  }


}



#endif
