// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Algs/GCRCalibAlg.cxx,v 1.5 2007/06/07 17:45:43 fewtrell Exp $

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
#include "facilities/Util.h"

// EXTLIB INCLUDES

// STD INCLUDES
#include <cmath>

using namespace std;
using namespace CalUtil;
using namespace facilities;

namespace calibGenCAL {
  using namespace CGCUtil;

  namespace defaults {
    static const float MAX_ORTHOGONAL_SIN(0.5);
    static const float MAX_LONGITUDINAL_SIN(sqrt(3.0)/2.0);
    static const float MM_CUT_FROM_END(30);
    static const int zListArr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 12, 14, 26};
    static const int zListSz(sizeof(zListArr)/sizeof(zListArr[0]));
    static const vector<int> zList(zListArr, zListArr+zListSz);
  };

  namespace {
    /// represent possible faces a GcrSelect track can cross on a single crystal.
    typedef enum {
      XFACE_ZTOP,
      XFACE_ZBOT,
      XFACE_XLEFT,
      XFACE_XRIGHT,
      XFACE_YLEFT,
      XFACE_YRIGHT
    } XFACE_BITPOS;


    static const unsigned topBtmOnly =
    1<<XFACE_ZTOP | 1<<XFACE_ZBOT;
  };


  /// return longitudinal center of xtal in xtal direction
  float xtalLongCtr(const DirNum dir,
                    const TwrNum twr)  {
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


  GCRCalibAlg::GCRCalibAlg(const string &cfgPath) :
    cutCrossedFaces(true),
    cutTrackAngle(true),
    cutLongitudinalPos(true),
    maxOrthogonalSin(defaults::MAX_ORTHOGONAL_SIN),
    maxLongitudinalSin(defaults::MAX_LONGITUDINAL_SIN),
    mmCutFromEnd(defaults::MM_CUT_FROM_END) {
    readCfg(cfgPath);
  }

  void GCRCalibAlg::readCfg(string cfgPath) {
    /// default case
    if (cfgPath=="")
      cfgPath = CGC_DEFAULT_CFGPATH;

    Util::expandEnvVar(&cfgPath);
    const SimpleIniFile cfgFile(cfgPath);

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
  
    vector<int> zVec(cfgFile.getVector<int>("GCR_CALIB",
                                            "Z_LIST",
                                            ", ",
                                            defaults::zList));

    zList.insert(zVec.begin(), zVec.end());
  }

  void GCRCalibAlg::fillHists(const unsigned nEventsMax,
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

    const unsigned nTotalEvents = rootFile.getEntries();
    LogStrm::get() << __FILE__ << ": Processing: " << nTotalEvents << " events." << endl;

    ////////////////
    // Event Loop //
    ////////////////
    eventData.clear();
    for (eventData.eventNum = 0; eventData.eventNum < nTotalEvents; eventData.eventNum++) {
      algData.nEventsAttempted++;
      eventData.next();

      if (eventData.eventNum % 10000 == 0 || algData.nEventsAttempted == nEventsMax) {
        LogStrm::get() << "Event: " << eventData.eventNum
                       << endl;
        LogStrm::get().flush();
      }

      if (algData.nEventsAttempted == nEventsMax)
        break;

      if (!rootFile.getEvent(eventData.eventNum)) {
        LogStrm::get() << "Warning, event " << eventData.eventNum << " not read." << endl;
        continue;
      }

      eventData.digiEvent = rootFile.getDigiEvent();
      if (!eventData.digiEvent) {
        LogStrm::get() << __FILE__ << ": Unable to read DigiEvent " << eventData.eventNum  << endl;
        continue;
      }

      eventData.gcrSelectEvent = rootFile.getGcrSelectEvent();
      if (!eventData.gcrSelectEvent) {
        LogStrm::get() << __FILE__ << ": Unable to read GcrSelectedEvent " << eventData.eventNum  << endl;
        continue;
      }
      algData.nEventsRead++;

      processGcrEvent();

      if (!eventData.finalHitMap.empty())
        processDigiEvent();
    }

    algData.summarizeAlg(LogStrm::get());
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
      LogStrm::get() << __FILE__ << ": No GcrSelect data found: " << eventData.eventNum  << endl;
      return;
    }

    const GcrSelectVals *const gcrSelectVals = dynamic_cast<const GcrSelectVals*>(gcrSelect->getGcrSelectVals());
    if (gcrSelectVals)
      eventData.inferredZ = max(0,gcrSelectVals->getInferedZ());
  

    const TObjArray *const      gcrSelectedXtalCol = gcrSelect->getGcrSelectedXtalCol();
    if (!gcrSelectedXtalCol) {
      LogStrm::get() << __FILE__ << ": No GcrSelectedXtalCol found: " << eventData.eventNum  << endl;
      return;
    }

    TIter gcrXtalIter(gcrSelectedXtalCol);

    const GcrSelectedXtal *pGcrXtal = 0;

    /////////////////////////////////////////
    /// Xtal Hit Loop ///////////////////////
    /////////////////////////////////////////
    while ((pGcrXtal = dynamic_cast<GcrSelectedXtal*>(gcrXtalIter.Next()))) {
      algData.nGcrHits++;
      processGcrHit(*pGcrXtal);
    }
  }

  void GCRCalibAlg::processGcrHit(const GcrSelectedXtal &gcrXtal) {
    //-- CUT 1: CROSSED XTAL FACES --//
    if (cutCrossedFaces) {
      unsigned crossedFaces = gcrXtal.getCrossedFaces();
      if (!crossedFaceCut(crossedFaces))
        return;
    }
    algData.nHitsXface++;

    //-- CUT 2: TRACK ANGLE --//
    // get xtal dir (X or Y)
    const idents::CalXtalId xtalId(gcrXtal.getXtalId());

    // get path info.
    const TVector3 entryPoint = gcrXtal.getEntryPoint();
    const TVector3 exitPoint  = gcrXtal.getExitPoint();

    if (cutTrackAngle) {
      const DirNum   xtalDir = XtalIdx(xtalId).getLyr().getDir();
      const TVector3 pathVec = exitPoint - entryPoint;

      if (!angleCut(pathVec, xtalDir))
        return;
    }
    algData.nHitsAngle++;

    //-- CUT 3: XTAL POS (cut near end of xtal) --//
    const XtalIdx xtalIdx(xtalId);

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
      LogStrm::get() << "no calDigiCol found for event#" << eventData.eventNum << endl;
      return;
    }

    TIter calDigiIter(calDigiCol);

    const CalDigi *     pCalDigi = 0;

    // loop through each 'hit' in one event
    while ((pCalDigi = dynamic_cast<CalDigi*>(calDigiIter.Next()))) {
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
    const idents::CalXtalId id(calDigi.getPackedId()); // get interaction information

    const XtalIdx xtalIdx(id);

    // see if current digi matches the 'blessed' list from numerous gcr cuts
    // remove from list if it does and process it
    // this slick STL erase() method will return non-zero only if it finds the key
    EventData::FinalHitMapIter hitIter(eventData.finalHitMap.find(xtalIdx));
    if (hitIter != eventData.finalHitMap.end()) {
      const GcrSelectedXtal &gcrXtal(*(hitIter->second));

      // now remove this hit from list.
      eventData.finalHitMap.erase(hitIter);

      //-- BEST RANGE? --//
      // get 'best' range for each face
      // we only deal with ranges >= best range
      CalArray<FaceNum, RngNum> bestRng;
      for (FaceNum face; face.isValid(); face++)
        bestRng[face] = RngNum(calDigi.getRange(0, (CalXtalId::XtalFace)face.val()));

      // we will always round mixed range results up
      // so we want the max of the 2 ranges
      const RngNum maxBestRng(*(max_element (bestRng.begin(),
                                             bestRng.end())));

      //-- RETRIEVE MEAN SIGNAL LEVELS : LOOP THROUGH ALL CHANNELS --//
      /// pedestal subtracted ADC values, per ADC channel
      CalArray<XtalRng, float> adcPed;
      CalArray<DiodeNum, float> meanDAC;
      CalArray<XtalDiode, float> dac;
      adcPed.fill(CIDAC2ADC::INVALID_ADC);
      meanDAC.fill(CIDAC2ADC::INVALID_ADC);
      dac.fill(CIDAC2ADC::INVALID_ADC);

      for (RngNum rng(maxBestRng); rng.isValid(); rng++) {
        const DiodeNum diode(rng.getDiode());
        for (FaceNum face; face.isValid(); face++) {
          // raw adc
          const float adc = calDigi.getAdcSelectedRange(rng.val(),
                                                        (CalXtalId::XtalFace)face.val());
          if (adc < 0) {
            LogStrm::get() << "Couldn't get adc val for face=" << face.val()
                           << " rng=" << rng.val() << endl;
            break;
          }
          
          //-- RETRIEVE ADC / PED / CIDAC --//
          const XtalRng xRng(face,rng);
          const RngIdx rngIdx(xtalIdx,
                              xRng);
          
          const float ped(algData.calPed->getPed(rngIdx));
          adcPed[xRng] = adc - ped;

          // only process DAC in LEX1 / HEX8
          if (rng == LEX1 || rng == HEX8)
            dac[XtalDiode(face,diode)] = algData.dac2adc->adc2dac(rngIdx, adcPed[xRng]);
        }
        
        // mean dac
        if (rng == LEX1 || rng == HEX8)
          if (dac[XtalDiode(POS_FACE,diode)] != CIDAC2ADC::INVALID_ADC &&
              dac[XtalDiode(NEG_FACE,diode)] != CIDAC2ADC::INVALID_ADC) {
            meanDAC[diode]  = sqrt(dac[XtalDiode(POS_FACE,diode)] *dac[XtalDiode(NEG_FACE,diode)]);
            
            // pathlength correct
            meanDAC[diode] *= CalGeom::CsIHeight/gcrXtal.getPathLength();
          }
      }
      
      //-- FILL ADC HISTOGRAMS --//
      for (FaceNum face; face.isValid(); face++)
        for (RngNum rng(maxBestRng); rng.isValid(); rng++) {
          const XtalRng xRng(face,rng);

          //-- MEAN ADC --//
          if (adcPed[xRng] == CIDAC2ADC::INVALID_ADC)
            continue;

          const RngIdx rngIdx(xtalIdx, xRng);

          //-- ADC RATIO --//
          const XtalRng nextRng(face,
                                RngNum(rng.val()+1)); 
          for (FaceNum face; face.isValid(); face++)
            if (rng <= HEX8 && adcPed[nextRng] != CIDAC2ADC::INVALID_ADC)
              algData.gcrHists->fillAdcRatio(rngIdx,
                                             adcPed[xRng],
                                             adcPed[nextRng]);
        }

      //-- FILL DAC HISTOGRAMS --//
      for (DiodeNum diode; diode.isValid(); diode++) {
        //-- MEAN DAC --//

        if (meanDAC[diode] == CIDAC2ADC::INVALID_ADC)
          continue;

        /// only fill these histograms for z's of interest.
        if (zList.find(eventData.inferredZ) != zList.end())
          algData.gcrHists->fillMeanCIDACZ(xtalIdx,
                                           diode,
                                           eventData.inferredZ,
                                           meanDAC[diode]);

        /// fill in histograms which do not use Z information
        algData.gcrHists->fillMeanCIDAC(xtalIdx,
                                        diode,
                                        meanDAC[diode]);

      }

      //-- MEAN DAC RATIO --//
      for (FaceNum face; face.isValid(); face++) {
        const FaceIdx faceIdx(xtalIdx,face);
        if (dac[XtalDiode(face,LRG_DIODE)] != CIDAC2ADC::INVALID_ADC &&
            dac[XtalDiode(face,SM_DIODE)] != CIDAC2ADC::INVALID_ADC) {
          algData.gcrHists->fillDACRatio(faceIdx, 
                                         dac[XtalDiode(face,LRG_DIODE)],
                                         dac[XtalDiode(face,SM_DIODE)]);
        }

      } // for (rng) /* fill histograms */
    }   // gcrXtal
  }

  bool GCRCalibAlg::crossedFaceCut( const unsigned crossedFaces) const {
    return crossedFaces == topBtmOnly;
  }

  bool GCRCalibAlg::angleCut(const TVector3 &pathVec,
                             const DirNum dir) const {
    const TVector3 pathDir = pathVec.Unit();

    const float    xSin    = abs(pathDir.x());
    const float    ySin    = abs(pathDir.y());


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

  bool GCRCalibAlg::posCut(const XtalIdx xtalIdx,
                           const TVector3 &entry,
                           const TVector3 &exit) const {
    const TwrNum twr     = xtalIdx.getTwr();
    const DirNum dir     = xtalIdx.getLyr().getDir();

    const float  xtalCtr = xtalLongCtr(dir, twr);


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
}; // namespace calibGenCAL

