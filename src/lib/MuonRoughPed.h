#ifndef MuonRoughPed_h
#define MuonRoughPed_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/MuonRoughPed.h,v 1.2 2006/06/22 21:50:23 fewtrell Exp $
/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "CGCUtil.h"
#include "RoughPed.h"

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

/** \brief Represents GLAST Cal LEX8 pedestals calibrations.  'rough' means
    that non-pedestal hits have not yet been cut.

    contains read & write methods to various file formats & code
    to calculate calibrations from digi ROOT event files
    @author Zachary Fewtrell
*/
class MuonRoughPed {
 public:
  MuonRoughPed(ostream &ostrm = cout);
  /// load histograms from ROOT file
  void loadHists(const string &filename);

  void fillHists(unsigned nEntries, 
                 const vector<string> &rootFileList,
                 bool periodicTrigger) ;


  /// Fit roughpedhist[]'s, assign means to m_calRoughPed
  void fitHists(RoughPed &roughPeds); 
  /// construct ouptut filename for given path,extension & input filename
  static void genOutputFilename(const string outputDir,
                                const string &inFilename,
                                const string &ext,
                                string &outFilename) {
    CGCUtil::genOutputFilename(outputDir,
                               "roughPeds",
                               inFilename,
                               ext,
                               outFilename);
  }


 private:

  /// allocate & create rough pedestal histograms & pointer array
  void initHists(); 

  /// count min number of entries in all enable histograms
  unsigned getMinEntries();

  /// list of histograms for 'rough' pedestals
  CalVec<FaceIdx, TH1S*> m_histograms; 

  /// log output to here.
  ostream &m_ostrm;

  /// generate histogram name string
  static string genHistName(FaceIdx faceIdx);
};

#endif
