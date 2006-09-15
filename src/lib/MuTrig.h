#ifndef MuTrig_h
#define MuTrig_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/MuTrig.h,v 1.1 2006/06/15 20:57:59 fewtrell Exp $
/** @file
    @author fewtrell
 */


// LOCAL INCLUDES
#include "CalPed.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalVec.h"

// EXTLIB INCLUDES
#include "TH1S.h"

// STD INCLUDES

using namespace std;
using namespace CalUtil;

/** \brief Represents GLAST Cal trigger efficiency calibrations

    contains read & write methods to various file formats & code
    to calculate calibrations from digi ROOT event files.

    calculates efficiency for both CIDAC and Muon tests

    @author fewtrell
*/
class MuTrig {
 public:
  /// describes which cal trigger channels are enabled
  /// in this configuration.
  /// \note 'row' refers to the row number on a single 
  /// Cal electronics board (i.e. 1 cal face), so it ranges
  /// 0->3, not 0->7
  typedef enum { EVEN_ROW_EVEN_COL = 0, 
                 EVEN_ROW_ODD_COL = 1} TRIG_CFG;

  /// \param ostrm output logged to here.
  MuTrig(ostream &ostrm = cout);


  /// \brief measure FLE trigger efficiency against muons
  /// \param filename root digi data file w/ diagnostic info
  /// enabled.
  void fillMuonHists(TRIG_CFG trigCfg,
                     const string &filename,
                     unsigned nEvents,
                     const CalPed &peds,
                     bool calLOEnabled=false
                     );
  
  /// \brief measure FLE trigger efficiency against charge injection
  /// \param filename root digi data file w/ diagnostic info
  /// enabled.
  void fillCIHists(const string &filename);

  /// fit muon and CI efficiency data
  void fitData(const CalPed &ped);

  /// write output to txt
  void writeTXT(const string &filename) const;

  /// write intermediate output to txt
  void writeCIMetavals(const string &filename) const;

  
  /// generate 'official' output filename from inputfile, path, and extension
  static void genOutputFilename(const string outputDir,
                                const string &inFilename,
                                const string &ext,
                                string &outFilename) {
    CGCUtil::genOutputFilename(outputDir,
                               "muTrig",
                               inFilename,
                               ext,
                               outFilename);
  }

  
 private:
  void initHists();

  /// determine if xtal is enabled for this 
  /// configuration.
  bool xtalEnabled(TRIG_CFG trigCfg,
                   GCRCNum gcrc,
                   ColNum col) {
    if (trigCfg == EVEN_ROW_EVEN_COL)
      if (gcrc % 2 == col%2)
        return true;
      else return false;
    else
      if (gcrc%2 != col%2)
        return true;
      else return false;
  }

  string genHistName(const string &type,
                     FaceIdx faceIdx);

  /// histograms of all muon hits that pass the cut
  CalVec<FaceIdx, TH1S*> m_muAdcHists;
  /// histograms of all muon hits which triggered fle
  CalVec<FaceIdx, TH1S*> m_muTrigHists;
  /// histogram of ci efficiency (think graph, not histogram)
  CalVec<FaceIdx, TH1S*> m_ciEffHists;
  /// histogram of ci adc mean values (think graph, not histogram)
  CalVec<FaceIdx, TH1S*> m_ciAdcHists;

  /// fle muon thresholds in adc units per channel
  CalVec<FaceIdx, float> m_muThresh;
  /// fle muon threshold widths
  CalVec<FaceIdx, float> m_muThreshWidth;
  /// fle charge-injection threholds in adc units
  CalVec<FaceIdx, float> m_ciThresh;
  /// fle ci thresh width
  CalVec<FaceIdx, float> m_ciThreshWidth;

  CalVec<FaceIdx, float> m_muThreshErr;
  CalVec<FaceIdx, float> m_muThreshWidthErr;
  CalVec<FaceIdx, float> m_ciThreshErr;
  CalVec<FaceIdx, float> m_ciThreshWidthErr;

  /// n triggers per channel for ci
  CalVec<FaceIdx, vector<unsigned short> >  m_ciTrigSum;
  /// n total hits per channel for ci
  CalVec<FaceIdx, vector<unsigned short> >  m_ciAdcN;
  /// sum of all hit-adc values per channel
  CalVec<FaceIdx, vector<unsigned> >    m_ciADCSum;

  /// (CIDAC pedestal - muon pedestal)
  CalVec<FaceIdx, float> m_delPed;

  /// represent unpopulated field
  static const short INVALID_ADC = -5000;

  /// used for internal logging
  ostream &m_ostrm;

};

#endif
