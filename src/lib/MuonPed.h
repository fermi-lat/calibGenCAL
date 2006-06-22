#ifndef MuonPed_h
#define MuonPed_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/MuonPed.h,v 1.1 2006/06/15 20:58:00 fewtrell Exp $
/** @file
    @author Zachary Fewtrell
 */



// LOCAL INCLUDES
#include "RoughPed.h"
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
  void fitHists(); 

  /// write muon LEX8 pedestals to simple columnar .txt file
  void writeTXT(const string &filename) const; 

  void readTXT(const string &filename);
  
  /// skip evenmt processing and load histograms from previous run
  void loadHists(const string &filename);

  float getPed(RngIdx rngIdx) const {return m_peds[rngIdx];}
  float getPedSig(RngIdx rngIdx) const {return m_pedSig[rngIdx];}

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

  /// output pedestal data.
  CalVec<RngIdx, float> m_peds; 
  /// corresponding err values for m_calMuonPed
  CalVec<RngIdx, float> m_pedSig; 

  ostream &m_ostrm;

  static const short INVALID_PED = -5000;

  static string genHistName(RngIdx rngIdx);
};

#endif
