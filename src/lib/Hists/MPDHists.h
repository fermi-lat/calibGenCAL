#ifndef MPDHists_h
#define MPDHists_h

// $Header: //

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalVec.h"

// EXTLIB INCLUDES
#include "TH1S.h"
#include "TProfile.h"
#include "CalUtil/SimpleCalCalib/CalMPD.h"

// STD INCLUDES
#include <string>
#include <sstream>

class TProfile;
class TH1I;
class TH1S;

namespace calibGenCAL {


  /** \brief Store histograms required to generate Calorimeter MevPerDAC calibration
      calibrations

      @author Zachary Fewtrell

  */
  class MPDHists {
  public:

    /// collection of possible fitting methods
    struct FitMethods {
      enum FitMethod {
        LANDAU,
        LANGAU
      };

      /// 'LANDAU' use this string for simple ROOT landau fit
      static const std::string LANDAU_STR;
      /// 'LANGAU' use this string for gaussian convolved landau w/
      /// step function background
      static const std::string LANGAU_STR;

      static       std::string &str2enum();
    };

    /// \param parentDir where to load / store histograms for this obj.
    MPDHists(const FitMethods::FitMethod fitMethod);

    /// fit histograms & save mean gain values to calMPD
    /// \param calMPD output calibration values
    /// \param fitMethod should be member of MPDHists::fitMethods
	void        fitHists(CalUtil::CalMPD &calMPD);

    /// delete empty histograms
    /// \note useful for data w/ < 16 Cal modules.
    void        trimHists();

    /// skip event processing & load histograms from previous analysis
    void        loadHists();

    /// allocate & create mpd histograms & pointer arrays
    /// \note you should cal this if you don't call loadHists() from file
    void        initHists();

    /// # of bins in dacL2S profiles
    static const unsigned short N_L2S_PTS     = 20;
    /// min LEDAC val for L2S fitting
    static const unsigned short L2S_MIN_LEDAC = 10;
    /// max LEDAC val for L2S fitting
    static const unsigned short L2S_MAX_LEDAC = 60;

    /// count min number of entries in all enable histograms
    unsigned    getMinEntries() const;

    template <typename T>
    static std::string genHistName(const std::string &type,
                            const T& idx) {
      std::ostringstream tmp;
      
      tmp <<  type
          << "_" << idx;
      return tmp.str();
    }

    /// most probable energy deposition of Muon passing vertically
    /// through a GLAST Cal CsI crystal
    static const float MUON_ENERGY;

    void fillDacLL(CalUtil::XtalIdx xtalIdx,
                   float dac);

    void fillL2S(CalUtil::XtalIdx xtalIdx,
                 float dacL,
                 float dacS) {
      m_dacL2SHists[xtalIdx]->Fill(dacS/dacL);
      // load dacL2S profile
      m_dacL2SSlopeProfs[xtalIdx]->Fill(dacL, dacS);
    }

    /// build tuple with fit outputs
    void buildTuple();

  private:
    /// fit single channel w/ specified function & store
    /// mpv and width
    /// \param mpv location to store fitted most-probable-value
    /// \param width location to store fitted peak width
    void fitChannel(TH1 &hist,
                    float &mpv,
                    float &width);

    /// profile X=bigdiodedac Y=smdiodedac 1 per xtal
    CalUtil::CalVec<CalUtil::XtalIdx, TH1S *>     m_dacL2SHists;

    /// profile X=bigdiodedac Y=smdiodedac 1 per xtal (not used in main calib,
    /// only for finding extra l2s slope info)
    CalUtil::CalVec<CalUtil::XtalIdx, TProfile *> m_dacL2SSlopeProfs;

    /// list of histograms of geometric mean for both ends on each xtal.
    CalUtil::CalVec<CalUtil::XtalIdx, TH1S *>     m_dacLLHists;

    /// contains sum of dacLLHists over all xtals
    TH1I                                         *m_dacLLSumHist;

    /// histograms of MPD fills per Cal layer
    TH1I *m_perLyr;

    /// histograms of MPD fills per Cal tower
    TH1I *m_perTwr;

    /// historams of MPD fills per Cal Xtal
    TH1S *m_perXtal;

    /// histograms of MPD fills per Cal tower & layer
    CalUtil::CalVec<CalUtil::TwrNum, TH1I * > m_perTwrLyr;

    /// histograms of MPD fills per Cal tower & column
    CalUtil::CalVec<CalUtil::TwrNum, TH1I * > m_perTwrCol;

    /// store current fitting method
    const FitMethods::FitMethod m_fitMethod;

    /// store current fitting function
    TF1 *m_fitFunc;
  };

}; // namespace calibGenCAL
#endif // MPDHists_h
