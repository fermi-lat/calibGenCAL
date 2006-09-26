#ifndef MuonAsym_h
#define MuonAsym_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/MuonAsym.h,v 1.3 2006/09/15 15:02:10 fewtrell Exp $
/** @file
    @author Zachary Fewtrell
*/


// LOCAL INCLUDES
#include "CGCUtil.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalVec.h"
#include "CalUtil/CalArray.h"


// EXTLIB INCLUDES

// STD INCLUDES

class CalAsym;
class CalPed;
class CIDAC2ADC;
class TwrHodoscope;
class TH2S;

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
  CalUtil::CalVec<CalUtil::AsymType, CalUtil::CalArray<CalUtil::XtalIdx, TH2S*> > m_histograms; 

  std::string genHistName(CalUtil::AsymType asymType, CalUtil::XtalIdx xtalIdx);

  ostream &m_ostrm;
};

#endif
