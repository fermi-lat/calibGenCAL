#ifndef CalDefs_H
#define CalDefs_H

/** @file CalDefs.h
    
    @author Zachary Fewtrell

    \brief Glast Calorimeter array classes
*/

// LOCAL

// GLAST
#include "idents/CalXtalId.h"

// EXTLIB

// STD
#include <string>
#include <vector>
#include <set>
#include <ostream>
#include <stdexcept>

using namespace std;

namespace CalDefs {
  // CalXtalId shorthand
  const unsigned short POS_FACE = (unsigned short)idents::CalXtalId::POS;
  const unsigned short NEG_FACE = (unsigned short)idents::CalXtalId::NEG;

  const unsigned short LEX8 = (unsigned short)idents::CalXtalId::LEX8;
  const unsigned short LEX1 = (unsigned short)idents::CalXtalId::LEX1;
  const unsigned short HEX8 = (unsigned short)idents::CalXtalId::HEX8;
  const unsigned short HEX1 = (unsigned short)idents::CalXtalId::HEX1;

  const unsigned short LRG_DIODE = (unsigned short)idents::CalXtalId::LARGE;
  const unsigned short SM_DIODE = (unsigned short)idents::CalXtalId::SMALL;

  const unsigned short X_DIR = 0;
  const unsigned short Y_DIR = 1;

  /// names for volume identifier fields
  enum {fLATObjects, fTowerY, fTowerX, fTowerObjects, fLayer,
        fMeasure, fCALXtal,fCellCmp, fSegment};
 

  ///////////////////////////////////////////
  //// Atomic (simple) Cal Geometry Id's  ///
  //// Id single Cal component w/ in it's ///
  //// immediate container                ///
  ///////////////////////////////////////////

  class SimpleId {
  public:
    operator unsigned() const {return m_data;}

    /// prefix ++ operator
    SimpleId operator++() {
      SimpleId tmp(*this);
      m_data++;
      return tmp;
    }

    /// postfix ++ operator
    SimpleId& operator++(int) {
      m_data++; 
      return *this;
    }

    unsigned short val() const {return m_data;}
  protected:
    SimpleId(unsigned short val) : m_data(val) {}
    SimpleId() : m_data(0) {}
    unsigned short m_data;
  };

  class TwrNum : public SimpleId {
  public:
    TwrNum() : 
      SimpleId() {}

    TwrNum(unsigned short val) : 
      SimpleId(val) {}
    
    unsigned short getRow() const {return m_data/N_COLS;}
    unsigned short getCol() const {return m_data%N_COLS;}
    
    static const unsigned short N_VALS=16;
    bool isValid() const {return m_data < N_VALS;}
    
    static const unsigned short N_COLS = 4;
    static const unsigned short N_ROWS = 4;
  };

  class DirNum : public SimpleId {
  public:
    DirNum() : SimpleId() {}
    DirNum(unsigned short val) : SimpleId(val) {}
    static const unsigned short N_VALS=2;
    bool isValid() const {return m_data < N_VALS;}
  };

  class LyrNum : public SimpleId {
  public:
    LyrNum() : SimpleId() {}
    LyrNum(unsigned short val) : SimpleId(val) {}
    static const unsigned short N_VALS=8;
    bool isValid() const {return m_data < N_VALS;}
    DirNum getDir() const {return (m_data%2 == 0) ? X_DIR : Y_DIR;} // 0 is 'X' Direction
    unsigned short getXLyr() const {return m_data/2;}
    unsigned short getYLyr() const {return (m_data-1)/2;}
  };


  class ColNum : public SimpleId {
  public:
    ColNum() : SimpleId() {}
    ColNum(unsigned short val) : SimpleId(val) {}
    static const unsigned short N_VALS=12;
    bool isValid() const {return m_data < N_VALS;}
  };

  class FaceNum : public SimpleId {
  public:
    FaceNum() : SimpleId() {}
    FaceNum(unsigned short val) : SimpleId(val) {}

    /// return string representation
    const string &getMnem() const {return m_str[m_data];}

    static const unsigned short N_VALS=2;    
    bool isValid() const {return m_data < N_VALS;}
  private:
    static const string m_str[N_VALS];
  };

  class DiodeNum : public SimpleId {
  public:
    DiodeNum(unsigned short val) : SimpleId(val) {}
    DiodeNum(void) : SimpleId() {}
    
    unsigned short getX8Rng() const {return m_data*2;}
    unsigned short getX1Rng() const {return getX8Rng()+1;}

    /// return string representation
    const string &getMnem() const {return m_str[m_data];}

    static const unsigned short N_VALS=2; 
    bool isValid() const {return m_data < N_VALS;}

  private:
    static const string m_str[N_VALS];
  };

  class RngNum : public SimpleId {
  public:
    RngNum(unsigned short val) : SimpleId(val) {}
    RngNum(void) : SimpleId() {}
    
    /// return string representation
    const string &getMnem() const {return m_str[m_data];}
    static const unsigned short N_VALS=4;
    
    DiodeNum getDiode() const {
      return idents::CalXtalId::rangeToDiode((idents::CalXtalId::AdcRange)m_data);
    }
    bool isValid() const {return m_data < N_VALS;}

  private:
    static const string m_str[N_VALS];

  };

  /////////////////////////////////////////////
  /// Xtal Wide Indexes                     ///
  /// Id all components of same type within ///
  /// a single xtal.                        ///
  /// internal integer storage is contiguous///
  /// and can be used as an array Index     ///
  /////////////////////////////////////////////

  class XtalWideIndex {
  public:
    /// prefix ++ operator
    XtalWideIndex operator++() {
      XtalWideIndex tmp(*this);
      m_data++;
      return tmp;
    }

    /// postfix ++ operator
    XtalWideIndex& operator++(int) {
      m_data++; 
      return *this;
    }

    //operator unsigned() const {return m_data;}

    bool operator==(const XtalWideIndex &that) const {return m_data == that.m_data;}
    bool operator<=(const XtalWideIndex &that) const {return m_data <= that.m_data;}
    bool operator!=(const XtalWideIndex &that) const {return m_data != that.m_data;}
    bool operator< (const XtalWideIndex &that) const {return m_data <  that.m_data;}
    //XtalWideIndex& operator= (const XtalWideIndex &that) {m_data = that.m_data;}

    unsigned val() const {return m_data;}

  protected:
    XtalWideIndex(unsigned short val) : m_data(val) {}
    XtalWideIndex() : m_data(0) {}
    unsigned short m_data;
  };

  class XtalDiode : public XtalWideIndex {
  public: 
    XtalDiode(const FaceNum face, const DiodeNum diode) :
      XtalWideIndex(face.val()*FACE_BASE + diode) {}

    XtalDiode() : XtalWideIndex() {}

    DiodeNum getDiode() const {return m_data%FACE_BASE;}
    FaceNum getFace()  const {return m_data/FACE_BASE;}

    static const unsigned short N_VALS = FaceNum::N_VALS*DiodeNum::N_VALS;
    bool isValid() const {return m_data < N_VALS;}
  protected:
    static const unsigned short FACE_BASE = DiodeNum::N_VALS;
  };

  class XtalRng : public XtalWideIndex {
  public:
    XtalRng(FaceNum face, RngNum rng) :
      XtalWideIndex(face.val()*FACE_BASE + rng) {};

    XtalRng() : XtalWideIndex() {}

    FaceNum getFace() const {return m_data/FACE_BASE;}
    RngNum getRng() const {return m_data%FACE_BASE;}

    XtalDiode getXtalDiode() const {
      return XtalDiode(getFace(), ((RngNum)getRng()).getDiode());
    }

    static const unsigned short N_VALS = FaceNum::N_VALS*RngNum::N_VALS;
    bool isValid() const {return m_data < N_VALS;}
  protected:
    static const unsigned short FACE_BASE = RngNum::N_VALS;
  };

  /////////////////////////////////////////////
  /// Tower Wide Cal Indexes                ///
  /// Id all components of same type within ///
  /// single tower.
  /// internal integer storage is contiguous///
  /// and can be used as an array Index     ///
  /////////////////////////////////////////////

  /** \brief Abstract root class for indexing Cal 
      components w/in single tower

      any interfaces w/ CalXtalId always work w/ tower bay 
      # 0.
  */
  class TwrWideIndex {
  public:  
    /// prefix ++ operator
    TwrWideIndex operator++() {
      TwrWideIndex tmp(*this);
      m_data++;
      return tmp;
    }

    /// postfix ++ operator
    TwrWideIndex& operator++(int) {
      m_data++; 
      return *this;
    }
    
    unsigned val() const {return m_data;}
    
    bool operator==(const TwrWideIndex &that) const {return m_data == that.m_data;}
    bool operator<=(const TwrWideIndex &that) const {return m_data <= that.m_data;}
    bool operator!=(const TwrWideIndex &that) const {return m_data != that.m_data;}
    bool operator< (const TwrWideIndex &that) const {return m_data <  that.m_data;}
    //TwrWideIndex& operator= (const TwrWideIndex &that) {m_data = that.m_data;}
  protected:
    TwrWideIndex(unsigned val) : m_data(val) {}
    TwrWideIndex() : m_data(0) {}
    unsigned m_data;
  };

  class tXtalIdx : public TwrWideIndex {
  public:
    tXtalIdx(const idents::CalXtalId &xtal) :
      TwrWideIndex(calc(xtal.getLayer(),
                        xtal.getColumn())) {}

    tXtalIdx(LyrNum lyr, ColNum col) :
      TwrWideIndex(calc(lyr,col)) {}

    tXtalIdx() : TwrWideIndex() {}

    idents::CalXtalId getCalXtalId() const {
      return idents::CalXtalId(0,//twr
                               getLyr(),
                               getCol());
    }
    static const unsigned N_VALS  = LyrNum::N_VALS*ColNum::N_VALS;

    LyrNum getLyr() const {return (m_data)/LYR_BASE;}
    ColNum getCol() const {return m_data%LYR_BASE;}

    /// operator to put tXtalIdx to output stream
    friend ostream& operator<< (ostream &stream, const tXtalIdx &idx);
    bool isValid() const {return m_data < N_VALS;}
    
  private:
    static unsigned calc(LyrNum lyr, ColNum col) {
      return lyr*LYR_BASE + col;
    }
    static const unsigned LYR_BASE  = ColNum::N_VALS;
  };
  
  class tFaceIdx : public TwrWideIndex {
  public:
    tFaceIdx() : TwrWideIndex() {}

    tFaceIdx(const idents::CalXtalId &xtalId) {
      if (!xtalId.validFace())
        throw invalid_argument("tFaceIdx requires valid face info in xtalId."
                               "  Programmer error");

      m_data = calc(xtalId.getLayer(),
                    xtalId.getColumn(),
                    xtalId.getFace());
    }

    tFaceIdx(LyrNum lyr, ColNum col, FaceNum face) :
      TwrWideIndex(calc(lyr,col,face)) {}

    tFaceIdx(tXtalIdx xtal, FaceNum face) :
      TwrWideIndex(calc(xtal.getLyr(), xtal.getCol(), face)) {}

    idents::CalXtalId getCalXtalId() const {
      return idents::CalXtalId(0,//twr
                               getLyr(),
                               getCol(),
                               getFace().val());
    }

    static const unsigned N_VALS = tXtalIdx::N_VALS*FaceNum::N_VALS;

    tXtalIdx getTXtalIdx() const {return tXtalIdx(getLyr(),
                                                  getCol());}

    LyrNum getLyr()  const {return (m_data)/LYR_BASE;}
    ColNum getCol()  const {return (m_data%LYR_BASE)/COL_BASE;}
    FaceNum getFace() const {return m_data%COL_BASE;}

    void setFace(FaceNum face) {
      m_data = calc(getLyr(),getCol(),face);
    }

    /// operator to put tFaceIdx to output stream
    friend ostream& operator<< (ostream &stream, const tFaceIdx &idx);
    
    bool isValid() const {return m_data < N_VALS;}
  private:
    static unsigned calc(LyrNum lyr, ColNum col, FaceNum face) {
      return lyr.val()*LYR_BASE + col.val()*COL_BASE + face.val();
    }
    static const unsigned short COL_BASE  = FaceNum::N_VALS;
    static const unsigned short LYR_BASE  = COL_BASE*ColNum::N_VALS;
  };

  class tDiodeIdx : public TwrWideIndex {
  public:
    tDiodeIdx(const idents::CalXtalId &xtalId, DiodeNum diode) {
      if (!xtalId.validFace())
        throw invalid_argument("tDiodeIdx requires valid face info in xtalId.  Programmer error");

      m_data = calc(xtalId.getLayer(),
                    xtalId.getColumn(),
                    xtalId.getFace(), 
                    diode);
    }

    tDiodeIdx(LyrNum lyr, ColNum col, FaceNum face, DiodeNum diode) :
      TwrWideIndex(calc(lyr,col,face,diode)) {}

    tDiodeIdx(tXtalIdx xtal, FaceNum face, DiodeNum diode) :
      TwrWideIndex(calc(xtal.getLyr(),xtal.getCol(),face,diode)) {}

    tDiodeIdx(tXtalIdx xtal, XtalDiode xDiode) :
      TwrWideIndex(calc(xtal.getLyr(), xtal.getCol(),
                        xDiode.getFace(), xDiode.getDiode())) {}

    tDiodeIdx(tFaceIdx face, DiodeNum diode) :
      TwrWideIndex(calc(face.getLyr(),face.getCol(),face.getFace(),diode)) {}
    
    tDiodeIdx() : TwrWideIndex() {}

    static const unsigned N_VALS = tFaceIdx::N_VALS*DiodeNum::N_VALS;

    LyrNum getLyr()   const {return (m_data)/LYR_BASE;}
    ColNum getCol()   const {return (m_data%LYR_BASE)/COL_BASE;}
    FaceNum getFace()  const {return (m_data%COL_BASE)/FACE_BASE;}
    DiodeNum getDiode() const {return m_data%FACE_BASE;}

    /// operator to put tDiodeIdx to output stream
    friend ostream& operator<< (ostream &stream, const tDiodeIdx &idx);

    tXtalIdx getTXtalIdx() const {return tXtalIdx(getLyr(),
                                                  getCol());}

    tFaceIdx getTFaceIdx() const {return tFaceIdx(getLyr(),
                                                  getCol(),
                                                  getFace());}

    
    bool isValid() const {return m_data < N_VALS;}
  private:
    static unsigned calc(LyrNum lyr, ColNum col, FaceNum face, DiodeNum diode) {
      return lyr.val()*LYR_BASE + col.val()*COL_BASE + face.val()*FACE_BASE + diode.val();
    }
    static const unsigned FACE_BASE = DiodeNum::N_VALS;
    static const unsigned COL_BASE  = FACE_BASE*FaceNum::N_VALS;
    static const unsigned LYR_BASE  = COL_BASE*ColNum::N_VALS;
  };

  class tRngIdx : public TwrWideIndex {
  public:
    tRngIdx(const idents::CalXtalId &xtalId) {
      if (!xtalId.validFace() || ! xtalId.validRange())
        throw invalid_argument("tRngIdx requires valid face and range info in xtalId.  Programmer error");
      m_data = calc(xtalId.getLayer(),xtalId.getColumn(),xtalId.getFace(), xtalId.getRange());
    }

    tRngIdx(LyrNum lyr, ColNum col, FaceNum face, RngNum rng) :
      TwrWideIndex(calc(lyr,col,face,rng)) 
      {}
    
    tRngIdx(tXtalIdx xtal, FaceNum face, RngNum rng) :
      TwrWideIndex(calc(xtal.getLyr(),xtal.getCol(),face,rng)) {}

    tRngIdx(tXtalIdx xtal, XtalRng xRng) :
      TwrWideIndex(calc(xtal.getLyr(), xtal.getCol(),
                        xRng.getFace(),xRng.getRng())) {}

    tRngIdx(tFaceIdx faceIdx, RngNum rng) :
      TwrWideIndex(calc(faceIdx.getLyr(),faceIdx.getCol(),faceIdx.getFace(),rng)) {}
    
    tRngIdx() : TwrWideIndex() {}

    idents::CalXtalId getCalXtalId() const {
      return idents::CalXtalId(getLyr(),
                               getCol(),
                               getFace(),
                               getRng().val());
    }
    
    LyrNum getLyr()  const {return (m_data)/LYR_BASE;}
    ColNum getCol()  const {return (m_data%LYR_BASE)/COL_BASE;}
    FaceNum getFace() const {return (m_data%COL_BASE)/FACE_BASE;}
    RngNum getRng()  const {return m_data%FACE_BASE;}

    void setFace(FaceNum face) {
      m_data = calc(getLyr(),getCol(),face,getRng());
    }
    void setRng(RngNum rng) {
      m_data = calc(getLyr(),getCol(),getFace(),rng);
    }
    
    static const unsigned N_VALS = tFaceIdx::N_VALS*RngNum::N_VALS;

    /// operator to put tDiodeIdx to output stream
    friend ostream& operator<< (ostream &stream, const tRngIdx &idx);
    
    tXtalIdx getTXtalIdx() const {return tXtalIdx(getLyr(),
                                                  getCol());}

    tFaceIdx getTFaceIdx() const {return tFaceIdx(getLyr(),
                                                  getCol(),
                                                  getFace());}

    bool isValid() const {return m_data < N_VALS;}
  private:
    static unsigned calc(LyrNum lyr, ColNum col, FaceNum face, RngNum rng) {
      return lyr*LYR_BASE + col*COL_BASE + face*FACE_BASE + rng;
    }
    static const unsigned FACE_BASE = RngNum::N_VALS;
    static const unsigned COL_BASE  = FACE_BASE*FaceNum::N_VALS;
    static const unsigned LYR_BASE  = COL_BASE*ColNum::N_VALS;
  };


  /////////////////////////////////////////////
  ///   LAT Wide Cal Indexes                ///
  /// Id all components of same type within ///
  /// currently configured lat.             ///
  /// internal integer storage is contiguous///
  /// and can be used as an array Index     ///
  /////////////////////////////////////////////
 
  /** \brief Abstract root class for indexing all Cal
      components w/in full LAT
  */
  class LATWideIndex {
  public:  
    /// prefix ++ operator
    LATWideIndex operator++() {
      LATWideIndex tmp(*this);
      m_data++;
      return tmp;
    }

    /// postfix ++ operator
    LATWideIndex& operator++(int) {
      m_data++; 
      return *this;
    }
    
    unsigned val() const {return m_data;}
    
    bool operator==(const LATWideIndex &that) const {return m_data == that.m_data;}
    bool operator<=(const LATWideIndex &that) const {return m_data <= that.m_data;}
    bool operator!=(const LATWideIndex &that) const {return m_data != that.m_data;}
    bool operator< (const LATWideIndex &that) const {return m_data <  that.m_data;}
    //LATWideIndex& operator= (const LATWideIndex &that) {m_data = that.m_data;}
  protected:
    LATWideIndex(unsigned val) : m_data(val) {}
    LATWideIndex() : m_data(0) {}
    unsigned m_data;
  };

  class XtalIdx : public LATWideIndex {
  public:
    XtalIdx(const idents::CalXtalId &xtal) :
      LATWideIndex(calc(xtal.getTower(),
                        xtal.getLayer(),
                        xtal.getColumn())) {}
        

    XtalIdx(TwrNum twr, LyrNum lyr, ColNum col) :
      LATWideIndex(calc(twr,lyr,col)) {}

    XtalIdx() : LATWideIndex() {}

    idents::CalXtalId getCalXtalId() const {
      return idents::CalXtalId(getTwr(),
                               getLyr(),
                               getCol());
    }
    static const unsigned N_VALS  = LyrNum::N_VALS*ColNum::N_VALS;

    TwrNum getTwr() const {return m_data/TWR_BASE;}
    LyrNum getLyr() const {return (m_data%TWR_BASE)/LYR_BASE;}
    ColNum getCol() const {return m_data%LYR_BASE;}

    /// operator to put XtalIdx to output stream
    friend ostream& operator<< (ostream &stream, const XtalIdx &idx);
    bool isValid() const {return m_data < N_VALS;}
    
  private:
    static unsigned calc(TwrNum twr, LyrNum lyr, ColNum col) {
      return twr*TWR_BASE + lyr*LYR_BASE + col;
    }
    static const unsigned LYR_BASE  = ColNum::N_VALS;
    static const unsigned TWR_BASE  = LYR_BASE*LyrNum::N_VALS;
  };
  
  class FaceIdx : public LATWideIndex {
  public:
    FaceIdx() : LATWideIndex() {}

    FaceIdx(const idents::CalXtalId &xtalId) {
      if (!xtalId.validFace())
        throw invalid_argument("FaceIdx requires valid face info in xtalId."
                               "  Programmer error");

      m_data = calc(xtalId.getTower(),
                    xtalId.getLayer(),
                    xtalId.getColumn(),
                    xtalId.getFace());
    }

    FaceIdx(TwrNum twr, LyrNum lyr, ColNum col, FaceNum face) :
      LATWideIndex(calc(twr,lyr,col,face)) {}

    FaceIdx(XtalIdx xtal, FaceNum face) :
      LATWideIndex(calc(xtal.getTwr(), xtal.getLyr(), xtal.getCol(), face)) {}

    idents::CalXtalId getCalXtalId() const {
      return idents::CalXtalId(getTwr(),
                               getLyr(),
                               getCol(),
                               getFace());
    }

    static const unsigned N_VALS = XtalIdx::N_VALS*FaceNum::N_VALS;

    XtalIdx getXtalIdx() const {return XtalIdx(getTwr(),
                                               getLyr(),
                                               getCol());}

    TwrNum getTwr()  const {return m_data/TWR_BASE;}
    LyrNum getLyr()  const {return (m_data%TWR_BASE)/LYR_BASE;}
    ColNum getCol()  const {return (m_data%LYR_BASE)/COL_BASE;}
    FaceNum getFace() const {return m_data%COL_BASE;}

    void setFace(FaceNum face) {
      m_data = calc(getTwr(),getLyr(),getCol(),face);
    }

    /// operator to put FaceIdx to output stream
    friend ostream& operator<< (ostream &stream, const FaceIdx &idx);
    
    bool isValid() const {return m_data < N_VALS;}
  private:
    static unsigned calc(TwrNum twr, LyrNum lyr, ColNum col, FaceNum face) {
      return twr*TWR_BASE + lyr*LYR_BASE + col*COL_BASE + face;
    }
    static const unsigned COL_BASE  = FaceNum::N_VALS;
    static const unsigned LYR_BASE  = COL_BASE*ColNum::N_VALS;
    static const unsigned TWR_BASE  = LYR_BASE*LyrNum::N_VALS;
  };

  class DiodeIdx : public LATWideIndex {
  public:
    DiodeIdx(const idents::CalXtalId &xtalId, DiodeNum diode) {
      if (!xtalId.validFace())
        throw invalid_argument("DiodeIdx requires valid face info in xtalId.  Programmer error");

      m_data = calc(xtalId.getTower(),
                    xtalId.getLayer(),
                    xtalId.getColumn(),
                    xtalId.getFace(), 
                    diode);
    }

    DiodeIdx(TwrNum twr, LyrNum lyr, ColNum col, FaceNum face, DiodeNum diode) :
      LATWideIndex(calc(twr,lyr,col,face,diode)) {}

    DiodeIdx(XtalIdx xtal, FaceNum face, DiodeNum diode) :
      LATWideIndex(calc(xtal.getTwr(),xtal.getLyr(),xtal.getCol(),face,diode)) {}

    DiodeIdx(XtalIdx xtal, XtalDiode xDiode) :
      LATWideIndex(calc(xtal.getTwr(), xtal.getLyr(), xtal.getCol(),
                        xDiode.getFace(), xDiode.getDiode())) {}

    DiodeIdx(FaceIdx face, DiodeNum diode) :
      LATWideIndex(calc(face.getTwr(),face.getLyr(),face.getCol(),face.getFace(),diode)) {}
    
    DiodeIdx() : LATWideIndex() {}

    static const unsigned N_VALS = FaceIdx::N_VALS*DiodeNum::N_VALS;

    TwrNum getTwr()   const {return m_data/TWR_BASE;}
    LyrNum getLyr()   const {return (m_data%TWR_BASE)/LYR_BASE;}
    ColNum getCol()   const {return (m_data%LYR_BASE)/COL_BASE;}
    FaceNum getFace()  const {return (m_data%COL_BASE)/FACE_BASE;}
    DiodeNum getDiode() const {return m_data%FACE_BASE;}

    /// operator to put DiodeIdx to output stream
    friend ostream& operator<< (ostream &stream, const DiodeIdx &idx);

    XtalIdx getXtalIdx() const {return XtalIdx(getTwr(),
                                               getLyr(),
                                               getCol());}

    FaceIdx getFaceIdx() const {return FaceIdx(getTwr(),
                                               getLyr(),
                                               getCol(),
                                               getFace());}

    
    bool isValid() const {return m_data < N_VALS;}
  private:
    static unsigned calc(TwrNum twr, LyrNum lyr, ColNum col, FaceNum face, DiodeNum diode) {
      return twr*TWR_BASE + lyr*LYR_BASE + col*COL_BASE + face*FACE_BASE + diode;
    }
    static const unsigned FACE_BASE = DiodeNum::N_VALS;
    static const unsigned COL_BASE  = FACE_BASE*FaceNum::N_VALS;
    static const unsigned LYR_BASE  = COL_BASE*ColNum::N_VALS;
    static const unsigned TWR_BASE  = LYR_BASE*LyrNum::N_VALS;
  };

  class RngIdx : public LATWideIndex {
  public:
    RngIdx(const idents::CalXtalId &xtalId) {
      if (!xtalId.validFace() || ! xtalId.validRange())
        throw invalid_argument("RngIdx requires valid face and range info in xtalId.  Programmer error");
      m_data = calc(xtalId.getTower(),xtalId.getLayer(),xtalId.getColumn(),xtalId.getFace(), xtalId.getRange());
    }

    RngIdx(TwrNum twr, LyrNum lyr, ColNum col, FaceNum face, RngNum rng) :
      LATWideIndex(calc(twr,lyr,col,face,rng)) 
      {}
    
    RngIdx(XtalIdx xtal, FaceNum face, RngNum rng) :
      LATWideIndex(calc(xtal.getTwr(),xtal.getLyr(),xtal.getCol(),face,rng)) {}

    RngIdx(XtalIdx xtal, XtalRng xRng) :
      LATWideIndex(calc(xtal.getTwr(), xtal.getLyr(), xtal.getCol(),
                        xRng.getFace(),xRng.getRng())) {}

    RngIdx(FaceIdx faceIdx, RngNum rng) :
      LATWideIndex(calc(faceIdx.getTwr(),faceIdx.getLyr(),faceIdx.getCol(),faceIdx.getFace(),rng)) {}
    
    RngIdx() : LATWideIndex() {}

    idents::CalXtalId getCalXtalId() const {
      return idents::CalXtalId(getTwr(),
                               getLyr(),
                               getCol(),
                               getFace(),
                               getRng());
    }
    
    TwrNum getTwr()  const {return m_data/TWR_BASE;}
    LyrNum getLyr()  const {return (m_data%TWR_BASE)/LYR_BASE;}
    ColNum getCol()  const {return (m_data%LYR_BASE)/COL_BASE;}
    FaceNum getFace() const {return (m_data%COL_BASE)/FACE_BASE;}
    RngNum getRng()  const {return m_data%FACE_BASE;}

    void setFace(FaceNum face) {
      m_data = calc(getTwr(),getLyr(),getCol(),face,getRng());
    }
    void setRng(RngNum rng) {
      m_data = calc(getTwr(),getLyr(),getCol(),getFace(),rng);
    }
    
    static const unsigned N_VALS = FaceIdx::N_VALS*RngNum::N_VALS;

    /// operator to put DiodeIdx to output stream
    friend ostream& operator<< (ostream &stream, const RngIdx &idx);
    
    XtalIdx getXtalIdx() const {return XtalIdx(getTwr(),
                                               getLyr(),
                                               getCol());}

    FaceIdx getFaceIdx() const {return FaceIdx(getTwr(),
                                               getLyr(),
                                               getCol(),
                                               getFace());}

    bool isValid() const {return m_data < N_VALS;}
  private:
    static unsigned calc(TwrNum twr, LyrNum lyr, ColNum col, FaceNum face, RngNum rng) {
      return twr*TWR_BASE + lyr*LYR_BASE + col*COL_BASE + face*FACE_BASE + rng;
    }
    static const unsigned FACE_BASE = RngNum::N_VALS;
    static const unsigned COL_BASE  = FACE_BASE*FaceNum::N_VALS;
    static const unsigned LYR_BASE  = COL_BASE*ColNum::N_VALS;
    static const unsigned TWR_BASE  = LYR_BASE*LyrNum::N_VALS; 
  };


  /** \brief id for 4 classes of Cal xtal asymmetry 

  4 asymmetry classes are
  - LL = lrg diode on both faces
  - LS = lrg diode on positive face, sm diode on neg
  - SL = sm. diode on pos. face, lrg diode on neg
  - SS = sm diode on both faces
      
  */

  class AsymType : public SimpleId {
  public:
    AsymType() : SimpleId() {}
    AsymType(DiodeNum posDiode, DiodeNum negDiode) :
      SimpleId(posDiode.val()*2 + negDiode.val()) {};
    
    inline DiodeNum getDiode(FaceNum face) {
      switch (face.val()) {
      case POS_FACE:
        return DiodeNum(m_data/2);
      case NEG_FACE:
        return DiodeNum(m_data%2);
      default:
        throw invalid_argument("Bad FaceNum value. Programmer error");
      }
    }

    static const unsigned short N_VALS = 4; 

    bool isValid() const {return m_data < N_VALS;}


    bool operator==(const AsymType &that) const {return m_data == that.m_data;}
    bool operator!=(const AsymType &that) const {return m_data != that.m_data;}

    /// return string representation
    const string &getMnem() const {return m_str[m_data];}
    
  private:
    static const string m_str[N_VALS];

  };
  const AsymType ASYM_LL(LRG_DIODE, LRG_DIODE);
  const AsymType ASYM_LS(LRG_DIODE, SM_DIODE);
  const AsymType ASYM_SL(SM_DIODE, LRG_DIODE);
  const AsymType ASYM_SS(SM_DIODE, SM_DIODE);

}; // namespace caldefs

#endif // CalDefs_H
