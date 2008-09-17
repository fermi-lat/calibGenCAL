#ifndef GCRHists_h
#define GCRHists_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Hists/GCRHists.h,v 1.10 2008/04/21 20:32:32 fewtrell Exp $

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "HistIdx.h"
#include "HistVec.h"
#include "HistMap.h"
#include "src/lib/Specs/singlex16.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalVec.h"

// EXTLIB INCLUDES
#include "TProfile.h"
#include "TH1S.h"
#include "TH1I.h"

// STD INCLUDES
#include <string>
#include <memory>

class TDirectory;

namespace CalUtil {
  class CalMPD;
}

namespace calibGenCAL {

  /// composite index (xtalIdx, diode size)
  class XtalDiodeId : public CalUtil::LATWideIndex {
  public:
    XtalDiodeId(const CalUtil::XtalIdx xtalIdx,
                const CalUtil::DiodeNum diode) :
      LATWideIndex(calc(xtalIdx, diode))
    {}

    /// construct from raw integer format (same as from val() method)
    explicit XtalDiodeId(const unsigned raw=0) :
      LATWideIndex(raw)
    {}

    /// construct from string repr
    explicit XtalDiodeId(const std::string &name);

    CalUtil::XtalIdx getXtalIdx() const { 
      CalUtil::XtalIdx xtalIdx;
      xtalIdx.setVal(m_data/XTAL_BASE); 
      return xtalIdx;
    }

    CalUtil::DiodeNum getDiode() const { return CalUtil::DiodeNum(m_data%XTAL_BASE); }

    std::string toStr() const {
      std::ostringstream tmp;
      tmp << getXtalIdx().toStr() + "_" + getDiode().toStr();
      return tmp.str();
    }
    
    bool isValid() const {
      return m_data < N_VALS;
    }
    static const unsigned N_VALS = CalUtil::XtalIdx::N_VALS*CalUtil::DiodeNum::N_VALS;

  private:
    static unsigned calc(const CalUtil::XtalIdx xtalIdx,
                         const CalUtil::DiodeNum diode) {
      return xtalIdx.val()*XTAL_BASE + diode.val();
    }

    static const unsigned short XTAL_BASE = CalUtil::DiodeNum::N_VALS;
    static const unsigned short N_FIELDS = 2;

  };


  inline std::string toPath(const XtalDiodeId &id) {
    return toPath(id.getXtalIdx()) + "/" + "D" + toPath(id.getDiode());
  }

  /// composite index (xtalIdx, diode size)
  class LyrDiodeId : public CalUtil::LATWideIndex {
  public:
    LyrDiodeId(const CalUtil::LyrNum lyr,
               const CalUtil::DiodeNum diode) :
      LATWideIndex(calc(lyr, diode))

    {}

    /// construct from raw integer format (same as from val() method)
    explicit LyrDiodeId(const unsigned raw=0) :
      LATWideIndex(raw)
    {}

    /// construct from string repr
    explicit LyrDiodeId(const std::string &name);

    CalUtil::LyrNum getLyr() const { return CalUtil::LyrNum(m_data/LYR_BASE); }

    CalUtil::DiodeNum getDiode() const { return CalUtil::DiodeNum(m_data%LYR_BASE); }

    std::string toStr() const {
      std::ostringstream tmp;
      tmp << getLyr().toStr() + "_" + getDiode().toStr();
      return tmp.str();
    }
    
    bool isValid() const {
      return m_data < N_VALS;
    }
    static const unsigned N_VALS = CalUtil::LyrNum::N_VALS*CalUtil::DiodeNum::N_VALS;

  private:
    static unsigned calc(const CalUtil::LyrNum lyr,
                         const CalUtil::DiodeNum diode) {
      return lyr.val()*LYR_BASE + diode.val();
    }

    static const unsigned short LYR_BASE = CalUtil::DiodeNum::N_VALS;
    static const unsigned short N_FIELDS = 2;

  };

  inline std::string toPath(const LyrDiodeId &id) {
    return toPath(id.getLyr()) + "/" + "D" + toPath(id.getDiode());
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

  /** \brief Represents GLAST Cal Optical gain calibration constants
      (MeV <-> CIDAC)

      contains read & write methods to various file formats 

      @author Zachary Fewtrell
  */
  class GCRHists {
  public:
    /// \param summaryMode enable to skip individual xtal histograms & only keep 'summary' histograms
    /// \param mevMode enable to generate histograms with mev scale axis
    /// \property rootDir all histograms for this collection are loaded / saved inside this directory
    /// \param writeDir (if non-zero) all new histograms will be written out to this directory opun class destruction.
    /// \param readDir (if non-zero) any associated histograms will be read from this directory upon construction 
    GCRHists(const bool summaryMode,
             const bool mevMode,
             TDirectory *const writeDir=0,
             TDirectory *const readDir=0);

    /// print histogram summary info to output stream
    void summarizeHists(ostream &ostrm) const;

    /// fill all associated histograms w/ valid CIDAC hit for given channel & particle Z
    /// \parm inferredZ inferred z for particle
    void fillMeanCIDACZ(const CalUtil::XtalIdx xtalIdx,
                        const CalUtil::DiodeNum diode, 
                        const unsigned short inferredZ,
                        const float cidac);

    /// fill all associated histograms w/ mev for given channel
    void fillMeV(const CalUtil::XtalIdx xtalIdx,
                 const CalUtil::DiodeNum diode, 
                 const float mev,
                 const unsigned short inferredZ);
    
    /// fill all associated histograms w/ valid CIDAC hit for given channel
    void fillMeanCIDAC(const CalUtil::XtalIdx xtalIdx,
                       const CalUtil::DiodeNum diode, 
                       const float cidac);

    /// fill all associated histograms w/ ratio of le & he cidac signals
    void fillDACRatio(const CalUtil::FaceIdx faceIdx,
                      const float leDAC,
                      const float heDAC);
  
    /// report minimum # of entries in any histogram
    unsigned minEntries() const;

  private:
    /// load all associated histogram from m_readDir
    void loadHists(TDirectory &dir);

    /// build new empty histogram data
    void initHists();

    /// set current directory for all histograms
    void setDirectory(TDirectory *const dir);

    /// collection of histograms with mean CIDAC values for each xtal
    typedef HistVec<XtalDiodeId, TH1S> MeanDACHistCol;
    /// sparse array stores histograms indexed by tuple(XtalIdx,DiodeNum)
    std::auto_ptr<MeanDACHistCol> m_meanDACHists;
    
    /// ratio between mean LE & HE CIDAC per xtal
    typedef HistVec<CalUtil::FaceIdx, TProfile> DACRatioProfCol;
    /// optionally disbaled when m_summaryMode = false
    std::auto_ptr<DACRatioProfCol> m_dacRatioProfs;

    /// collection of histograms with mev values for all z's summed over all xtals
    typedef HistVec<CalUtil::DiodeNum, TH1I>  MeVSumHistCol;

    /// sum over all xtals & all inferredZ's (optional)
    std::auto_ptr<MeVSumHistCol> m_mevSumHists;

    typedef HistVec<LyrDiodeId, TH1I> MeVSumLyrHistCol;
    /// sum over all xtals & all inferredZ's (optional)
    std::auto_ptr<MeVSumLyrHistCol> m_mevSumLyrHists;

    typedef HistMap<ZDiodeId, TH1I> MeVSumZHistCol;
    auto_ptr<MeVSumZHistCol> m_mevSumZHists;

    /// histogrm inferred INFERREDZ values
    TH1I *m_zHist;

    /// sum over all xtals
    TProfile *m_dacRatioSumProf;
  
    /// generate summary histograms only (no individual channels)
    const bool m_summaryMode;

    /// optionally generate histograms with mev axis scale
    const bool m_mevMode;

    /// all new (& modified) histograms written to this directory
    TDirectory *const m_writeDir;
  };
}; // namespace calibGenCAL
#endif
