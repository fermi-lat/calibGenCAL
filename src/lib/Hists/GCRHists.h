#ifndef GCRHists_h
#define GCRHists_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Hists/GCRHists.h,v 1.1 2007/03/27 18:50:50 fewtrell Exp $

/** @file
    @author Zachary Fewtrell
 */

// LOCAL INCLUDES
#include "../Util/CGCUtil.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalVec.h"
#include "CalUtil/CalArray.h"

// EXTLIB INCLUDES

// STD INCLUDES
#include <map>
#include <set>

class TH1S;
class TProfile;
class CalMPD;

/** \brief Represents GLAST Cal Optical gain calibration constants
    (MeV <-> CIDAC)

    contains read & write methods to various file formats 

    @author Zachary Fewtrell
 */
class GCRHists {
public:
  GCRHists(bool summaryMode);

  /// skip event processing & load histograms from previous analysis
  void     loadHists(const string &filename);

  /// print histogram summary info to output stream
  void     summarizeHists(ostream &ostrm) const;

  /// delete all internal histograms w/ 0 entries
  void     trimHists();

  /// count min number of entries in all enabled histograms
  unsigned getMinEntries() const;

  /// fit histograms & save means
  /// \param calMPD write fitted mevPerDAC to calMPD
  /// \param fit a peak to each particle w/ given Zs
  void     fitHists(CalMPD &calMPD, const std::set<unsigned short> &zList);

  /// fill all associated histograms w/ valid ADC hit for given channel
  void fillADCHit(CalUtil::RngIdx rngIdx, float adc);

  /// fill all associated histograms w/ valid CIDAC hit for given channel
  void fillMeanCIDAC(CalUtil::DiodeNum diode, CalUtil::XtalIdx xtalIdx, float cidac);

  /// fill all associated histograms w/ ratio of signal between given adc range & next higher range
  void fillAdcRatio(CalUtil::RngIdx rngIdx, 
				    float thisADC,
				    float nextADC);

  /// fill all associated histograms w/ ratio of le & he cidac signals
  void fillDacRatio(CalUtil::FaceIdx faceIdx,
				    float leDAC,
				    float heDAC);
  
  /// allocate & create mpdmetry histograms & pointer arrays
  /// \note you should cal this if you don't call loadHists() from file
  void     initHists();

private:
  

  /// list of histograms of geometric mean for both ends on each xtal.
  CalUtil::CalVec<CalUtil::DiodeNum,
                  CalUtil::CalArray<CalUtil::XtalIdx, TH1S *> > m_meanDACHists;

  /// generate name for particular histogram
  std::string genMeanDACHistName(CalUtil::DiodeNum diode,
                                 CalUtil::XtalIdx xtalIdx) const;

  /// list of histograms of geometric mean for both ends on each xtal.
  CalUtil::CalVec<CalUtil::RngIdx, TH1S *> m_adcHists;
  /// generate name for particular histogram
  std::string genADCHistName(CalUtil::RngIdx) const;

  /// ratio between mean LE & HE CIDAC per xtal
  CalUtil::CalVec<CalUtil::FaceIdx, TProfile *> m_dacRatioProfs;
  /// generate name for particular histogram
  std::string genDACRatioProfName(CalUtil::FaceIdx faceIdx) const;

  /// ratio between 2 adjacent ADC ranges (mean of both faces) per xtal
  /// \note outermost index from 0 -> 2 by lower of 2 compared ranges
  /// (i.e. index 0 is rng 0 vs rng 1, index 2 is rng 2 vs rng 3)
  CalUtil::CalVec<CalUtil::RngNum,
                  CalUtil::CalArray<CalUtil::FaceIdx, TProfile *> > m_adcRatioProfs;
  /// generate name for particular profogram
  std::string genADCRatioProfName(CalUtil::RngNum rng,
                                  CalUtil::FaceIdx faceIdx) const;

  /// sum over all xtals
  CalUtil::CalArray<CalUtil::DiodeNum, TH1S *> m_meanDACSumHist;
  /// generate name for particular histogram
  std::string genMeanDACSumHistName(CalUtil::DiodeNum diode) const;

  /// sum over all xtals
  CalUtil::CalArray<CalUtil::RngNum, TH1S *>   m_meanADCSumHist;
  /// generate name for particular histogram
  std::string genMeanADCSumHistName(CalUtil::RngNum rng) const;

  /// sum over all xtals
  TProfile *m_dacRatioSumProf;

  /// generate name for particular profogram
  std::string genDACRatioSumProfName() const;

  /// sum over all xtals
  CalUtil::CalVec<CalUtil::RngNum, TProfile *> m_adcRatioSumProf;
  /// generate name for particular profogram
  std::string genADCRatioSumProfName(CalUtil::RngNum rng) const;
  
  /// generate summary histograms only (no individual channels)
  bool  summaryMode;

};

#endif
