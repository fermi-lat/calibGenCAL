#ifndef GCRCalibAlg_h
#define GCRCalibAlg_h

// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/Optical/GCRCalibAlg.h,v 1.2 2008/05/13 16:53:59 fewtrell Exp $

/** @file
    @author Zach Fewtrell
*/

// LOCAL INCLUDES

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalVec.h"
#include "CalUtil/SimpleCalCalib/CalMPD.h"
#include "CalUtil/SimpleCalCalib/IdealCalCalib.h"

// EXTLIB INCLUDES

// STD INCLUDES
#include <vector>
#include <string>
#include <map>
#include <set>
#include <memory>

class TVector3;
class GcrSelectedXtal;
class GcrSelectEvent;
class CalDigi;
class DigiEvent;

class SimpleIniFile;

namespace CalUtil {
  class CalPed;
  class CIDAC2ADC;
  class CalMPD;
}

namespace calibGenCAL {
  class GCRHists;
  class AsymHists;

  /** \brief Algorithm-type class for generating GLAST Cal optical Calibrations for 
      GCR events
  */


  class GCRCalibAlg {
  public:
    /// \param cfgPath (optional) ini type filename for loading optional parameters.  set to "" for defautls (supports env var expansion)
    /// \param inputMPDTxtPath (optional) path to MeVPerDAC text calibration file.  If != "", enables histograms with energy scale axes.
    GCRCalibAlg(const std::string &cfgPath="",
                const std::string &inputMPDTxtPath="");

    /// populate histograms from digi root event file
    /// \nEvents max # events to loop through
    void fillHists(const unsigned nEventsMax,
                   const std::vector<std::string> &digiFileList,
                   const std::vector<std::string> &gcrSelectRootFileList,
                   const CalUtil::CalPed &peds,
                   const CalUtil::CIDAC2ADC &dac2adc,
                   GCRHists &gcrHists,
                   AsymHists &asymHists);



  private:

    /// load parameters from cfg file.
    void readCfg(std::string cfgPath);

    /// perform cuts based on GCRSelect info
    /// fill list of xtals for digi processing
    void     processGcrEvent();

    /// process single GCRSelect hit
    void     processGcrHit(const GcrSelectedXtal &gcrXtal);

    /// fill histograms w/ digi info from hit selected
    /// by process GCR event
    void     processDigiEvent();

    /// process single CalDigi hit
    void     processDigiHit(const CalDigi &calDigi);

    /// fill asymmetry histograms w/ dac signal info
    void fillAsymHit(const CalUtil::CalVec<CalUtil::XtalDiode, float> &cidac);

    /// cut xtal hit based on track entry & exit face.
    /// return true for good hit.
    bool     crossedFaceCut(const unsigned crossedFaces) const;
    /// cut hit based on incident track angle
    /// \note use orthogonalAngleCut && longitudinalAngleCut
    /// variables.
    bool     angleCut(const TVector3 &pathVec,
                      const CalUtil::DirNum dir) const;

    /// cut hit based on hit distancefrom xtal face
    /// \note use xtalEndCut variable
    bool     posCut(const CalUtil::XtalIdx xtalIdx,
                    const TVector3 &entry,
                    const TVector3 &exit) const;

    /// store cfg & status data pertinent to current algorithm run
    struct AlgData {
      AlgData() {
        clear();
      }

      /// reset all member values
      void clear() {
        nEventsAttempted = 0;
        nEventsRead      = 0;
        nGcrHits         = 0;
        nHitsXface       = 0;
        nHitsAngle       = 0;
        nHitsPos         = 0;
        fill(nFills.begin(), nFills.end(), 0);
        calPed           = 0;
        dac2adc          = 0;
      }

      void summarizeAlg(ostream &ostrm) const;

      /// number of events attempt to read from root file
      unsigned                                       nEventsAttempted;
      /// number of events sucessfully read from root file
      unsigned                                       nEventsRead;
      /// number of GcrSelectedXtal hits processed
      unsigned                                       nGcrHits;
      /// number of Hits pass the whichFacesCrossed cut
      unsigned                                       nHitsXface;
      /// number of hits pass the incident angle cut
      unsigned                                       nHitsAngle;
      /// number of hits pass the distance xtal pos cut
      unsigned                                       nHitsPos;

      /// number of histogram fills.
      CalUtil::CalVec<CalUtil::DiodeNum, unsigned> nFills;

      /// number of asymmetry histogram fills
      CalUtil::CalVec<CalUtil::DiodeNum, unsigned> nAsymFills;

      /// adc pedestals for use by algorithm
      const CalUtil::CalPed *                                 calPed;

      /// cidac2adc for use by alg
      const CalUtil::CIDAC2ADC *                              dac2adc;

      GCRHists *                               gcrHists;
      AsymHists * asymHists;
    }     algData;

    /// store data pertinent to current event
    struct EventData {
      EventData() {
        clear();
      }

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
        inferredZ = 0;
      }

      /// current event index
      unsigned eventNum;

      /// pointer to current digiEvent leaf
      const DigiEvent *digiEvent;

      /// pointer to current gcrEvent leaf
      const GcrSelectEvent * gcrSelectEvent;

      /// list of xtals which passed GCR cuts
      typedef std::map < CalUtil::XtalIdx, 
                         const GcrSelectedXtal * > FinalHitMap;

      typedef FinalHitMap::iterator  FinalHitMapIter;

      FinalHitMap      finalHitMap;

      unsigned short inferredZ;

    } eventData;

    /// enable hit cut based on path entry & exit face on xtal
    bool  cutCrossedFaces;
    /// enable hit cut based on track angle
    bool  cutTrackAngle;
    /// enable hit cut based on longitudinal hit position.
    bool  cutLongitudinalPos;

    /// sin of max incident angle component orthogonal to xtal length.
    float maxOrthogonalSin;
    /// sin of max incident angle component along xtal length.
    float maxLongitudinalSin;
    /// ignore tracks which come w/ in some distance from xtal end
    float mmCutFromEnd;
    /// directly based on mmCutFromEnd
    float mmCutFromCtr;
    /// energy limit for asymmetry histograms
    float minAsymMeV;
    /// energy limit for asymmetry histograms
    float maxAsymMeV;

    /// list of particles 'of interest' for storage in histograms
    std::set<int> zList;

    /// if !="", enables energy scale histogram axes.
    std::auto_ptr<CalUtil::CalMPD> m_inputMPD;

    /// stores ideal (average) calibration values.
    CalUtil::IdealCalCalib m_idealCalib;
  };


}; // namespace calibGenCAL
#endif
