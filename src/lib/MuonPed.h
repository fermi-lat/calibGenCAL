#ifndef MuonPed_h
#define MuonPed_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/MuonPed.h,v 1.2 2006/06/22 21:50:23 fewtrell Exp $
/** @file
    @author Zachary Fewtrell
 */



// LOCAL INCLUDES
#include "RoughPed.h"
#include "CalPed.h"
#include "CGCUtil.h"

// GLAST INCLUDES
#include "CalUtil/CalVec.h"
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalArray.h"

// EXTLIB INCLUDES
#include "TH1S.h"
#include "TFile.h"

// STD INCLUDES


using namespace std;
using namespace CalUtil;

/** \brief \brief Represents GLAST Cal ADC pedestal calibrations

    contains read & write methods to various file formats & code
    to calculate calibrations from digi ROOT event files

    @author Zachary Fewtrell
*/
class MuonPed {
 public:
  MuonPed(ostream &ostrm = cout);
 
  /// Fill muonpedhist histograms w/ nEvt event data
  /// \param rootFilename.  input digi event file
  /// \param histFilename.  output root file for histograms.
  void fillHists(unsigned nEntries, 
                 const vector<string> &rootFileList, 
                 const RoughPed &roughPeds,
                 bool periodicTrigger); 

  /// Fit muonpedhist[]'s, assign means to m_calMuonPed
  void fitHists(CalPed &peds); 

  /// skip evenmt processing and load histograms from previous run
  void loadHists(const string &filename);

  static void genOutputFilename(const string outputDir,
                                const string &inFilename,
                                const string &ext,
                                string &outFilename) {
    CGCUtil::genOutputFilename(outputDir,
                               "muonPeds",
                               inFilename,
                               ext,
                               outFilename);
  }




 private:
  /// allocate & create muon pedestal histograms & pointer array
  void initHists(); 

  /// count min number of entries in all enable histograms
  unsigned getMinEntries();

  /// list of histograms for 'muon' pedestals
  CalVec<RngIdx, TH1S*> m_histograms; 
  ostream &m_ostrm;

  static string genHistName(RngIdx rngIdx);
};

#endif
