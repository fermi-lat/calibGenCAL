#ifndef MuonMPD_h
#define MuonMPD_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/MuonMPD.h,v 1.1 2006/06/15 20:58:00 fewtrell Exp $
/** @file
    @author Zachary Fewtrell
 */


// LOCAL INCLUDES
#include "CIDAC2ADC.h"
#include "MuonPed.h"
#include "MuonAsym.h"
#include "CGCUtil.h"

// GLAST INCLUDES
#include "CalUtil/CalVec.h"
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalArray.h"


// EXTLIB INCLUDES
#include "TFile.h"
#include "TH1S.h"
#include "TSpline.h"


// STD INCLUDES
#include <string>
#include <memory>

using namespace std;
using namespace CalUtil;

/** \brief Represents GLAST Cal Optical gain calibration constants
    (MeV <-> CIDAC)

    contains read & write methods to various file formats & code
    to calculate calibrations from digi ROOT event files

    @author Zachary Fewtrell
*/
class MuonMPD {
 public:
  MuonMPD(ostream &ostrm = cout);

  /// populate histograms from digi root event file
  void fillHists(unsigned nEntries,
                 const vector<string> &rootFileList, 
                 const MuonPed &peds,
                 const MuonAsym &asym,
                 const CIDAC2ADC &dac2adc); 

  /// fit histograms & save means
  void fitHists(); 

  void writeTXT(const string &filename) const;
  
  void readTXT(const string &filename);

  static void genOutputFilename(const string outputDir,
                                const string &inFilename,
                                const string &ext,
                                string &outFilename) {
    CGCUtil::genOutputFilename(outputDir,
                               "muonMPD",
                               inFilename,
                               ext,
                               outFilename);
  }


  /// skip event processing & load histograms from previous analysis
  void loadHists(const string &filename);

  /// write ADC2MEV (4-range) factors to txt file
  void writeADC2NRG(const string &filename,
                    const MuonAsym &asym,
                    const CIDAC2ADC &dac2adc);


 private:
  /// allocate & create mpdmetry histograms & pointer arrays
  void initHists();

  /// count min number of entries in all enable histograms
  unsigned getMinEntries();

  /// 2d vector N_MPD_PTS lograt mpdvals per xtal/mpdType
  CalVec<DiodeNum, CalArray<XtalIdx, float> > m_mpd; 
  /// corresponding error value
  CalVec<DiodeNum, CalArray<XtalIdx, float> > m_mpdErr; 

  string genHistName(const string &type,
                     XtalIdx xtalIdx);

  ostream &m_ostrm;

  /// # of bins in dacL2S profiles
  static const unsigned short N_L2S_PTS = 20;
  /// min LEDAC val for L2S fitting
  static const unsigned short L2S_MIN_LEDAC = 10;
  /// max LEDAC val for L2S fitting
  static const unsigned short L2S_MAX_LEDAC = 60;

  static const short INVALID_MPD = -5000;
  
  /// profile X=bigdiodedac Y=smdiodedac 1 per xtal
  CalVec<XtalIdx, TH1S*> m_dacL2SHists;  

  /// profile X=bigdiodedac Y=smdiodedac 1 per xtal (not used in main calib, only for finding extra l2s slope info)
  CalVec<XtalIdx, TProfile*> m_dacL2SSlopeProfs;
  /// list of histograms of geometric mean for both ends on each xtal.
  CalVec<XtalIdx, TH1S*> m_dacLLHists; 

  /// most likely vertical muon energy deposition in Cal CsI crystal
  static const float MUON_ENERGY;

};

#endif
