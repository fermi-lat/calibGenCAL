// $Header: $

/** @file 
    @author Zach Fewtrell
*/

// LOCAL INCLUDES
#include "../Algs/GCRCalibAlg.h"
#include "../Util/SimpleIniFile.h"
#include "../Specs/CalGeom.h"
#include "../Util/RootFileAnalysis.h"
#include "../Util/CGCUtil.h"
#include "../Hists/GCRHists.h"
#include "../CalibDataTypes/CalPed.h"
#include "../CalibDataTypes/CIDAC2ADC.h"

// GLAST INCLUDES
#include "gcrSelectRootData/GcrSelectEvent.h"
#include "digiRootData/DigiEvent.h"

// EXTLIB INCLUDES

// STD INCLUDES
#include <cmath>

using namespace std;
using namespace CalUtil;
using namespace CGCUtil;

namespace defaults {
  static const float MAX_ORTHOGONAL_SIN   = 0.5;
  static const float MAX_LONGITUDINAL_SIN = sqrt(3.0)/2.0;
  static const float MM_CUT_FROM_END      = 30;
};


static const short INVALID_ADC = -5000;

/// return longitudinal center of xtal in xtal direction
float xtalLongCtr(DirNum dir,
                  TwrNum twr)  {
  if (dir == X_DIR) {
    // get tower index in current direction
    char  nTwrX   = twr.getCol();

    // get pos of tower ctr.

    //              origin +  nTwr offet (low face) + mid tower
    float twrCtrX = 0      + (nTwrX-2)*CalGeom::twrPitch     + CalGeom::twrPitch/2;

    return twrCtrX;
  }

  // Y_DIR
  else {
    // get tower index in current direciton
    char  nTwrY   = twr.getRow();

    // get pos of twer ctr
    float twrCtrY = 0 + (nTwrY-2)*CalGeom::twrPitch + CalGeom::twrPitch/2;

    return twrCtrY;
  }
}


GCRCalibAlg::GCRCalibAlg(const SimpleIniFile *cfgFile) :
  cutCrossedFaces(true),
  cutTrackAngle(true),
  cutLongitudinalPos(true),
  maxOrthogonalSin(defaults::MAX_ORTHOGONAL_SIN),
  maxLongitudinalSin(defaults::MAX_LONGITUDINAL_SIN),
  mmCutFromEnd(defaults::MM_CUT_FROM_END) {
  if (cfgFile)
    readCfg(*cfgFile);
 
}

void GCRCalibAlg::readCfg(const SimpleIniFile &cfgFile) {
  maxOrthogonalSin   = cfgFile.getVal("GCR_CALIB",
                                      "MAX_ORTHOGONAL_SIN",
                                      maxOrthogonalSin);
  maxLongitudinalSin = cfgFile.getVal("GCR_CALIB",
                                      "MAX_LONGITUDINAL_SIN",
                                      maxLongitudinalSin);
  mmCutFromEnd       = cfgFile.getVal("GCR_CALIB",
                                      "MM_CUT_FROM_END",
                                      mmCutFromEnd);
  mmCutFromCtr       = CalGeom::CsILength/2 - mmCutFromEnd;

  cutCrossedFaces    = cfgFile.getVal("GCR_CALIB",
                                      "CUT_CROSSED_FACES",
                                      true);

  cutTrackAngle      = cfgFile.getVal("GCR_CALIB",
                                      "CUT_TRACK_ANGLE",
                                      true);

  cutLongitudinalPos = cfgFile.getVal("GCR_CALIB",
                                      "CUT_LONGITUDINAL_POS",
                                      true);
}

void GCRCalibAlg::fillHists(unsigned nEntries,
                         const vector<string> &digiRootFileList,
                         const vector<string> &gcrSelectRootFileList,
                         const CalPed &peds,
                         const CIDAC2ADC &dac2adc,
                         GCRHists &gcrHists
                         ) {
  algData.clear();
  algData.calPed  = &peds;
  algData.dac2adc = &dac2adc;
  algData.gcrHists = &gcrHists;
  

  RootFileAnalysis rootFile(0, &digiRootFileList, 0, 0, &gcrSelectRootFileList);

  // enable only needed branches in root file
  rootFile.getDigiChain()->SetBranchStatus("*", 0);
  rootFile.getDigiChain()->SetBranchStatus("m_calDigiCloneCol");
  rootFile.getDigiChain()->SetBranchStatus("m_summary");
  rootFile.getGcrSelectChain()->SetBranchStatus("m_gcrSelect");

  unsigned nEvents = rootFile.getEntries();
  LogStream::get() << __FILE__ << ": Processing: " << nEvents << " events." << endl;

  ////////////////
  // Event Loop //
  ////////////////
  eventData.clear();
  for (eventData.eventNum = 0; eventData.eventNum < nEvents; eventData.eventNum++) {
    algData.nEventsAttempted++;
    eventData.next();

    if (eventData.eventNum % 10000 == 0) {
      // quit if we have enough entries in each histogram
      unsigned currentMin = algData.gcrHists->getMinEntries();
      if (currentMin >= nEntries) break;
      LogStream::get() << "Event: " << eventData.eventNum
                       << " min entries per histogram: " << currentMin
                       << endl;
      LogStream::get().flush();
    }

    if (!rootFile.getEvent(eventData.eventNum)) {
      LogStream::get() << "Warning, event " << eventData.eventNum << " not read." << endl;
      continue;
    }

    eventData.digiEvent = rootFile.getDigiEvent();
    if (!eventData.digiEvent) {
      LogStream::get() << __FILE__ << ": Unable to read DigiEvent " << eventData.eventNum  << endl;
      continue;
    }

    eventData.gcrSelectEvent = rootFile.getGcrSelectEvent();
    if (!eventData.gcrSelectEvent) {
      LogStream::get() << __FILE__ << ": Unable to read GcrSelectedEvent " << eventData.eventNum  << endl;
      continue;
    }
    algData.nEventsRead++;

    processGcrEvent();

    if (!eventData.finalHitMap.empty())
      processDigiEvent();
  }

  algData.summarizeAlg(LogStream::get());
}

void GCRCalibAlg::AlgData::summarizeAlg(ostream &ostrm) const {
  ostrm << "nEventsAttempted: " << nEventsAttempted << endl
        << "nEventsRead: " << nEventsRead << endl
        << "nGcrHits: " << nGcrHits << endl
        << "nHitsXface: " << nHitsXface << endl
        << "nHitsAngle: " << nHitsAngle << endl
        << "nHitsPos: " << nHitsPos << endl
        << "nFillsLE: " << nFills[LRG_DIODE] << endl
        << "nFillsHE: " << nFills[SM_DIODE]  << endl;
}

void GCRCalibAlg::processGcrEvent() {
  GcrSelect *gcrSelect = eventData.gcrSelectEvent->getGcrSelect();

  if (!gcrSelect) {
    LogStream::get() << __FILE__ << ": No GcrSelect data found: " << eventData.eventNum  << endl;
    return;
  }

  const TObjArray *      gcrSelectedXtalCol = gcrSelect->getGcrSelectedXtalCol();
  if (!gcrSelectedXtalCol) {
    LogStream::get() << __FILE__ << ": No GcrSelectedXtalCol found: " << eventData.eventNum  << endl;
    return;
  }

  const TClonesArray *   calDigiCol = eventData.digiEvent->getCalDigiCol();
  if (!calDigiCol) {
    LogStream::get() << "no calDigiCol found for event#" << eventData.eventNum << endl;
    return;
  }

  TIter gcrXtalIter(gcrSelectedXtalCol);

  const GcrSelectedXtal *pGcrXtal = 0;

  TIter calDigiIter(calDigiCol);

  /////////////////////////////////////////
  /// Xtal Hit Loop ///////////////////////
  /////////////////////////////////////////
  while ((pGcrXtal = (GcrSelectedXtal *)gcrXtalIter.Next())) {
    algData.nGcrHits++;
    processGcrHit(*pGcrXtal);
  }
}

void GCRCalibAlg::processGcrHit(const GcrSelectedXtal &gcrXtal) {
  //-- CUT 1: CROSSED XTAL FACES --//
  if (cutCrossedFaces) {
    unsigned crossedFaces = gcrXtal.getCrossedFaces();
    //     cout << eventData.eventNum << " " << XtalIdx(idents::CalXtalId(gcrXtal.getXtalId())).val()
    //          << " " << crossedFaces << endl;
    if (!crossedFaceCut(crossedFaces))
      return;
  }
  algData.nHitsXface++;

  //-- CUT 2: TRACK ANGLE --//
  // get xtal dir (X or Y)
  idents::CalXtalId xtalId(gcrXtal.getXtalId());

  // get path info.
  TVector3 entryPoint = gcrXtal.getEntryPoint();
  TVector3 exitPoint  = gcrXtal.getExitPoint();

  if (cutTrackAngle) {
    DirNum   xtalDir = XtalIdx(xtalId).getLyr().getDir();
    TVector3 pathVec = exitPoint - entryPoint;

    if (!angleCut(pathVec, xtalDir))
      return;
  }
  algData.nHitsAngle++;

  //-- CUT 3: XTAL POS (cut near end of xtal) --//
  XtalIdx xtalIdx(xtalId);

  if (cutLongitudinalPos)
    if (!posCut(xtalIdx, entryPoint, exitPoint))
      return;
  algData.nHitsPos++;

  //-- ADD XTAL TO HIT 'FINAL' HIT LIST --//
  eventData.finalHitMap[xtalIdx] = &gcrXtal;
}

void GCRCalibAlg::processDigiEvent() {
  const TClonesArray *calDigiCol = eventData.digiEvent->getCalDigiCol();


  if (!calDigiCol) {
    LogStream::get() << "no calDigiCol found for event#" << eventData.eventNum << endl;
    return;
  }

  TIter calDigiIter(calDigiCol);

  const CalDigi *     pCalDigi = 0;

  // loop through each 'hit' in one event
  while ((pCalDigi = (CalDigi *)calDigiIter.Next())) {
    // skip once we've processed every digi which
    // corresponds to a 'good' GCR hit
    if (eventData.finalHitMap.empty())
      break;

    const CalDigi &calDigi = *pCalDigi; // use reference to avoid -> syntax

    processDigiHit(calDigi);
  }
}

void GCRCalibAlg::processDigiHit(const CalDigi &calDigi) {
  //-- XtalId --//
  idents::CalXtalId id(calDigi.getPackedId()); // get interaction information

  XtalIdx xtalIdx(id);

  // see if current digi matches the 'blessed' list from numerous gcr cuts
  // remove from list if it does and process it
  // this slick STL erase() method will return non-zero only if it finds the key
  EventData::FinalHitMapIter hitIter = eventData.finalHitMap.find(xtalIdx);
  if (hitIter != eventData.finalHitMap.end()) {
    const GcrSelectedXtal &gcrXtal = *(hitIter->second);

    // now remove this hit from list.
    eventData.finalHitMap.erase(hitIter);

    //-- BEST RANGE? --//
    // get 'best' range for each face
    // we only deal with ranges >= best range
    CalArray<FaceNum, RngNum> bestRng;
    for (FaceNum face; face.isValid(); face++)
      bestRng[face] = calDigi.getRange(0, (CalXtalId::XtalFace)face.val());

    // we will always round mixed range results up
    // so we want the max of the 2 ranges
    RngNum maxBestRng(*(max_element (bestRng.begin(),
                                     bestRng.end())));

    //-- RETRIEVE MEAN SIGNAL LEVELS : LOOP THROUGH ALL CHANNELS --//
    CalArray<XtalRng, float> xtalADC;
    CalArray<DiodeNum, float> meanDAC;
    CalArray<XtalDiode, float> dac;
    xtalADC.fill(INVALID_ADC);
    meanDAC.fill(INVALID_ADC);
    dac.fill(INVALID_ADC);

    for (RngNum rng(maxBestRng); rng.isValid(); rng++) {
      DiodeNum diode = rng.getDiode();
      for (FaceNum face; face.isValid(); face++) {
        // raw adc
        float adc = calDigi.getAdcSelectedRange(rng.val(),
                                                (CalXtalId::XtalFace)face.val());
        if (adc < 0) {
          LogStream::get() << "Couldn't get adc val for face=" << face.val()
                           << " rng=" << rng.val() << endl;
          break;
        }

        //-- RETRIEVE ADC / PED / CIDAC --//
        RngIdx rngIdx(xtalIdx,
                      face,
                      rng);

        float ped = algData.calPed->getPed(rngIdx);
        xtalADC[XtalRng(face,rng)] = adc - ped;

        // only process DAC in LEX1 / HEX8
        if (rng != LEX1 && rng != HEX8)
          continue;

        // only process DAC in LEX1 / HEX8
        if (rng == LEX1 || rng == HEX8)
          // double check that all dac signals are > 0
          dac[XtalDiode(face,diode)] = algData.dac2adc->adc2dac(rngIdx, adc);

      }
      
      // mean dac
      if (rng == LEX1 || rng == HEX8)
        if (dac[XtalDiode(POS_FACE,diode)] != INVALID_ADC &&
            dac[XtalDiode(NEG_FACE,diode)] != INVALID_ADC)
          meanDAC[diode]  = sqrt(dac[XtalDiode(POS_FACE,diode)] *dac[XtalDiode(NEG_FACE,diode)]);

      // pathlength correct
      meanDAC[diode] *= CalGeom::CsIHeight/gcrXtal.getPathLength();
    }

    //-- FILL ADC HISTOGRAMS --//
    for (FaceNum face; face.isValid(); face++)
      for (RngNum rng(maxBestRng); rng.isValid(); rng++) {
        XtalRng xRng(face,rng);

        //-- MEAN ADC --//
        if (xtalADC[xRng] == INVALID_ADC)
          continue;

        RngIdx rngIdx(xtalIdx, xRng);
        algData.gcrHists->fillADCHit(rngIdx, xtalADC[xRng]);

        //-- ADC RATIO --//
        XtalRng nextRng(face,rng+1); 
        for (FaceNum face; face.isValid(); face++)
          if (rng <= HEX8 && xtalADC[nextRng] != INVALID_ADC)
            algData.gcrHists->fillAdcRatio(rngIdx,
                                           xtalADC[xRng],
                                           xtalADC[nextRng]);
                                         

        //-- FILL DAC HISTOGRAMS --//
        for (DiodeNum diode; diode.isValid(); diode++) {
          //-- MEAN DAC --//

          if (meanDAC[diode] == INVALID_ADC)
            continue;

          algData.gcrHists->fillMeanCIDAC(diode, xtalIdx, meanDAC[diode]);
        }

        //-- MEAN DAC RATIO --//
        for (FaceNum face; face.isValid(); face++) {
          FaceIdx faceIdx(xtalIdx,face);
          if (dac[XtalDiode(face,LRG_DIODE)] != INVALID_ADC &&
              dac[XtalDiode(face,SM_DIODE)] != INVALID_ADC) {
            algData.gcrHists->fillDacRatio(faceIdx, 
                                           dac[XtalDiode(face,LRG_DIODE)],
                                           dac[XtalDiode(face,SM_DIODE)]);
          }
        }
      } // for (rng) /* fill histograms */
  }   // gcrXtal
}

/// represent possible faces a GcrSelect track can cross on a single crystal.
typedef enum {
  XFACE_ZTOP,
  XFACE_ZBOT,
  XFACE_XLEFT,
  XFACE_XRIGHT,
  XFACE_YLEFT,
  XFACE_YRIGHT
} XFACE_BITPOS;

bool GCRCalibAlg::crossedFaceCut( unsigned crossedFaces) const {
  static const unsigned topBtmOnly =
    1<<XFACE_ZTOP | 1<<XFACE_ZBOT;


  return crossedFaces == topBtmOnly;
}

bool GCRCalibAlg::angleCut(const TVector3 &pathVec,
                        DirNum dir) const {
  TVector3 pathDir = pathVec.Unit();

  float    xSin    = abs(pathDir.x());
  float    ySin    = abs(pathDir.y());


  if (dir == X_DIR) {
    if (xSin > maxLongitudinalSin)
      return false;
    if (ySin > maxOrthogonalSin)
      return false;
  }
  else {
    if (ySin > maxLongitudinalSin)
      return false;
    if (xSin > maxOrthogonalSin)
      return false;
  }

  return true;
}

bool GCRCalibAlg::posCut(XtalIdx xtalIdx,
                      const TVector3 &entry,
                      const TVector3 &exit) const {
  TwrNum twr     = xtalIdx.getTwr();
  DirNum dir     = xtalIdx.getLyr().getDir();

  float  xtalCtr = xtalLongCtr(dir, twr);


  if (dir == X_DIR) {
    if (abs(entry.x() - xtalCtr) > mmCutFromCtr)
      return false;
    if (abs(exit.x() - xtalCtr) > mmCutFromCtr)
      return false;
  } else {
    if (abs(entry.y() - xtalCtr) > mmCutFromCtr)
      return false;
    if (abs(exit.y() - xtalCtr) > mmCutFromCtr)
      return false;
  }

  return true;
}

