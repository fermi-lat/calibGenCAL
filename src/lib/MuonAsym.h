#ifndef MuonAsym_h
#define MuonAsym_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/MuonAsym.h,v 1.2 2006/06/22 21:50:22 fewtrell Exp $
/** @file
    @author Zachary Fewtrell
 */


// LOCAL INCLUDES
#include "CalPed.h"
#include "CalAsym.h"
#include "CIDAC2ADC.h"
#include "CGCUtil.h"
#include "TwrHodoscope.h"

// GLAST INCLUDES
#include "CalUtil/CalVec.h"
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalArray.h"


// EXTLIB INCLUDES
#include "TFile.h"
#include "TH2S.h"
#include "TSpline.h"

// STD INCLUDES
#include <string>
#include <memory>

using namespace std;
using namespace CalUtil;

/** \brief Reresents GLAST Cal crystal light asymmetry calibration constants.

    contains read & write methods to various file formats & code
    to calculate calibrations from digi ROOT event files

    @author Zachary Fewtrell
*/
class MuonAsym {
 public:
  MuonAsym(ostream &ostrm = cout);

  /// populate asymmetry profiles w/ nEvt worth of data.
  void fillHists(unsigned nEntries,
                 const vector<string> &rootFileList, 
                 const CalPed &peds,
                 const CIDAC2ADC &dac2adc); 

  /// load mean values from asymmetry profiles into m_asym*** arrays
  void fitHists(CalAsym &calAsym); 

  /// load histograms from ROOT output of previous run
  void loadHists(const string &filename);

  /// print histogram summary info to output stream
  void summarizeHists(ostream &ostrm=cout);

  static void genOutputFilename(const string outputDir,
                                const string &inFilename,
                                const string &ext,
                                string &outFilename) {
    CGCUtil::genOutputFilename(outputDir,
                               "muonAsym",
                               inFilename,
                               ext,
                               outFilename);
  }


 private:
  bool passCutX(const TwrHodoscope &hscope);
  bool passCutY(const TwrHodoscope &hscope);

  /// allocate & create asymmetry histograms & pointer arrays
  void initHists();

  /// count min number of entries in all enable histograms
  unsigned getMinEntries();
  /// list of histograms for muon asymmetry
  CalVec<AsymType, CalArray<XtalIdx, TH2S*> > m_histograms; 

  string genHistName(AsymType asymType, XtalIdx xtalIdx);

  ostream &m_ostrm;
};

#endif
