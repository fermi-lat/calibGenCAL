#ifndef MuonMPD_h
#define MuonMPD_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/MuonMPD.h,v 1.3 2006/09/15 15:02:10 fewtrell Exp $
/** @file
    @author Zachary Fewtrell
 */

// LOCAL INCLUDES
#include "CIDAC2ADC.h"
#include "CalPed.h"
#include "CalAsym.h"
#include "CalMPD.h"
#include "CGCUtil.h"
#include "TwrHodoscope.h"

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
                 const CalPed &peds,
                 const CalAsym &asym,
                 const CIDAC2ADC &dac2adc); 

  /// fit histograms & save means
  void fitHists(CalMPD &calMPD); 
  /// skip event processing & load histograms from previous analysis
  void loadHists(const string &filename);

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

  static void writeADC2NRG(const string &filename,
                                    const CalAsym &asym,
                                    const CIDAC2ADC &dac2adc,
                                    const CalMPD &mpd);

 private:
  bool passCutX(const TwrHodoscope &hscope);
  bool passCutY(const TwrHodoscope &hscope);

  /// allocate & create mpdmetry histograms & pointer arrays
  void initHists();

  /// count min number of entries in all enable histograms
  unsigned getMinEntries();

  string genHistName(const string &type,
                     XtalIdx xtalIdx);

  ostream &m_ostrm;

  /// # of bins in dacL2S profiles
  static const unsigned short N_L2S_PTS = 20;
  /// min LEDAC val for L2S fitting
  static const unsigned short L2S_MIN_LEDAC = 10;
  /// max LEDAC val for L2S fitting
  static const unsigned short L2S_MAX_LEDAC = 60;
  
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
