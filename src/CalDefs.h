#ifndef CalDefs_H
#define CalDefs_H 1

#include "idents/CalXtalId.h"

#include <string>
#include <vector>
#include <ostream>

using namespace std;

namespace CalDefs {
  // CalXtalId shorthand
  const short POS_FACE = (short)idents::CalXtalId::POS;
  const short NEG_FACE = (short)idents::CalXtalId::NEG;

  const short LEX8 = (short)idents::CalXtalId::LEX8;
  const short LEX1 = (short)idents::CalXtalId::LEX1;
  const short HEX8 = (short)idents::CalXtalId::HEX8;
  const short HEX1 = (short)idents::CalXtalId::HEX1;

  const short LARGE_DIODE = (short)idents::CalXtalId::LARGE;
  const short SMALL_DIODE = (short)idents::CalXtalId::SMALL;

  const short X_DIR = 0;
  const short Y_DIR = 1;

  /// names for volume identifier fields
  enum {fLATObjects, fTowerY, fTowerX, fTowerObjects, fLayer,
        fMeasure, fCALXtal,fCellCmp, fSegment};
 
  /// Generic typesafe vector used for fast & simple arrays
  /// based on various Calorimeter geometry idents
  template <class CalSize, class _Tp >
    class CalVec : protected vector<_Tp > {
    public:

    CalVec() : parent_type() {};
    CalVec(const size_type &sz) : parent_type(sz){}
    CalVec(const size_type &sz, const value_type &val) : parent_type(sz,val){}

    reference operator[] (const CalSize &idx) {
      return vector<value_type>::operator[](idx.getInt());
    }
    const_reference operator[] (const CalSize &idx) const {
      return parent_type::operator[](idx.getInt());
    }

    reference at(const CalSize &idx) {
      return parent_type::at(idx.getInt());
    }
    const_reference at(const CalSize &idx) const {
      return parent_type::at(idx.getInt());
    }

    void resize(size_type sz) {
      parent_type::resize(sz);
    }

    void clear() {
      parent_type::clear();
    }
    
    void erase() {
      parent_type::erase();
    }

    size_type size() const {
      return parent_type::size();
    }
 
    const_iterator begin() const {return parent_type::begin();}
    iterator begin() {return parent_type::begin();}

    const_iterator end() const {return parent_type::end();}
    iterator end() {return parent_type::end();}
    
    private:
    typedef vector<value_type> parent_type;
  };

  ///////////////////////////////////////////
  //// Atomic (simple) Cal Geometry Id's  ///
  //// Id single Cal component w/ in it's ///
  //// immediate container                ///
  ///////////////////////////////////////////

  class SimpleId {
  public:
    operator int() const {return m_data;}

    /// prefix ++ operator
    SimpleId operator++() {
      SimpleId tmp(*this);
      m_data++;
      return tmp;
    }

    /// postfix ++ operator
    SimpleId& operator++(int tmp) {
      m_data++; 
      return *this;
    }

    short getInt() const {return m_data;}
  protected:
    SimpleId(short val) : m_data(val) {}
    SimpleId() : m_data(0) {}
    short m_data;
  };

  class TwrNum : public SimpleId {
  public:
    TwrNum() : SimpleId() {}
    TwrNum(short val) : SimpleId(val) {}
    static const short N_VALS=1; // only supportinjg 1 tower currently
    bool isValid() const {return m_data < N_VALS;}
  };

  class LyrNum : public SimpleId {
  public:
    LyrNum() : SimpleId() {}
    LyrNum(short val) : SimpleId(val) {}
    static const short N_VALS=8;
    bool isValid() const {return m_data < N_VALS;}
    short getDir() const {return (m_data%2 == 0) ? X_DIR : Y_DIR;} // 0 is 'X' Direction
    short getXLyr() const {return m_data/2;}
    short getYLyr() const {return (m_data-1)/2;}
  };

  class DirNum : public SimpleId {
  public:
    DirNum() : SimpleId() {}
    DirNum(short val) : SimpleId(val) {}
    static const short N_VALS=2;
    bool isValid() const {return m_data < N_VALS;}
  };

  class ColNum : public SimpleId {
  public:
    ColNum() : SimpleId() {}
    ColNum(short val) : SimpleId(val) {}
    static const short N_VALS=12;
    bool isValid() const {return m_data < N_VALS;}
  };

  class FaceNum : public SimpleId {
  public:
    FaceNum() : SimpleId() {}
    FaceNum(short val) : SimpleId(val) {}
    static const vector<string> MNEM;
    static const short N_VALS=2;    
    bool isValid() const {return m_data < N_VALS;}
  };

  class DiodeNum : public SimpleId {
  public:
    DiodeNum(short val) : SimpleId(val) {}
    DiodeNum(void) : SimpleId() {}
    
    short getX8Rng() const {return m_data*2;}
    short getX1Rng() const {return getX8Rng()+1;}

    static const vector<string> MNEM;
    static const short N_VALS=2; 
    bool isValid() const {return m_data < N_VALS;}
  };

  class RngNum : public SimpleId {
  public:
    RngNum(short val) : SimpleId(val) {}
    RngNum(void) : SimpleId() {}

    static const vector<string> MNEM;
    static const short N_VALS=4;
    
    short getDiode() const {
      return idents::CalXtalId::rangeToDiode((idents::CalXtalId::AdcRange)m_data);
    }
    bool isValid() const {return m_data < N_VALS;}
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
    XtalWideIndex& operator++(int tmp) {
      m_data++; 
      return *this;
    }

    //operator int() const {return m_data;}

    bool operator==(const XtalWideIndex &that) const {return m_data == that.m_data;}
    bool operator<=(const XtalWideIndex &that) const {return m_data <= that.m_data;}
    bool operator!=(const XtalWideIndex &that) const {return m_data != that.m_data;}
    bool operator< (const XtalWideIndex &that) const {return m_data <  that.m_data;}
    //XtalWideIndex& operator= (const XtalWideIndex &that) {m_data = that.m_data;}

    int getInt() const {return m_data;}

  protected:
    XtalWideIndex(short val) : m_data(val) {}
    XtalWideIndex() : m_data(0) {}
    short m_data;
  };

  class XtalDiode : public XtalWideIndex {
  public: 
    XtalDiode(const short face, const short diode) :
      XtalWideIndex(face*FACE_BASE + diode) {}

    XtalDiode() : XtalWideIndex() {}

    short getDiode() const {return m_data%FACE_BASE;}
    short getFace()  const {return m_data/FACE_BASE;}

    static const short N_VALS = FaceNum::N_VALS*DiodeNum::N_VALS;
    bool isValid() const {return m_data < N_VALS;}
  protected:
    static const short FACE_BASE = DiodeNum::N_VALS;
  };

  class XtalRng : public XtalWideIndex {
  public:
    XtalRng(short face, short rng) :
      XtalWideIndex(face*FACE_BASE + rng) {};

    XtalRng() : XtalWideIndex() {}

    short getFace() const {return m_data/FACE_BASE;}
    short getRng() const {return m_data%FACE_BASE;}

    XtalDiode getXtalDiode() const {
      return XtalDiode(getFace(), ((RngNum)getRng()).getDiode());
    }

    static const short N_VALS = FaceNum::N_VALS*RngNum::N_VALS;
    bool isValid() const {return m_data < N_VALS;}
  protected:
    static const short FACE_BASE = RngNum::N_VALS;
  };

  /////////////////////////////////////////////
  ///   LAT Wide Cal Indexes                ///
  /// Id all components of same type within ///
  /// currently configured lat.             ///
  /// internal integer storage is contiguous///
  /// and can be used as an array Index     ///
  /////////////////////////////////////////////

  class LATWideIndex {
  public:  
    /// prefix ++ operator
    LATWideIndex operator++() {
      LATWideIndex tmp(*this);
      m_data++;
      return tmp;
    }

    /// postfix ++ operator
    LATWideIndex& operator++(int tmp) {
      m_data++; 
      return *this;
    }
    
    int getInt() const {return m_data;}
    
    bool operator==(const LATWideIndex &that) const {return m_data == that.m_data;}
    bool operator<=(const LATWideIndex &that) const {return m_data <= that.m_data;}
    bool operator!=(const LATWideIndex &that) const {return m_data != that.m_data;}
    bool operator< (const LATWideIndex &that) const {return m_data <  that.m_data;}
    //LATWideIndex& operator= (const LATWideIndex &that) {m_data = that.m_data;}
  protected:
    LATWideIndex(int val) : m_data(val) {}
    LATWideIndex() : m_data(0) {}
    unsigned m_data;
  };

  class XtalIdx : public LATWideIndex {
  public:
    XtalIdx(const idents::CalXtalId &xtal) :
      LATWideIndex(calc(xtal.getTower(),
                        xtal.getLayer(),
                        xtal.getColumn())) {}
        

    XtalIdx(short twr, short lyr, short col) :
      LATWideIndex(calc(twr,lyr,col)) {}

    XtalIdx() : LATWideIndex() {}

    idents::CalXtalId getCalXtalId() const {
      return idents::CalXtalId(getTwr(),
                               getLyr(),
                               getCol());
    }
    static const unsigned N_VALS  = LyrNum::N_VALS*ColNum::N_VALS;

    short getTwr() const {return m_data/TWR_BASE;}
    short getLyr() const {return (m_data%TWR_BASE)/LYR_BASE;}
    short getCol() const {return m_data%LYR_BASE;}

    /// operator to put XtalIdx to output stream
    friend ostream& operator<< (ostream &stream, const XtalIdx &idx);
    bool isValid() const {return m_data < N_VALS;}
    
  private:
    static int calc(short twr, short lyr, short col) {
      return twr*TWR_BASE + lyr*LYR_BASE + col;
    }
    static const int LYR_BASE  = ColNum::N_VALS;
    static const int TWR_BASE  = LYR_BASE*LyrNum::N_VALS;
  };
  
  class FaceIdx : public LATWideIndex {
  public:
    FaceIdx() : LATWideIndex() {}

    FaceIdx(const idents::CalXtalId &xtalId) {
      if (!xtalId.validFace())
        throw invalid_argument("FaceIdx requires valid face info in xtalId.  Programmer error");

      m_data = calc(xtalId.getTower(),
                    xtalId.getLayer(),
                    xtalId.getColumn(),
                    xtalId.getFace());
    }

    FaceIdx(short twr, short lyr, short col, short face) :
      LATWideIndex(calc(twr,lyr,col,face)) {}

    FaceIdx(XtalIdx xtal, short face) :
      LATWideIndex(calc(xtal.getTwr(), xtal.getLyr(), xtal.getCol(), face)) {}

    idents::CalXtalId getCalXtalId() const {
      return idents::CalXtalId(getTwr(),
                               getLyr(),
                               getCol(),
                               getFace());
    }

    static const int N_VALS = XtalIdx::N_VALS*FaceNum::N_VALS;

    XtalIdx getXtalIdx() const {return XtalIdx(getTwr(),
                                               getLyr(),
                                               getCol());}

    short getTwr()  const {return m_data/TWR_BASE;}
    short getLyr()  const {return (m_data%TWR_BASE)/LYR_BASE;}
    short getCol()  const {return (m_data%LYR_BASE)/COL_BASE;}
    short getFace() const {return m_data%COL_BASE;}

    void setFace(short face) {
      m_data = calc(getTwr(),getLyr(),getCol(),face);
    }

    /// operator to put FaceIdx to output stream
    friend ostream& operator<< (ostream &stream, const FaceIdx &idx);
    
    bool isValid() const {return m_data < N_VALS;}
  private:
    static int calc(short twr, short lyr, short col, short face) {
      return twr*TWR_BASE + lyr*LYR_BASE + col*COL_BASE + face;
    }
    static const short COL_BASE  = FaceNum::N_VALS;
    static const short LYR_BASE  = COL_BASE*ColNum::N_VALS;
    static const short TWR_BASE  = LYR_BASE*LyrNum::N_VALS;
  };

  class DiodeIdx : public LATWideIndex {
  public:
    DiodeIdx(const idents::CalXtalId &xtalId, short diode) {
      if (!xtalId.validFace())
        throw invalid_argument("DiodeIdx requires valid face info in xtalId.  Programmer error");

      m_data = calc(xtalId.getTower(),
                    xtalId.getLayer(),
                    xtalId.getColumn(),
                    xtalId.getFace(), 
                    diode);
    }

    DiodeIdx(short twr, short lyr, short col, short face, short diode) :
      LATWideIndex(calc(twr,lyr,col,face,diode)) {}

    DiodeIdx(XtalIdx xtal, short face, short diode) :
      LATWideIndex(calc(xtal.getTwr(),xtal.getLyr(),xtal.getCol(),face,diode)) {}

    DiodeIdx(XtalIdx xtal, XtalDiode xDiode) :
      LATWideIndex(calc(xtal.getTwr(), xtal.getLyr(), xtal.getCol(),
			xDiode.getFace(), xDiode.getDiode())) {}

    DiodeIdx(FaceIdx face, short diode) :
      LATWideIndex(calc(face.getTwr(),face.getLyr(),face.getCol(),face.getFace(),diode)) {}
    
    DiodeIdx() : LATWideIndex() {}

    static const int N_VALS = FaceIdx::N_VALS*DiodeNum::N_VALS;

    short getTwr()   const {return m_data/TWR_BASE;}
    short getLyr()   const {return (m_data%TWR_BASE)/LYR_BASE;}
    short getCol()   const {return (m_data%LYR_BASE)/COL_BASE;}
    short getFace()  const {return (m_data%COL_BASE)/FACE_BASE;}
    short getDiode() const {return m_data%FACE_BASE;}

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
    static int calc(short twr, short lyr, short col, short face, short diode) {
      return twr*TWR_BASE + lyr*LYR_BASE + col*COL_BASE + face*FACE_BASE + diode;
    }
    static const int FACE_BASE = DiodeNum::N_VALS;
    static const int COL_BASE  = FACE_BASE*FaceNum::N_VALS;
    static const int LYR_BASE  = COL_BASE*ColNum::N_VALS;
    static const int TWR_BASE  = LYR_BASE*LyrNum::N_VALS;
  };

  class RngIdx : public LATWideIndex {
  public:
    RngIdx(const idents::CalXtalId &xtalId) {
      if (!xtalId.validFace() || ! xtalId.validRange())
        throw invalid_argument("RngIdx requires valid face and range info in xtalId.  Programmer error");
      m_data = calc(xtalId.getTower(),xtalId.getLayer(),xtalId.getColumn(),xtalId.getFace(), xtalId.getRange());
    }

    RngIdx(short twr, short lyr, short col, short face, short rng) :
      LATWideIndex(calc(twr,lyr,col,face,rng)) 
      {}
    
    RngIdx(XtalIdx xtal, short face, short rng) :
      LATWideIndex(calc(xtal.getTwr(),xtal.getLyr(),xtal.getCol(),face,rng)) {}

    RngIdx(XtalIdx xtal, XtalRng xRng) :
      LATWideIndex(calc(xtal.getTwr(), xtal.getLyr(), xtal.getCol(),
			xRng.getFace(),xRng.getRng())) {}

    RngIdx(FaceIdx faceIdx, short rng) :
      LATWideIndex(calc(faceIdx.getTwr(),faceIdx.getLyr(),faceIdx.getCol(),faceIdx.getFace(),rng)) {}
    
    RngIdx() : LATWideIndex() {}

    idents::CalXtalId getCalXtalId() const {
      return idents::CalXtalId(getTwr(),
                               getLyr(),
                               getCol(),
                               getFace(),
                               getRng());
    }
    
    short getTwr()  const {return m_data/TWR_BASE;}
    short getLyr()  const {return (m_data%TWR_BASE)/LYR_BASE;}
    short getCol()  const {return (m_data%LYR_BASE)/COL_BASE;}
    short getFace() const {return (m_data%COL_BASE)/FACE_BASE;}
    short getRng()  const {return m_data%FACE_BASE;}

    void setFace(short face) {
      m_data = calc(getTwr(),getLyr(),getCol(),face,getRng());
    }
    void setRng(short rng) {
      m_data = calc(getTwr(),getLyr(),getCol(),getFace(),rng);
    }
    
    static const int N_VALS = FaceIdx::N_VALS*RngNum::N_VALS;

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
    static int calc(short twr, short lyr, short col, short face, short rng) {
      return twr*TWR_BASE + lyr*LYR_BASE + col*COL_BASE + face*FACE_BASE + rng;
    }
    static const int FACE_BASE = RngNum::N_VALS;
    static const int COL_BASE  = FACE_BASE*FaceNum::N_VALS;
    static const int LYR_BASE  = COL_BASE*ColNum::N_VALS;
    static const int TWR_BASE  = LYR_BASE*LyrNum::N_VALS; 
  };


}; // namespace caldefs

#endif // CalDefs_H
