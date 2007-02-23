#ifndef GCRHists_h
#define GCRHists_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/GCRHists.h,v 1.3 2006/09/15 15:02:10 fewtrell Exp $
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
#include <map>

class SimpleIniFile;
class CalPed;
class CalMPD;
class CIDAC2ADC;
class DigiEvent;
class GcrSelectEvent;
class GcrSelectedXtal;
class CalDigi;
class TVector3;
class TH1S;
class TProfile;

/** \brief Represents GLAST Cal Optical gain calibration constants
    (MeV <-> CIDAC)

    contains read & write methods to various file formats & code
    to calculate calibrations from digi ROOT event files

    @author Zachary Fewtrell
*/
class GCRHists {
 public:
  GCRHists(const SimpleIniFile *cfgFile=NULL,
           bool summaryMode=false
           );

  /// load parameters from cfg file.
  void readCfg(const SimpleIniFile &cfgFile);

  /// populate histograms from digi root event file
  void fillHists(unsigned nEntries,
                 const std::vector<std::string> &digiRootFileList, 
                 const std::vector<std::string> &gcrSelectRootFileList, 
                 const CalPed &peds,
                 const CIDAC2ADC &dac2adc); 

  /// fit histograms & save means
  void fitHists(CalMPD &calMPD); 

  /// skip event processing & load histograms from previous analysis
  void loadHists(const string &filename);

  /// print histogram summary info to output stream
  void summarizeHists(ostream &ostrm) const;

  /// delete all internal histograms w/ 0 entries
  void trimHists();

 private:
  /// allocate & create mpdmetry histograms & pointer arrays
  void initHists();

  /// perform cuts based on GCRSelect info
  /// fill list of xtals for digi processing
  void processGcrEvent();

  /// process single GCRSelect hit
  void processGcrHit(const GcrSelectedXtal &gcrXtal);

  /// fill histograms w/ digi info from hit selected
  /// by process GCR event
  void processDigiEvent();

  /// process single CalDigi hit
  void processDigiHit(const CalDigi &calDigi);

  /// cut xtal hit based on track entry & exit face.
  /// return true for good hit.
  bool crossedFaceCut(unsigned crossedFaces) const;

  /// cut hit based on incident track angle
  /// \note use orthogonalAngleCut && longitudinalAngleCut
  /// variables.
  bool angleCut(const TVector3 &pathVec,
                CalUtil::DirNum dir) const;

  /// cut hit based on hit distancefrom xtal face
  /// \note use xtalEndCut variable
  bool posCut(CalUtil::XtalIdx xtalIdx,
              const TVector3 &entry,
              const TVector3 &exit) const;


  /// count min number of entries in all enabled histograms
  unsigned getMinEntries() const;

  /// list of histograms of geometric mean for both ends on each xtal.
  CalUtil::CalVec<CalUtil::DiodeNum, 
    CalUtil::CalArray<CalUtil::XtalIdx, TH1S*> > m_meanDACHists; 
  /// generate name for particular histogram
  std::string genMeanDACHistName(CalUtil::DiodeNum diode, 
                                 CalUtil::XtalIdx xtalIdx) const;
  
  /// list of histograms of geometric mean for both ends on each xtal.
  CalUtil::CalVec<CalUtil::RngNum, 
    CalUtil::CalArray<CalUtil::XtalIdx, TH1S*> > m_meanADCHists; 
  /// generate name for particular histogram
  std::string genMeanADCHistName(CalUtil::RngNum rng, 
                                 CalUtil::XtalIdx xtalIdx) const;
  
  /// ratio between mean LE & HE CIDAC per xtal
  CalUtil::CalVec<CalUtil::XtalIdx, TProfile*> m_dacRatioProfs; 
  /// generate name for particular histogram
  std::string genDACRatioProfName(CalUtil::XtalIdx xtalIdx) const;
  
  /// ratio between 2 adjacent ADC ranges (mean of both faces) per xtal
  /// \note outermost index from 0 -> 2 by lower of 2 compared ranges 
  /// (i.e. index 0 is rng 0 vs rng 1, index 2 is rng 2 vs rng 3)
  CalUtil::CalVec<CalUtil::RngNum,
    CalUtil::CalArray<CalUtil::XtalIdx, TProfile*> > m_adcRatioProfs; 
  /// generate name for particular profogram
  std::string genADCRatioProfName(CalUtil::RngNum rng,
                                  CalUtil::XtalIdx xtalIdx) const;
  
  /// sum over all xtals
  CalUtil::CalArray<CalUtil::DiodeNum, TH1S*> m_meanDACSumHist;
  /// generate name for particular histogram
  std::string genMeanDACSumHistName(CalUtil::DiodeNum diode) const;

  /// sum over all xtals
  CalUtil::CalArray<CalUtil::RngNum, TH1S*> m_meanADCSumHist;
  /// generate name for particular histogram
  std::string genMeanADCSumHistName(CalUtil::RngNum rng) const;

  /// sum over all xtals
  TProfile* m_dacRatioSumProf;
  /// generate name for particular profogram
  std::string genDACRatioSumProfName() const;

  /// sum over all xtals
  CalUtil::CalVec<CalUtil::RngNum, TProfile*> m_adcRatioSumProf;
  /// generate name for particular profogram
  std::string genADCRatioSumProfName(CalUtil::RngNum rng) const;
  

  /// store cfg & status data pertinent to current algorithm run
  struct AlgData {
    AlgData() {clear();}

    /// reset all member values
    void clear() {
      nEventsAttempted = 0;
      nEventsRead      = 0;
      nGcrHits         = 0;
      nHitsXface       = 0;
      nHitsAngle       = 0;
      nHitsPos         = 0;
      nFills.fill(0);
      calPed           = 0;
      dac2adc          = 0;
    }

    void summarizeAlg(ostream &ostrm) const;

    /// number of events attempt to read from root file
    unsigned nEventsAttempted;
    /// number of events sucessfully read from root file
    unsigned nEventsRead;
    /// number of GcrSelectedXtal hits processed
    unsigned nGcrHits;
    /// number of Hits pass the whichFacesCrossed cut
    unsigned nHitsXface;
    /// number of hits pass the incident angle cut
    unsigned nHitsAngle;
    /// number of hits pass the distance xtal pos cut
    unsigned nHitsPos;
    /// number of histogram fills.
    CalUtil::CalArray<CalUtil::DiodeNum, unsigned> nFills;

    /// adc pedestals for use by algorithm
    const CalPed *calPed;

    /// cidac2adc for use by alg
    const CIDAC2ADC *dac2adc;

  } algData;

  /// store data pertinent to current event
  struct EventData {
    EventData() {clear();}

    /// reset all member values
    void clear() {
      eventNum       = 0;
      next();
    }

    /// clear out obj in prep for next event
    /// leave vars that need to persiste from 
    /// event to event
    void next() {
      digiEvent      = 0;
      gcrSelectEvent = 0;
      finalHitMap.clear();
    }

    /// current event index
    unsigned eventNum;

    /// pointer to current digiEvent leaf
    const DigiEvent *digiEvent;

    /// pointer to current gcrEvent leaf
    GcrSelectEvent *gcrSelectEvent;

    /// list of xtals which passed GCR cuts
    typedef std::map<CalUtil::XtalIdx, const GcrSelectedXtal *> FinalHitMap;
    typedef FinalHitMap::iterator FinalHitMapIter;
    FinalHitMap finalHitMap;
  } eventData;

  /// enable hit cut based on path entry & exit face on xtal
  bool cutCrossedFaces;
  /// enable hit cut based on track angle
  bool cutTrackAngle;
  /// enable hit cut based on longitudinal hit position.
  bool cutLongitudinalPos;

  /// sin of max incident angle component orthogonal to xtal length.
  float maxOrthogonalSin;
  /// sin of max incident angle component along xtal length.
  float maxLongitudinalSin;
  /// ignore tracks which come w/ in some distance from xtal end
  float mmCutFromEnd;
  /// directly based on mmCutFromEnd
  float mmCutFromCtr;

  /// generate summary histograms only (no individual channels)
  bool summaryMode;

};

#endif
