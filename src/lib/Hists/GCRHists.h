#ifndef GCRHists_h
#define GCRHists_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Hists/GCRHists.h,v 1.4 2007/04/24 16:45:07 fewtrell Exp $

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "../Util/CGCUtil.h"
#include "../Specs/singlex16.h"
#include "HistMap.h"
#include "HistVec.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalVec.h"
#include "CalUtil/CalArray.h"

// EXTLIB INCLUDES
#include "TProfile.h"
#include "TH1S.h"
#include "TFile.h"

// STD INCLUDES
#include <string>
#include <memory>

class TDirectory;

namespace calibGenCAL {
  class CalMPD;

  /** \brief Represents GLAST Cal Optical gain calibration constants
      (MeV <-> CIDAC)

      contains read & write methods to various file formats 

      @author Zachary Fewtrell
  */
  class GCRHists {
  public:
    /// \param summaryMode enable to skip individual xtal histograms & only keep 'summary' histograms
    /// \param rootFilePath if != "", then all matching histograms will be loaded from given file.
    GCRHists(const bool summaryMode, 
             const std::string &rootFilePath="");

    /// print histogram summary info to output stream
    void summarizeHists(ostream &ostrm) const;

    /// set ROOT directory for each histogram
    void setHistDir(TDirectory *const dir);

    /// fit histograms & save means
    /// \param calMPD write fitted mevPerDAC to calMPD
    /// \param fit a peak to each particle w/ given Zs
    void fitHists(CalMPD &calMPD);

    /// fill all associated histograms w/ valid CIDAC hit for given channel & particle Z
    /// \parm inferredZ inferred z for particle
    void fillMeanCIDACZ(const CalUtil::XtalIdx xtalIdx,
                       const CalUtil::DiodeNum diode, 
                       const unsigned short inferredZ,
                       const float cidac);

    /// fill all associated histograms w/ valid CIDAC hit for given channel
    void fillMeanCIDAC(const CalUtil::XtalIdx xtalIdx,
                       const CalUtil::DiodeNum diode, 
                       const float cidac);

    /// fill all associated histograms w/ ratio of signal between given adc range 
    /// & next higher range
    void fillAdcRatio(const CalUtil::RngIdx rngIdx, 
                      const float thisADC,
                      const float nextADC);

    /// fill all associated histograms w/ ratio of le & he cidac signals
    void fillDACRatio(const CalUtil::FaceIdx faceIdx,
                      const float leDAC,
                      const float heDAC);
  
    /// report minimum # of entries in any histogram
    unsigned minEntries() const;

  private:
    /// skip event processing & load histograms from previous analysis
    void loadHists(const string &filename);

    /// build new empty histogram data
    void initHists();
    
    /// index class for mean CIDAC histogram collection
    class MeanDACHistId {
    public:
      MeanDACHistId(const unsigned short inferredZ,
                    const CalUtil::XtalIdx xtalIdx,
                    const CalUtil::DiodeNum diode) :
        inferredZ(inferredZ),
        xtalIdx(xtalIdx),
        diode(diode)
      {}

      /// construct from raw integer format (same as from val() method)
      MeanDACHistId(const unsigned raw) :
        inferredZ((raw/CalUtil::DiodeNum::N_VALS)/CalUtil::XtalIdx::N_VALS),
        xtalIdx((raw/CalUtil::DiodeNum::N_VALS)%CalUtil::XtalIdx::N_VALS),
        diode(raw%CalUtil::DiodeNum::N_VALS)
      {}

      const unsigned short inferredZ;
      const CalUtil::XtalIdx xtalIdx;
      const CalUtil::DiodeNum diode;

      /// convert to string representation
      std::string toStr() const;

      bool operator==(const MeanDACHistId &that) const {
        return val() == that.val();
      }

      bool operator!=(const MeanDACHistId &that) const {
        return !(*this == that);
      }

      bool operator<(const MeanDACHistId &that) const {
        return val() < that.val();
      }

      /// convert to flat index for storage
      unsigned val() const {
        return diode.val() + CalUtil::DiodeNum::N_VALS*(xtalIdx.val() + inferredZ*CalUtil::XtalIdx::N_VALS);
      }
    };


    /// collection stores histograms indexed by tuple(inferredZ,XtalIdx,diode)
    /// as needed.
    typedef HistMap<MeanDACHistId, TH1S> MeanDACHistCol;
    std::auto_ptr<MeanDACHistCol> m_meanDACHists;

    /// index class for mean CIDAC histogram collection (all Z's combined)
    class MeanDACAllZHistId {
    public:
      MeanDACAllZHistId(const CalUtil::XtalIdx xtalIdx,
                        const CalUtil::DiodeNum diode) :
        xtalIdx(xtalIdx),
        diode(diode)
      {}

      /// construct from raw integer format (same as from val() method)
      MeanDACAllZHistId(const unsigned raw) :
        xtalIdx(raw/CalUtil::DiodeNum::N_VALS),
        diode(raw%CalUtil::DiodeNum::N_VALS)
      {}

      const CalUtil::XtalIdx xtalIdx;
      const CalUtil::DiodeNum diode;

      /// convert to string representation
      std::string toStr() const;

      bool operator==(const MeanDACAllZHistId &that) const {
        return val() == that.val();
      }

      bool operator!=(const MeanDACAllZHistId &that) const {
        return !(*this == that);
      }

      bool operator<(const MeanDACAllZHistId &that) const {
        return val() < that.val();
      }

      /// convert to flat index for storage
      unsigned val() const {
        return diode.val() + CalUtil::DiodeNum::N_VALS*xtalIdx.val();
      }
    };


    /// collection stores histograms indexed by tuple(XtalIdx,DiodeNum)
    /// as needed.
    typedef HistMap<MeanDACAllZHistId, TH1S> MeanDACAllZHistCol;
    std::auto_ptr<MeanDACAllZHistCol> m_meanDACAllZHists;
    
    
    /// ratio between mean LE & HE CIDAC per xtal
    typedef HistVec<CalUtil::FaceIdx, TProfile> DACRatioProfCol;
    /// optionally disbaled when summaryMode = false
    std::auto_ptr<DACRatioProfCol> m_dacRatioProfs;


    /// ratio between 2 adjacent ADC ranges (mean of both faces) per xtal
    /// \note outermost index from 0 -> 2 by lower of 2 compared ranges
    /// (i.e. index 0 is rng 0 vs rng 1, index 2 is rng 2 vs rng 3)
    typedef HistVec<CalUtil::RngIdx, TProfile> ADCRatioProfCol;
    /// optionally disbaled when summaryMode = false
    std::auto_ptr<ADCRatioProfCol> m_adcRatioProfs;

    /// index class for mean dac summary histograms
    class MeanDACSumHistId {
    public:
      MeanDACSumHistId(const unsigned short inferredZ,
                       const CalUtil::DiodeNum diode) :
        inferredZ(inferredZ),
        diode(diode) {}

      /// construct from raw integer format (same as from val() method)
      MeanDACSumHistId(const unsigned raw) :
        inferredZ(raw/CalUtil::DiodeNum::N_VALS),
        diode(raw%CalUtil::DiodeNum::N_VALS)
      {
      }


      const unsigned short inferredZ;
      const CalUtil::DiodeNum diode;

      /// convert to string representation
      std::string toStr() const;

      bool operator==(const MeanDACSumHistId &that) const {
        return val() == that.val();
      }

      bool operator!=(const MeanDACSumHistId &that) const {
        return !(*this == that);
      }

      bool operator<(const MeanDACSumHistId &that) const {
        return val() < that.val();
      } 

      /// convert to flat index for storage
      unsigned val() const {
        return inferredZ*CalUtil::DiodeNum::N_VALS + diode.val();
      }
    };

    /// sum over all xtals, outer map is for particle inferredZ, inner array for diode
    typedef HistMap<MeanDACSumHistId, TH1S> MeanDACSumHistCol;
    std::auto_ptr<MeanDACSumHistCol> m_meanDACSumHists;

    /// sum over all xtals & all inferredZ's
    TH1S *m_meanDACSumAllZ;

    /// histogrm inferred INFERREDZ values
    TH1S *m_zHist;

    /// sum over all xtals
    TProfile *m_dacRatioSumProf;
  
    /// sum over all xtals
    typedef HistVec<CalUtil::RngNum, TProfile> ADCRatioSumProfCol;
    std::auto_ptr<ADCRatioSumProfCol> m_adcRatioSumProfs;
  
    /// generate summary histograms only (no individual channels)
    const bool  summaryMode;
  };

}; // namespace calibGenCAL
#endif
