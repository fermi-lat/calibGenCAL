#ifndef IdxPath_h
#define IdxPath_h

// LOCAL INCLUDES
#include "../Util/string_util.h"

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

  /// composite index (XtalIdx,  diode size, inferred Z)
  class MeanDacZId : public CalUtil::LATWideIndex {
  public:
    MeanDacZId(const unsigned short inferredZ,
               const CalUtil::XtalIdx xtalIdx,
               const CalUtil::DiodeNum diode
               ) :
      LATWideIndex(calc(inferredZ, xtalIdx, diode))
    {}

    /// construct from raw integer format (same as from val() method)
    explicit MeanDacZId(const unsigned raw) :
      LATWideIndex(raw)
    {}

    /// construct from string repr
    explicit MeanDacZId(const std::string &name);

    std::string toStr() const {
      std::ostringstream tmp;
      tmp << getXtalIdx().toStr() + "_" + getDiode().toStr() + "_Z" << getInferredZ();
      return tmp.str();
    }
    
    unsigned short getInferredZ() const {return m_data/Z_BASE;}

    CalUtil::XtalIdx getXtalIdx() const {return CalUtil::XtalIdx((m_data%Z_BASE)/XTAL_BASE);}

    CalUtil::DiodeNum getDiode() const {return CalUtil::DiodeNum(m_data%XTAL_BASE);}

  private:
    static unsigned calc(const unsigned short inferredZ,
                         const CalUtil::XtalIdx xtalIdx,
                         const CalUtil::DiodeNum diode
                         ) {
      return inferredZ*Z_BASE + xtalIdx.val()*XTAL_BASE + diode.val();
    }

    static const unsigned XTAL_BASE = CalUtil::DiodeNum::N_VALS;
      
    static const unsigned Z_BASE = XTAL_BASE*CalUtil::XtalIdx::N_VALS;

    static const unsigned short N_FIELDS = 3;

  };

  inline std::string toPath(const MeanDacZId &id) {
    std::ostringstream tmp;
    tmp << toPath(id.getXtalIdx())      << "/" 
        << toPath(id.getDiode())        << "/" 
        << "Z" << toString(id.getInferredZ());
    return tmp.str();
  }


  /// composite index (xtalIdx, diode size)
  class MeanDACId : public CalUtil::LATWideIndex {
  public:
    MeanDACId(const CalUtil::XtalIdx xtalIdx,
              const CalUtil::DiodeNum diode) :
      LATWideIndex(calc(xtalIdx, diode))

    {}

    /// construct from raw integer format (same as from val() method)
    explicit MeanDACId(const unsigned raw) :
      LATWideIndex(raw)
    {}

    /// construct from string repr
    explicit MeanDACId(const std::string &name);

    CalUtil::XtalIdx getXtalIdx() const { return CalUtil::XtalIdx(m_data/XTAL_BASE); }

    CalUtil::DiodeNum getDiode() const { return CalUtil::DiodeNum(m_data%XTAL_BASE); }

    std::string toStr() const {
      std::ostringstream tmp;
      tmp << getXtalIdx().toStr() + "_" + getDiode().toStr();
      return tmp.str();
    }

  private:
    static unsigned calc(const CalUtil::XtalIdx xtalIdx,
                         const CalUtil::DiodeNum diode) {
      return xtalIdx.val()*XTAL_BASE + diode.val();
    }

    static const unsigned short XTAL_BASE = CalUtil::DiodeNum::N_VALS;
    static const unsigned short N_FIELDS = 2;

  };


  inline std::string toPath(const MeanDACId &id) {
    return toPath(id.getXtalIdx()) + "/" + "D" + toPath(id.getDiode());
  }
  
  /// composite index (diode size, inferred Z)
  class ZDiodeId : public CalUtil::SimpleId {
  public:
    ZDiodeId(const unsigned short inferredZ,
             const CalUtil::DiodeNum diode
             ) :
      SimpleId(calc(inferredZ, diode)) {}

    /// construct from raw integer format (same as from val() method)
    explicit ZDiodeId(const unsigned raw) :
      SimpleId(raw)
    {}

    /// construct from string repr
    explicit ZDiodeId(const std::string &name);

    unsigned short getInferredZ() const {return m_data/Z_BASE;}
      
    CalUtil::DiodeNum getDiode() const {return CalUtil::DiodeNum(m_data%Z_BASE);}

    bool operator==(const ZDiodeId that) const {return m_data == that.m_data;}
    bool operator!=(const ZDiodeId that) const {return m_data != that.m_data;}
    bool operator<(const ZDiodeId that) const {return m_data < that.m_data;}

    std::string toStr() const {
      std::ostringstream tmp;
      tmp << getDiode().toStr() + "_Z" << getInferredZ();
      return tmp.str();
    }
    
  private:
    static unsigned calc(const unsigned short inferredZ,
                         const CalUtil::DiodeNum diode
                         ){
      return inferredZ*Z_BASE + diode.val();
    }

    static const unsigned short Z_BASE = CalUtil::DiodeNum::N_VALS;
    static const unsigned short N_FIELDS = 2;
  };

  inline std::string toPath(const ZDiodeId &id) {
    return toPath(id.getDiode()) + "/" + "Z" + toString(id.getInferredZ());
  }

}



#endif
