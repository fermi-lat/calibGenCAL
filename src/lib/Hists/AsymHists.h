#ifndef AsymHists_h
#define AsymHists_h

// $Header: //

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalVec.h"
#include "CalUtil/SimpleCalCalib/CalAsym.h"

// EXTLIB INCLUDES

// STD INCLUDES
#include <string>
#include <ostream>

class TH2S;
class TDirectory;

namespace CalUtil {

  class CalAsym;
}

namespace calibGenCAL {

  /** \brief Store histograms required to generate Calorimeter Light Asymmetry
      calibrations

      @author Zachary Fewtrell

  */
  class AsymHists {
  public:
    AsymHists::AsymHists()
    {
    }

    /// allocate & create asymmetry histograms & pointer arrays
    /// \note you should cal this if you don't call loadHists() from file
    void        initHists();

    /// load histograms from ROOT output of previous run
    void        loadHists(const TDirectory &readDir);

    /// print histogram summary info to output stream
    void        summarizeHists(std::ostream &ostrm) const;

    /// delete empty histograms
    /// \note useful for data w/ < 16 Cal modules.
    void        trimHists();

    /// count min number of entries in all enable histograms
    unsigned    getMinEntries() const;

    void        fitHists(CalUtil::CalAsym &calAsym);

    TH2S *getHist(const CalUtil::AsymType asymType,
                  const CalUtil::XtalIdx xtalIdx) {
      return m_histograms[asymType][xtalIdx];
    }

  private:
    /// list of histograms for muon asymmetry
    CalUtil::CalVec<CalUtil::AsymType, CalUtil::CalVec<CalUtil::XtalIdx, TH2S *> > m_histograms;

    static std::string genHistName(const CalUtil::AsymType asymType,
                                   const CalUtil::XtalIdx xtalIdx);
  };

}; // namespace calibGenCAL
#endif // AsymHists_h
