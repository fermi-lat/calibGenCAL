#ifndef TrigHists_h
#define TrigHists_h

// $Header: //

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalVec.h"

// EXTLIB INCLUDES

// STD INCLUDES
#include <string>
#include <ostream>

class TH1S;
class TDirectory;

namespace CalUtil {
  class CalTrig;
}

namespace calibGenCAL {

  /** \brief Store histograms required to generate Calorimeter Light Trigmetry
      calibrations

      @author Zachary Fewtrell

  */
  class TrigHists {
  public:
    
    /// index various classes of histograms
    typedef enum {
      SPECTRUM_HIST, ///< spectrum where all cuts pass, but trigger bit can be on or off
      TRIGGER_HIST, ///< all cuts pass AND trigger bit is on
      N_HIST_TYPES
    } HistType;

    /// return string repr of enum
    static std::string enum_str(const HistType histType);

    /// allocate & create trigger histograms & pointer arrays
    /// \note you should cal this if you don't call loadHists() from file
    void        initHists(const unsigned short nBins,
                          const float maxMeV);

    /// load histograms from ROOT output of previous run
    void        loadHists(const TDirectory &readDir);

    /// print histogram summary info to output stream
    void        summarizeHists(std::ostream &ostrm) const;

    /// delete empty histograms
    /// \note useful for data w/ < 16 Cal modules.
    void        trimHists();

    /// count min number of entries in all enable histograms
    unsigned    getMinEntries() const;

    /// get histogram of hits where trigger bit was set (numerator)
    TH1S *getTrigHist(const CalUtil::FaceIdx faceIdx) {
      return m_trigHists[faceIdx];
    }

    /// get histogram of hits regardless of whether trigger bit was set (denominator)
    TH1S *getSpecHist(const CalUtil::FaceIdx faceIdx) {
      return m_specHists[faceIdx];
    }

  private:
    static std::string genHistName(const HistType histType,
                                   const CalUtil::FaceIdx faceIdx);


    /// histogram hits where trigger bit was set (numerator)
    CalUtil::CalVec<CalUtil::FaceIdx, TH1S *> m_trigHists;


    /// histogram hits regardless of whether trigger bit was set (denominator)
    CalUtil::CalVec<CalUtil::FaceIdx, TH1S *> m_specHists;

  };

}; // namespace calibGenCAL
#endif // TrigHists_h
