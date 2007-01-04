// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/MuonCalibTkr.cxx,v 1.13 2006/10/19 17:57:57 fewtrell Exp $

/** @file
    @author Zachary Fewtrell
 */

// LOCAL INCLUDES
#include "MuonCalibTkr.h"
#include "RootFileAnalysis.h"
#include "CIDAC2ADC.h"
#include "CalGeom.h"
#include "CalPed.h"
#include "CalAsym.h"
#include "CalMPD.h"
#include "CGCUtil.h"
#include "SimpleIniFile.h"
#include "CalHodoscope.h"
#include "AsymHists.h"
#include "MPDHists.h"

// GLAST INCLUDES
#include "digiRootData/DigiEvent.h"

// EXTLIB INCLUDES
#include "CLHEP/Geometry/Vector3D.h"
#include "TH2S.h"

// STD INCLUDES
#include <sstream>
#include <algorithm>
#include <cassert>

using namespace std;
using namespace CalUtil;
using namespace CalGeom;
using namespace CGCUtil;
using namespace CLHEP;

MuonCalibTkr::MuonCalibTkr(const SimpleIniFile &cfg,
                           const CalPed &ped,
                           const CIDAC2ADC &dac2adc,
                           AsymHists &asymHists,
                           MPDHists &mpdHists) :
  m_asymHists(asymHists),
  m_mpdHists(mpdHists),
  eventData(ped, dac2adc)
{
  readCfg(cfg);
}

void MuonCalibTkr::cfgBranches(RootFileAnalysis &rootFile) {
  // enable only needed branches in root file
  rootFile.getDigiChain()->SetBranchStatus("*", 0);
  rootFile.getDigiChain()->SetBranchStatus("m_eventId");
  rootFile.getDigiChain()->SetBranchStatus("m_runId");
  rootFile.getDigiChain()->SetBranchStatus("m_calDigiCloneCol");
  rootFile.getDigiChain()->SetBranchStatus("m_summary");

  rootFile.getSvacChain()->SetBranchStatus("*", 0);
  rootFile.getSvacChain()->SetBranchStatus("EventID");
  rootFile.getSvacChain()->SetBranchStatus("RunID");
  rootFile.getSvacChain()->SetBranchStatus("GemConditionsWord");
  rootFile.getSvacChain()->SetBranchStatus("GemDeltaEventTime");
  rootFile.getSvacChain()->SetBranchStatus("TkrNumTracks");
  rootFile.getSvacChain()->SetBranchStatus("Tkr1EndPos*");
  rootFile.getSvacChain()->SetBranchStatus("Tkr1EndDir*");

  rootFile.getSvacChain()->SetBranchAddress("RunID",
                                            &eventData.svacRunID);

  rootFile.getSvacChain()->SetBranchAddress("EventID",
                                            &eventData.svacEventID);

  rootFile.getSvacChain()->SetBranchAddress("GemConditionsWord",
                                            &eventData.gemConditionsWord);

  rootFile.getSvacChain()->SetBranchAddress("GemDeltaEventTime",
                                            &eventData.gemDeltaEventTime);

  rootFile.getSvacChain()->SetBranchAddress("TkrNumTracks",
                                            &eventData.tkrNumTracks);

  rootFile.getSvacChain()->SetBranchAddress("Tkr1EndPos",
                                            &eventData.tkr1EndPos);

  rootFile.getSvacChain()->SetBranchAddress("Tkr1EndDir",
                                            &eventData.tkr1EndDir);
}

void MuonCalibTkr::fillHists(unsigned nEntries,
                             const vector<string> &digiFileList,
                             const vector<string> &svacFileList) {
  m_asymHists.initHists();
  m_mpdHists.initHists();

  RootFileAnalysis rootFile(0,
                            &digiFileList,
                            0,
                            &svacFileList);

  // configure active branches from input ROOT trees
  cfgBranches(rootFile);

  unsigned nEvents = rootFile.getEntries();
  LogStream::get() << __FILE__ << ": Processing: " << nEvents << " events." << endl;

  /////////////////////
  // DIGI Event Loop //
  /////////////////////
  //nEvents = 100000;
  for (eventData.eventNum = 0; eventData.eventNum < nEvents; eventData.eventNum++) {
    eventData.next();
    //LogStream::get() << "event: " << eventData.eventNum << endl;

    if (eventData.eventNum % 10000 == 0) {
      // quit if we have enough entries in each histogram
      unsigned currentMin = m_mpdHists.getMinEntries();
      if (currentMin >= nEntries) break;
      LogStream::get() << "Event: " << eventData.eventNum
                       << " min entries per histogram: " << currentMin
                       << endl;
      LogStream::get().flush();

      algData.printStatus(LogStream::get());
    }

    if (!rootFile.getEvent(eventData.eventNum)) {
      LogStream::get() << "Warning, event " << eventData.eventNum << " not read." << endl;
      continue;
    }

    DigiEvent *digiEvent = rootFile.getDigiEvent();
    if (!digiEvent) {
      LogStream::get() << __FILE__ << ": Unable to read DigiEvent " << eventData.eventNum  << endl;
      continue;
    }

    // check that event ID is in sync for both SVAC & Digi
    assert(digiEvent->getRunId()  == eventData.svacRunID &&
           digiEvent->getEventId() == eventData.svacEventID);

    // quick high level event cut
    if (!eventCut())
      continue;

    if (!processEvent(*digiEvent))
      continue;
  }
}

bool MuonCalibTkr::processEvent(const DigiEvent &digiEvent) {
  if (!processTrk())
    return false;

  if (!processDigiEvent(digiEvent))
    return false;

  if (!processFinalHitList())
    return false;

  return true;
}

bool MuonCalibTkr::processTrk() {
  // extract tracker track into CLHEP geometry objects
  Vec3D tkr1EndDir(eventData.tkr1EndDir[0],
                   eventData.tkr1EndDir[1],
                   eventData.tkr1EndDir[2]);

  eventData.theta = tkr1EndDir.getTheta();
  // general angle cut theta (angle from vert) < 45 deg
  if (algData.maxThetaRads < eventData.theta)
    return false;

  algData.passTheta++;

  Vec3D tkr1EndPos(eventData.tkr1EndPos[0],
                   eventData.tkr1EndPos[1],
                   eventData.tkr1EndPos[2]);

  // find track intersection w/ each Cal lyr
  for (LyrNum lyr; lyr.isValid(); lyr++) {
    // position of tracker track @ top of current lyr
    Vec3D   trkLyrTopPos(trkToZ (tkr1EndPos, tkr1EndDir,
                                 // put me .001mm from z boundary to avoid
                                 // gnarly floating point rounding issues
                                 // most geometry is defined to .01 mm
                                 lyrCtrZ (lyr)+CsIHeight/2-.001));
    XtalIdx xtalTop(pos2Xtal (trkLyrTopPos));

    // make sure track passes through valid crystals
    if (!xtalTop.isValid())
      continue;

    Vec3D   trkLyrCtrPos(trkToZ (tkr1EndPos, tkr1EndDir, lyrCtrZ (lyr)));
    XtalIdx xtalCtr(pos2Xtal (trkLyrCtrPos));

    if (!xtalCtr.isValid())
      continue;

    Vec3D   trkLyrBtmPos(trkToZ (tkr1EndPos, tkr1EndDir,
                                 // put me .001 from z boundary to avoid
                                 // gnarly floating point rounding issues
                                 // most geometry is defined to .01 mm
                                 lyrCtrZ (lyr)-CsIHeight/2+.001));
    XtalIdx xtalBtm(pos2Xtal (trkLyrBtmPos));

    if (!xtalBtm.isValid())
      continue;

    // track passes through valid crystals
    algData.passXtalTrk++;

    // check that entry / exit point are in same xtal
    // (guarantees entry & exit from same xtal top & bottom face
    if (xtalTop != xtalBtm)
      continue;
    algData.passXtalClip++;

    // check that entry/exit point are in proper region of xtal face
    Vec3D ctrPos(xtalCtrPos (xtalCtr));

    //LogStream::get() << "xtal:\t" << xtalIdx.getCalXtalId() << endl;
    //LogStream::get() << "trkPos:\t" << tkr1EndPos << endl;
    //LogStream::get() << "trkDir:\t" << tkr1EndDir << endl;
    //LogStream::get() << "xtalCtr:\t" << xtalCtr << endl;
    //LogStream::get() << "trkXtalTop:\t" << trkLyrTopPos << endl;
    //LogStream::get() << "trkXtalCtr:\t" << trkLyrCtrPos << endl;
    //LogStream::get() << "trkXtalBtm:\t" << trkLyrBtmPos << endl;

    // test that track makes clean pass through 'meat' of xtal
    DirNum dir(xtalCtr.getLyr ().getDir());

    if (!validXtalIntersect(dir, ctrPos, trkLyrTopPos) ||
        !validXtalIntersect(dir, ctrPos, trkLyrBtmPos))
      continue;
    algData.passXtalEdge++;

    // finally we have a valid candidate xtal
    // offset @ Z center of xtal
    Vec3D ctrOffset(trkLyrCtrPos - ctrPos);

    eventData.trkHitMap[xtalCtr] = ctrOffset;
  }

  // possible that there were no good xtal intersections
  if (eventData.trkHitMap.empty())
    return false;

  return true;
}

bool MuonCalibTkr::processDigiEvent(const DigiEvent &digiEvent) {
  const TClonesArray *calDigiCol = digiEvent.getCalDigiCol();

  if (!calDigiCol) {
    LogStream::get() << "no calDigiCol found for event#" << eventData.eventNum << endl;
    return false;
  }

  TIter calDigiIter(calDigiCol);

  const CalDigi      *pCalDigi = 0;

  // loop through each digi 'hit' in one event
  while ((pCalDigi = (CalDigi *)calDigiIter.Next())) {
    const CalDigi &calDigi = *pCalDigi;   // use reference to avoid -> syntax

    eventData.hscope.addHit(calDigi);
  }

  // cut on complete hodoscope
  return hodoCut(eventData.hscope);
}

bool MuonCalibTkr::processFinalHitList() {
  for (map < XtalIdx, Vec3D >::const_iterator it = eventData.trkHitMap.begin();
       it != eventData.trkHitMap.end();
       it++) {
    const XtalIdx & xtalIdx(it->first);
    const Vec3D   & xtalPos(it->second);

    if (eventData.hscope.perLyr[xtalIdx.getLyr()] > algData.maxHitsPerLyr)
      continue;
    algData.passXtalMulti++;

    // retrieve dac values from hscope
    CalArray<XtalDiode, float> dac;
    CalArray<XtalDiode, float> adc_ped;
    CalArray<XtalDiode, bool> dacValid;
    for (XtalDiode xDiode; xDiode.isValid(); xDiode++) {
      DiodeIdx diodeIdx(xtalIdx, xDiode);
      dac[xDiode]      = eventData.hscope.dac[diodeIdx];
      adc_ped[xDiode]  = eventData.hscope.adc_ped[diodeIdx];
      
      dacValid[xDiode] = (dac[xDiode] != CIDAC2ADC::INVALID_ADC);
    }

    //-- FILL HISTS --//
    if (dacValid[XtalDiode(POS_FACE, LRG_DIODE)] &&
        dacValid[XtalDiode(NEG_FACE, LRG_DIODE)]) {
      /////////////////////
      //-- MEV PER DAC --//
      /////////////////////
      // calculate mean dac (from both faces)
      CalArray<DiodeNum, float> meanDAC;
      CalArray<DiodeNum, float> meanADC;
      for (DiodeNum diode; diode.isValid(); diode++) {
        float cosTheta = cos(eventData.theta);

        meanDAC[diode] = (dac[XtalDiode(POS_FACE, diode)] +
                          dac[XtalDiode(NEG_FACE, diode)]) / 2;
        meanDAC[diode] *= cosTheta;


        meanADC[diode] = (adc_ped[XtalDiode(POS_FACE, diode)] +
                          adc_ped[XtalDiode(NEG_FACE, diode)]) / 2;
        meanADC[diode] *= cosTheta;
      }



      ///////////////////
      //-- ASYMMETRY --//
      ///////////////////
      DirNum dir = xtalIdx.getLyr().getDir();
      float  longOffset;
      if (dir == X_DIR)
        longOffset = xtalPos.x();
      else
        longOffset = xtalPos.y();

      //-- LE DIODE MPD --//
      algData.mpdLrgFills++;
      m_mpdHists.fillDacLL(xtalIdx, meanDAC[LRG_DIODE]);
      

      //-- HE DIODE MPD --//
      if (dacValid[XtalDiode(POS_FACE, SM_DIODE)] &&
          dacValid[XtalDiode(NEG_FACE, SM_DIODE)]) {
        algData.mpdSmFills++;
        m_mpdHists.fillL2S(xtalIdx, meanDAC[LRG_DIODE], meanDAC[SM_DIODE]);
      }

      // xtra cut, asym skips region near xtal end, regardless of earlier
      // geometry cut
      unsigned short segmentNo = CalAsym::pos2xtalSegment(longOffset);
      if (segmentNo < 1 || segmentNo > 10)
        continue;

      for (AsymType asymType; asymType.isValid(); asymType++) {
        DiodeNum pDiode = asymType.getDiode(POS_FACE);
        DiodeNum nDiode = asymType.getDiode(NEG_FACE);
        if (!dacValid[XtalDiode(POS_FACE, pDiode)] ||
            !dacValid[XtalDiode(NEG_FACE, nDiode)])
          continue;

        algData.asymFills++;
        float asym = dac[XtalDiode(POS_FACE, pDiode)] /
                     dac[XtalDiode(NEG_FACE, nDiode)];
        asym = log(asym);

        m_asymHists.getHist(asymType, xtalIdx)->Fill(segmentNo, asym);
      }
    }
  }

  return true;
}

bool MuonCalibTkr::eventCut() {
  algData.nTotalEvents++;

  if (!(eventData.gemConditionsWord & 2))
    return false;
  algData.passTrigWord++;

  if (eventData.tkrNumTracks != 1)
    return false;
  algData.passTkrNumTracks++;

  if (eventData.gemDeltaEventTime
      < algData.minDeltaEventTime)
    return false;
  algData.passDeltaEventTime++;

  return true;
}

bool MuonCalibTkr::hodoCut(const CalHodoscope &hscope) {
  if (hscope.count >= algData.maxNHits)
    return false;

  algData.passHitCount++;

  return true;
}

bool MuonCalibTkr::trkCut() {
  return true;
}

bool MuonCalibTkr::hitCut() {
  return true;
}

void MuonCalibTkr::readCfg(const SimpleIniFile &cfg) {
  algData.minDeltaEventTime = cfg. getVal<unsigned>("MUON_CALIB_TKR",
                                                    "MIN_DELTA_EVENT_TIME",
                                                    2000);

  algData.xtalLongCut = cfg.       getVal<float>("MUON_CALIB_TKR",
                                                 "XTAL_MM_FROM_END",
                                                 30);

  algData.xtalOrthCut = cfg.       getVal<float>("MUON_CALIB_TKR",
                                                 "XTAL_MM_FROM_EDGE",
                                                 5);

  float maxThetaDegrees  = cfg.    getVal<float>("MUON_CALIB_TKR",
                                                 "MAX_TRK_THETA",
                                                 45);

  // convert from degree to radian
  algData.maxThetaRads  = degreesToRadians(maxThetaDegrees);

  algData.maxNHits      = cfg. getVal<unsigned short>("MUON_CALIB_TKR",
                                                      "MAX_N_HITS",
                                                      16);

  algData.maxHitsPerLyr = cfg. getVal<unsigned short>("MUON_CALIB_TKR",
                                                      "MAX_HITS_PER_LYR",
                                                      1);
}

void MuonCalibTkr::EventData::next() {
  svacEventID       = 0;
  svacRunID         = 0;

  gemConditionsWord = 0;
  gemDeltaEventTime = 0;

  tkrNumTracks      = 0;
  fill(tkr1EndPos,
       tkr1EndPos + sizeof(tkr1EndPos)/sizeof(tkr1EndPos[0]),
       0);
  fill(tkr1EndDir,
       tkr1EndDir + sizeof(tkr1EndDir)/sizeof(tkr1EndDir[0]),
       0);

  theta = 0;

  trkHitMap.clear();

  hscope.clear();
}

Vec3D MuonCalibTkr::trkToZ(const Vec3D &start,
                           const Vec3D &dir,
                           float z) {
  float zTravel = z - start.z();


  return start + dir*(zTravel/dir.z());
}

bool MuonCalibTkr::validXtalIntersect(DirNum dir,
                                      CalGeom::Vec3D xtalCtr,
                                      CalGeom::Vec3D intersect) {
  Vec3D offset(intersect - xtalCtr);

  if (dir == X_DIR) {
    if (abs(offset.x()) > CsILength/2 - algData.xtalLongCut)
      return false;
    if (abs(offset.y()) > CsIWidth/2 - algData.xtalOrthCut)
      return false;
  } else {
    if (abs(offset.y()) > CsILength/2 - algData.xtalLongCut)
      return false;
    if (abs(offset.x()) > CsIWidth/2 - algData.xtalOrthCut)
      return false;
  }

  return true;
}

void MuonCalibTkr::AlgData::printStatus(ostream &ostrm) {
  ostrm << "nTotalEvents       " << nTotalEvents << endl;
  ostrm << "passTrigWord       " << passTrigWord << endl;
  ostrm << "passTkrNumTracks   " << passTkrNumTracks << endl;
  ostrm << "passDeltaEventTime " << passDeltaEventTime << endl;
  ostrm << "passTheta          " << passTheta << endl;
  ostrm << "passHitCount       " << passHitCount << endl;
  ostrm << "passXtalTrk        " << passXtalTrk << endl;
  ostrm << "passXtalClip       " << passXtalClip << endl;
  ostrm << "passXtalEdge       " << passXtalEdge << endl;
  ostrm << "passXtalMulti      " << passXtalMulti << endl;
  ostrm << "asymFills          " << asymFills << endl;
  ostrm << "mpdLrgFills        " << mpdLrgFills << endl;
  ostrm << "mpdSmFills         " << mpdSmFills << endl;
}

