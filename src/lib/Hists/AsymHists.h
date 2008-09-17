#ifndef AsymHists_h
#define AsymHists_h

// $Header: //

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "src/lib/Hists/HistVec.h"
#include "src/lib/Specs/CalResponse.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalVec.h"
#include "CalUtil/SimpleCalCalib/CalAsym.h"

// EXTLIB INCLUDES
#include "TH2S.h"

// STD INCLUDES
#include <string>
#include <ostream>
#include <sstream>
#include <memory>


class TDirectory;

namespace CalUtil {

  class CalAsym;
}

namespace calibGenCAL {
  /// index class used for Asymmetry histograms
  class AsymHistId : public CalUtil::LATWideIndex {
  public:
    AsymHistId (const CalUtil::AsymType asymType,
                const CalUtil::XtalIdx xtalIdx) :
      LATWideIndex(calc(xtalIdx, asymType))
    {}

    /// construct from raw integer format (same as from val() method)
    explicit AsymHistId(const unsigned raw) :
      LATWideIndex(raw)
    {}

    /// construct from string repr
    explicit AsymHistId(const std::string &name);

    AsymHistId() : LATWideIndex()
    {}

    std::string toStr() const {
      std::ostringstream tmp;
      tmp << getXtalIdx().toStr() + "_" + getAsymType().toStr();
      return tmp.str();
    }
    
    CalUtil::XtalIdx getXtalIdx() const {
      CalUtil::XtalIdx retVal;
      retVal.setVal(m_data/XTAL_BASE);
      return retVal;
    }

    CalUtil::AsymType getAsymType() const {return CalUtil::AsymType(m_data%XTAL_BASE);}

    static const unsigned N_VALS = CalUtil::XtalIdx::N_VALS*CalUtil::AsymType::N_VALS;

    bool isValid() {return m_data < N_VALS;}
  private:
    static unsigned calc(const CalUtil::XtalIdx xtalIdx,
                         const CalUtil::AsymType asymType
                         ) {
      return xtalIdx.val()*XTAL_BASE + asymType.val();
    }

    static const unsigned XTAL_BASE = CalUtil::AsymType::N_VALS;

    static const unsigned short N_FIELDS = 2;
  };

  inline std::string toPath(const AsymHistId &id) {
    std::ostringstream tmp;
    tmp << toPath(id.getXtalIdx()) << "/"
        << toPath(id.getAsymType()) << "/";
    return tmp.str();
  }
  

  /** \brief Store histograms required to generate Calorimeter Light Asymmetry
      calibrations

      \note internal asymmetry histograms have average asymmetry slope subtracted from all values - this reduces artificial spread caused by asymmetry slope change across bin.

      @author Zachary Fewtrell

  */
  class AsymHists {
  public:
    /// \param nSlicesPerXtal number of slices along entire crystal length
    /// \param nSlicesPerHist only the middle 'n' slices will actually be measued as outer most slices have very high error
    /// \param calGain instrument gain setting is needed to determine histogram limits for mixed-diode asymmetry
    AsymHists(const CalResponse::CAL_GAIN_INTENT calGain=CalResponse::FLIGHT_GAIN,
              const unsigned short nSlicesPerXtal=12,
              const unsigned short nSlicesPerHist=10,
              TDirectory *const writeDir=0,
              TDirectory *const readDir=0);

    /// load histograms from ROOT output of previous run
    void        loadHists(TDirectory &readDir);

    /// print histogram summary info to output stream
    void        summarizeHists(std::ostream &ostrm) const;

    void        fitHists(CalUtil::CalAsym &calAsym);

    /// return pointer to histogram for given index, return 0 if it doesn't exist
    const TH2S *getHist(const CalUtil::AsymType asymType,
                  const CalUtil::XtalIdx xtalIdx) const {
      return m_asymHists->getHist(AsymHistId(asymType, xtalIdx));
    }

    /// fill appropriate asymmetry histogram
    /// \param mmFromCtrLong  mm from center of xtal in longitudinal direction.
    /// \param posDAC positive face signal in CIDAC units for given diode.
    /// \param negDAC negative face signal in CIDAC units for given diode.
    void fill(const CalUtil::AsymType asymType,
              const CalUtil::XtalIdx xtalIdx,
              const float mmFromCtrLong,
              const float posDAC,
              const float negDAC);

    void trimHists() {
      m_asymHists->trimHists();
    }

    size_t getMinEntries() const {
      return m_asymHists->getMinEntries();
    }

  private:
    /// allocate & create asymmetry histograms & pointer arrays
    void        initHists();

    void setDirectory(TDirectory *const dir) {
      m_asymHists->setDirectory(dir);
    }

    /// collection of histograms for light asymmetry
    typedef HistVec<AsymHistId, TH2S> AsymHistCol;

    std::auto_ptr<AsymHistCol> m_asymHists;

    static std::string genHistName(const CalUtil::AsymType asymType,
                                   const CalUtil::XtalIdx xtalIdx);

    /// number of slices along entire xtal
    const unsigned short m_nSlicesPerXtal;
    
    /// number of slices in xtal histogram
    const unsigned short m_nSlicesPerHist;

    /// nominal asymmetry slope in units of log(POSDAC/NEGDAC)/mm from center of xtal
    static float nominalAsymSlope() {
      return .0022;
    }

    /// nominal light asymmetry @ center of crystal for given asymmetry type & cal gain setting
    static float nominalAsymCtr(const CalUtil::AsymType asymType,
                                const CalResponse::CAL_GAIN_INTENT calGain) {
      // same-diode asymmetry is always very close to 0 @ center of xtal
      if (asymType==CalUtil::ASYM_LL || asymType == CalUtil::ASYM_SS)
        return 0;

      // mixed-diode asymmetry contains gain ratio between big/small diode
      float meanMixedAsym = (calGain == CalResponse::FLIGHT_GAIN) ? 4.0 : 1.7;

      // sign of asymmetry depends on large/small or small/large fraction
      if (asymType == CalUtil::ASYM_SL)
        meanMixedAsym *= -1;

      return meanMixedAsym;
    }

    const float m_mmCoveredByHists;
    const float m_mmIgnoredOnXtalEnd;


    /// all new (& modified) histograms written to this directory
    TDirectory *const m_writeDir;

    const CalResponse::CAL_GAIN_INTENT m_calGain;
  };

}; // namespace calibGenCAL
#endif // AsymHists_h
