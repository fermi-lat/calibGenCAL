#ifndef RoughPed_h
#define RoughPed_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/RoughPed.h,v 1.1 2006/06/15 20:58:00 fewtrell Exp $
/** @file
    @author Zachary Fewtrell
 */



// LOCAL INCLUDES
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

/** \brief Represents GLAST Cal LEX8 pedestals calibrations.  'rough' means
    that non-pedestal hits have not yet been cut.

    contains read & write methods to various file formats & code
    to calculate calibrations from digi ROOT event files
    @author Zachary Fewtrell
*/
class RoughPed {
 public:
  RoughPed(ostream &ostrm = cout);
 
  /// Fill roughpedhist histograms w/ nEvt event data
  /// \param rootFilename.  input digi event file
  /// \param histFilename.  output root file for histograms.
  void fillHists(unsigned nEntries, 
                 const vector<string> &rootFileList,
                 bool periodicTrigger); 

  /// Fit roughpedhist[]'s, assign means to m_calRoughPed
  void fitHists(); 

  /// write rough LEX8 pedestals to simple columnar .txt file
  void writeTXT(const string &filename) const; 

  /// read in rough peds from txt file
  void readTXT(const string &filename);
  
  /// load histograms from ROOT file
  void loadHists(const string &filename);

  /// retrieve pedestal for specific channel
  float getPed(FaceIdx faceIdx) const {return m_peds[faceIdx];}
  /// retrieve pedestal sigma for specific channel
  float getPedSig(FaceIdx faceIdx) const {return m_pedSig[faceIdx];}

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

  /// 'rough' first pass cal pedestals, all signals used (i.e. 'hits' and 'misses'), indexed by FaceIdx()
  CalVec<FaceIdx, float> m_peds; 
  /// corresponding err values for m_calRoughPed
  CalVec<FaceIdx, float> m_pedSig; 

  /// log output to here.
  ostream &m_ostrm;

  /// indicate unpopulated field
  static const short INVALID_PED = -5000;

  /// generate histogram name string
  static string genHistName(FaceIdx faceIdx);
};

#endif
