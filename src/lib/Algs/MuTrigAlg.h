#ifndef MuTrigAlg_h
#define MuTrigAlg_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Algs/MuTrigAlg.h,v 1.5 2007/10/15 15:17:32 golpa Exp $

/** @file
    @author fewtrell
*/

// LOCAL INCLUDES

// GLAST INCLUDES
#include "CalUtil/CalVec.h"
#include "CalUtil/CalDefs.h"

// EXTLIB INCLUDES
#include "TH1.h"

// STD INCLUDES

class TH1S;

namespace CalUtil {

  class CalPed;
}

namespace calibGenCAL {


  /** \brief Represents GLAST Cal trigger efficiency calibrations

  contains read & write methods to various file formats & code
  to calculate calibrations from digi ROOT event files.

  calculates efficiency for both CIDAC and Muon tests

  @author fewtrell
  */
  class MuTrigAlg {
  public:
    /// describes which cal trigger channels are enabled
    /// in this configuration.
    /// \note 'row' refers to the row number on a single
    /// Cal electronics board (i.e. 1 cal face), so it ranges
    /// 0->3, not 0->7
    typedef enum {
      EVEN_ROW_EVEN_COL = 0,
      EVEN_ROW_ODD_COL  = 1
    } TRIG_CFG;

    /// \param ostrm output logged to here.
    MuTrigAlg();

    /// \brief measure FLE trigger efficiency against muons
    /// \param filename root digi data file w/ diagnostic info
    /// enabled.
    void fillMuonHists(const TRIG_CFG trigCfg,
                       const std::string &filename,
                       unsigned nEvents,
					   const CalUtil::CalPed &peds,
                       const bool calLOEnabled = false
                       );

    /// \brief measure FLE trigger efficiency against charge injection
    /// \param filename root digi data file w/ diagnostic info
    /// enabled.
    void fillCIHists(const std::string &filename);

    /// fit muon and CI efficiency data
    void fitData(const CalUtil::CalPed &ped);

    /// write output to txt
    void writeTXT(const std::string &filename) const;

    /// write intermediate output to txt
    void writeCIMetavals(const std::string &filename) const;

  private:
    void initHists();

    /// determine if xtal is enabled for this
    /// configuration.
    static bool xtalEnabled(const TRIG_CFG trigCfg,
                     const CalUtil::GCRCNum gcrc,
                     const CalUtil::ColNum col) {
      if (trigCfg == EVEN_ROW_EVEN_COL)
        if (gcrc.val() % 2 == col.val()%2)
          return true;
        else return false;
      else
        if (gcrc.val()%2 != col.val()%2)
          return true;
        else return false;
    }

    static std::string genHistName(const std::string &type,
                                   const CalUtil::FaceIdx faceIdx);

    /// histograms of all muon hits that pass the cut
    CalUtil::CalVec<CalUtil::FaceIdx, TH1S *>                   m_muAdcHists;
    /// histograms of all muon hits which triggered fle
    CalUtil::CalVec<CalUtil::FaceIdx, TH1S *>                   m_muTrigHists;
    /// histogram of ci efficiency (think graph, not histogram)
    CalUtil::CalVec<CalUtil::FaceIdx, TH1S *>                   m_ciEffHists;
    /// histogram of ci adc mean values (think graph, not histogram)
    CalUtil::CalVec<CalUtil::FaceIdx, TH1S *>                   m_ciAdcHists;

    /// fle muon thresholds in adc units per channel
    CalUtil::CalVec<CalUtil::FaceIdx, float>                    m_muThresh;
    /// fle muon threshold widths
    CalUtil::CalVec<CalUtil::FaceIdx, float>                    m_muThreshWidth;
    /// fle charge-injection threholds in adc units
    CalUtil::CalVec<CalUtil::FaceIdx, float>                    m_ciThresh;
    /// fle ci thresh width
    CalUtil::CalVec<CalUtil::FaceIdx, float>                    m_ciThreshWidth;

    CalUtil::CalVec<CalUtil::FaceIdx, float>                    m_muThreshErr;
    CalUtil::CalVec<CalUtil::FaceIdx, float>                    m_muThreshWidthErr;
    CalUtil::CalVec<CalUtil::FaceIdx, float>                    m_ciThreshErr;
    CalUtil::CalVec<CalUtil::FaceIdx, float>                    m_ciThreshWidthErr;

    /// n triggers per channel for ci
    CalUtil::CalVec<CalUtil::FaceIdx, vector<unsigned short> >  m_ciTrigSum;
    /// n total hits per channel for ci
    CalUtil::CalVec<CalUtil::FaceIdx, vector<unsigned short> >  m_ciAdcN;
    /// sum of all hit-adc values per channel
    CalUtil::CalVec<CalUtil::FaceIdx, vector<unsigned> >        m_ciADCSum;

    /// (CIDAC pedestal - muon pedestal)
    CalUtil::CalVec<CalUtil::FaceIdx, float>                    m_delPed;

    /// represent unpopulated field
    static const short INVALID_ADC = -5000;
  };

}; // namespace calibGenCAL
#endif
