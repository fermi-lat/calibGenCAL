#ifndef AsymHists_h
#define AsymHists_h

// $Header: //

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "../CalibDataTypes/CalAsym.h"

// GLAST INCLUDES
#include "CalUtil/CalVec.h"
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalArray.h"

// EXTLIB INCLUDES

// STD INCLUDES
#include <string>
#include <ostream>

class TH2S;
class CalAsym;

/** \brief Store histograms required to generate Calorimeter Light Asymmetry
    calibrations

    @author Zachary Fewtrell

*/
class AsymHists {
 public:
  AsymHists::AsymHists() :
    m_histograms(CalUtil::AsymType::N_VALS)
    {
    }

  /// allocate & create asymmetry histograms & pointer arrays
  /// \note you should cal this if you don't call loadHists() from file
  void        initHists();

  /// load histograms from ROOT output of previous run
  void        loadHists(const TFile &histFile);

  /// print histogram summary info to output stream
  void        summarizeHists(std::ostream &ostrm);

  /// delete empty histograms
  /// \note useful for data w/ < 16 Cal modules.
  void        trimHists();

  /// count min number of entries in all enable histograms
  unsigned    getMinEntries();

  void        fitHists(CalAsym &calAsym);

  TH2S *getHist(CalUtil::AsymType asymType,
                CalUtil::XtalIdx xtalIdx) {
    return m_histograms[asymType][xtalIdx];
  }

 private:
  /// list of histograms for muon asymmetry
  CalUtil::CalVec<CalUtil::AsymType, CalUtil::CalArray<CalUtil::XtalIdx, TH2S *> > m_histograms;

  std::string genHistName(CalUtil::AsymType asymType,
                          CalUtil::XtalIdx xtalIdx);
};

#endif // AsymHists_h
