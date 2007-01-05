#ifndef MuonCalibTkr_h
#define MuonCalibTkr_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/MuonCalibTkr.h,v 1.1 2007/01/04 23:23:01 fewtrell Exp $

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "CGCUtil.h"
#include "CalGeom.h"
#include "CalHodoscope.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalVec.h"
#include "CalUtil/CalArray.h"

// EXTLIB INCLUDES

// STD INCLUDES
#include <iostream>
#include <map>

class CalPed;
class CIDAC2ADC;
class CalAsym;
class CalMPD;
class CalDigi;
class DigiEvent;
class SimpleIniFile;
class RootFileAnalysis;
class AsymHists;
class MPDHists;

/** \brief Algorithm class generates CalMPD calibration data from digi ROOT
    event files

    @author Zachary Fewtrell
*/
class MuonCalibTkr {
 public:
  MuonCalibTkr(const SimpleIniFile &cfg,
               const CalPed &ped,
               const CIDAC2ADC &dac2adc,
               AsymHists &asymHists,
               MPDHists &mpdHists);

  /// populate histograms from digi root event file
  void        fillHists(unsigned nEntries,
                        const std::vector<std::string> &digiFileList,
                        const std::vector<std::string> &svacFileList);

 private:
  /// process a single event for histogram fill
  bool           processEvent(const DigiEvent &digiEvent);

  /// extract Tracker Track from SVAC & collect
  /// list of potentially valid hit-crystals
  /// \return false if event fails cut during processing
  bool           processTrk();

  /// process CalDigi half of event
  /// \return false if event fails cut during processing
  bool           processDigiEvent(const DigiEvent &digiEvent);

  /// process list of hits selected by tracker tracks
  /// against info from digi in CalHodoscope class
  bool           processFinalHitList();

  /// process a single xtal hit & fill apporiate histograms.
  /// use info from CalHodoscope as well as tracker track
  /// \return false if event fails cut during processing
  bool           processHit(CalUtil::XtalIdx xtalIdx);

  /// quick / high level event cut
  bool           eventCut();

  /// cut after digi hodoscope has been filled
  bool           hodoCut(const CalHodoscope &hscope);

  /// cut based on Tracker track info
  bool           trkCut();

  /// cut based on each cal layer
  bool           lyrCut(const CalHodoscope &hscope);

  /// cut on each cal xtal
  bool           hitCut();

  /// read in cfg parameters
  void           readCfg(const SimpleIniFile &cfg);

  /// enable ROOT TTree branches & assign data destinations
  void           cfgBranches(RootFileAnalysis &rootFile);

  /// extrapolate from start point along dir to
  /// given Z coordinate.  return resulting
  /// 3D LAT coordinate.
  CalGeom::Vec3D trkToZ(const CalGeom::Vec3D &start,
                        const CalGeom::Vec3D &dir,
                        float z);

  /// cut if track enters or exits the xtal too close to xtal edges
  bool           validXtalIntersect(CalUtil::DirNum dir,
                                    CalGeom::Vec3D xtalCtr,
                                    CalGeom::Vec3D intersect);

  /// asymmetry historgrams
  AsymHists &m_asymHists;

  MPDHists &m_mpdHists;

  class AlgData {
  private:
    void init() {
      minDeltaEventTime  = 0;
      xtalLongCut        = 0;
      xtalOrthCut        = 0;
      maxThetaRads       = 0;
      maxHitsPerLyr      = 0;
      maxNHits           = 0;

      nTotalEvents       = 0;
      passTrigWord       = 0;
      passTkrNumTracks   = 0;
      passDeltaEventTime = 0;
      passHitCount       = 0;
      passTheta          = 0;
      passXtalTrk        = 0;
      passXtalClip       = 0;
      passXtalEdge       = 0;
      passXtalMulti      = 0;

      asymFills          = 0;
      mpdLrgFills        = 0;
      mpdSmFills         = 0;
    }

  public:
    AlgData() {
      init();
    }

    /// total number of events read
    unsigned nTotalEvents;

    /// n events pass trigger word cut
    unsigned passTrigWord;

    /// n events pass tkr num tracks cut
    unsigned passTkrNumTracks;

    /// n events pass delta event time cut
    unsigned passDeltaEventTime;

    /// total n xtal hits
    unsigned passHitCount;

    /// total Tkr Tracks w/ good theta ange
    unsigned passTheta;

    /// track extrapolates to valid xtal inside of cal geom
    unsigned passXtalTrk;

    /// track passes through single xtal in Cal lyr
    unsigned passXtalClip;

    /// track avoids XY-'edges' of xtal
    unsigned passXtalEdge;

    /// only 1 cal digi per lyr is used
    unsigned passXtalMulti;

    /// total n asymmetry histogram fills
    unsigned asymFills;

    /// total n mevPerDAC histogram fills (LRG_DIODE)
    unsigned mpdLrgFills;

    /// total n mevPerDAC histogram fills (SM_DIODE)
    unsigned mpdSmFills;

    /// event cut value for time between events
    unsigned minDeltaEventTime;
    /// hit cut value, mm from end of xtal
    float    xtalLongCut;
    /// hit cut value, mm from edge of xtal
    float    xtalOrthCut;
    /// track cut value, max radians from vertical
    float    maxThetaRads;
    /// event cut value
    unsigned short maxHitsPerLyr;
    /// event cut value, max Cal hits per event
    unsigned short maxNHits;

    void printStatus(std::ostream &ostrm);
  } algData;

  class EventData {
  private:
    /// reset all member variables
    void init() {
      eventNum = 0;
      next();
    }

  public:
    EventData(const CalPed &ped,
              const CIDAC2ADC &dac2adc) :
      hscope(ped, dac2adc)
      {
        init();
      }

    /// rest all member variables that do not retain data
    /// from one event to next.
    void next();

    /// index into event files
    unsigned eventNum;

    /// eventID as reported by svac tree
    unsigned svacEventID;
    /// runID as reported by svac tree
    unsigned svacRunID;

    /// from svac
    int      tkrNumTracks;

    /// from svac
    float    tkr1EndPos[3];

    /// from svac
    float    tkr1EndDir[3];

    /// from svac
    unsigned gemConditionsWord;

    /// from svac
    unsigned gemDeltaEventTime;

    /// store set of valid hit xtals derived from tracker track
    std::map<CalUtil::XtalIdx, CalGeom::Vec3D> trkHitMap;

    /// store hodoscopic summary of all event hits.
    CalHodoscope hscope;

    /// theta angle (from vertical) of Tracker track
    float theta;
    
  } eventData;
};

#endif
