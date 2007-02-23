// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/GCRHists.cxx,v 1.7 2006/09/20 15:46:43 fewtrell Exp $
/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "GCRHists.h"
#include "CalGeom.h"
#include "SimpleIniFile.h"
#include "RootFileAnalysis.h"
#include "CalMPD.h"
#include "CalPed.h"
#include "CIDAC2ADC.h"
#include "CGCUtil.h"

// GLAST INCLUDES
#include "gcrSelectRootData/GcrSelectEvent.h"
#include "digiRootData/DigiEvent.h"

// EXTLIB INCLUDES
#include "TH1S.h"
#include "TProfile.h"

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
float xtalLongCtr(DirNum dir, TwrNum twr)  {

  if (dir == X_DIR) {
    // get tower index in current direction
    char nTwrX = twr.getCol();
            
    // get pos of tower ctr.

    //              origin +  nTwr offet (low face) + mid tower
    float twrCtrX = 0      + (nTwrX-2)*CalGeom::twrPitch     + CalGeom::twrPitch/2;

    return twrCtrX;
  }

  // Y_DIR
  else {
    // get tower index in current direciton
    char nTwrY = twr.getRow();

    // get pos of twer ctr
    float twrCtrY = 0 + (nTwrY-2)*CalGeom::twrPitch + CalGeom::twrPitch/2;

    return twrCtrY;
  }
}

GCRHists::GCRHists(const SimpleIniFile *cfgFile,
                   bool summaryMode
                   ) :
  m_meanDACHists(DiodeNum::N_VALS),
  m_meanADCHists(RngNum::N_VALS),
  m_dacRatioProfs(XtalIdx::N_VALS),
  m_adcRatioProfs(3),   // only need index of 3 (0vs1,1vs2,2vs3)
  m_adcRatioSumProf(3), // only need index of 3 (0vs1,1vs2,2vs3)
  cutCrossedFaces(true),
  cutTrackAngle(true),
  cutLongitudinalPos(true),
  maxOrthogonalSin(defaults::MAX_ORTHOGONAL_SIN),
  maxLongitudinalSin(defaults::MAX_LONGITUDINAL_SIN),
  mmCutFromEnd(defaults::MM_CUT_FROM_END),
  summaryMode(summaryMode)
{
  if (cfgFile)
    readCfg(*cfgFile);
}

void GCRHists::readCfg(const SimpleIniFile &cfgFile) {
  maxOrthogonalSin = cfgFile.getVal("GCR_CALIB",
                                    "MAX_ORTHOGONAL_SIN",
                                    maxOrthogonalSin);
  maxLongitudinalSin = cfgFile.getVal("GCR_CALIB",
                                      "MAX_LONGITUDINAL_SIN",
                                      maxLongitudinalSin);
  mmCutFromEnd = cfgFile.getVal("GCR_CALIB",
                                "MM_CUT_FROM_END",
                                mmCutFromEnd);
  mmCutFromCtr = CalGeom::CsILength/2 - mmCutFromEnd;


  cutCrossedFaces = cfgFile.getVal("GCR_CALIB",
                                   "CUT_CROSSED_FACES",
                                   true);
  
  cutTrackAngle = cfgFile.getVal("GCR_CALIB",
                                 "CUT_TRACK_ANGLE",
                                 true);
  
  cutLongitudinalPos = cfgFile.getVal("GCR_CALIB",
                                      "CUT_LONGITUDINAL_POS",
                                      true);
}  

void GCRHists::initHists() {
  //-- INIT SUMMARY HISTS  --//
  string histname;

  for (DiodeNum diode; diode.isValid(); diode++) {
    histname = genMeanDACSumHistName(diode);
    m_meanDACSumHist[diode] = new TH1S(histname.c_str(),
                                       histname.c_str(),
                                       4096,0,4095);
  }

  for (RngNum rng; rng.isValid(); rng++) {
    histname = genMeanADCSumHistName(rng);
    m_meanADCSumHist[rng] = new TH1S(histname.c_str(),
                                     histname.c_str(),
                                     4096,0,4095);
  }

  histname = genDACRatioSumProfName();
  m_dacRatioSumProf = new TProfile(histname.c_str(),
                                   histname.c_str(),
                                   4096,0,4095);

  for (RngNum rng; rng <= HEX8; rng++) {
    histname = genADCRatioSumProfName(rng);
    m_adcRatioSumProf[rng] = new TProfile(histname.c_str(),
                                          histname.c_str(),
                                          4096,0,4095);
  }

                              
  if (!summaryMode) {
    //-- INIT PER-XTAL HISTS --//
    for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
      for (DiodeNum diode; diode.isValid(); diode++) {
        histname = genMeanDACHistName(diode, xtalIdx);
        m_meanDACHists[diode][xtalIdx] = new TH1S(histname.c_str(),
                                                  histname.c_str(),
                                                  4096,0,4095);
      }

      for (RngNum rng; rng.isValid(); rng++) {
        histname = genMeanADCHistName(rng, xtalIdx);
        m_meanADCHists[rng][xtalIdx] = new TH1S(histname.c_str(),
                                                histname.c_str(),
                                                4096,0,4095);
      }
      
      histname = genDACRatioProfName(xtalIdx);
      m_dacRatioProfs[xtalIdx] = new TProfile(histname.c_str(),
                                              histname.c_str(),
                                              4096,0,4095);
      
      for (RngNum rng; rng <= HEX8; rng++) {
        histname = genADCRatioProfName(rng, xtalIdx);
        m_adcRatioProfs[rng][xtalIdx] = new TProfile(histname.c_str(),
                                                     histname.c_str(),
                                                     4096,0,4095);
      }
      
    } 
  }
}


void GCRHists::fillHists(unsigned nEntries,
                       const vector<string> &digiRootFileList, 
                       const vector<string> &gcrSelectRootFileList, 
                       const CalPed &peds,
                       const CIDAC2ADC &dac2adc) {
  initHists();
  algData.clear();
  algData.calPed  = &peds;
  algData.dac2adc = &dac2adc;
  
  
  RootFileAnalysis rootFile(0, &digiRootFileList, 0, 0, &gcrSelectRootFileList);
  // enable only needed branches in root file
  rootFile.getDigiChain()->SetBranchStatus("*",0);
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
      unsigned currentMin = getMinEntries();
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

void GCRHists::AlgData::summarizeAlg(ostream &ostrm) const {
  ostrm << "nEventsAttempted: " << nEventsAttempted << endl
        << "nEventsRead: " << nEventsRead << endl
        << "nGcrHits: " << nGcrHits << endl
        << "nHitsXface: " << nHitsXface << endl
        << "nHitsAngle: " << nHitsAngle << endl
        << "nHitsPos: " << nHitsPos << endl
        << "nFillsLE: " << nFills[LRG_DIODE] << endl
        << "nFillsHE: " << nFills[SM_DIODE]  << endl;
}

void GCRHists::processGcrEvent() {
  GcrSelect *gcrSelect = eventData.gcrSelectEvent->getGcrSelect();
  if (!gcrSelect) {
    LogStream::get() << __FILE__ << ": No GcrSelect data found: " << eventData.eventNum  << endl;
    return;
  }

  const TObjArray *gcrSelectedXtalCol = gcrSelect->getGcrSelectedXtalCol();
  if (!gcrSelectedXtalCol) {
    LogStream::get() << __FILE__ << ": No GcrSelectedXtalCol found: " << eventData.eventNum  << endl;
    return;
  }

  const TClonesArray *calDigiCol = eventData.digiEvent->getCalDigiCol();
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
  while ((pGcrXtal = (GcrSelectedXtal*)gcrXtalIter.Next())) {
    algData.nGcrHits++;
    processGcrHit(*pGcrXtal);
  }
}


void GCRHists::processGcrHit(const GcrSelectedXtal &gcrXtal) {

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
  TVector3 exitPoint = gcrXtal.getExitPoint();

  if (cutTrackAngle) {
    DirNum xtalDir = XtalIdx(xtalId).getLyr().getDir();
    TVector3 pathVec = exitPoint - entryPoint;

    if (!angleCut(pathVec, xtalDir))
      return;
  }
  algData.nHitsAngle++;

  //-- CUT 3: XTAL POS (cut near end of xtal) --//
  XtalIdx xtalIdx(xtalId);
  
  if (cutLongitudinalPos) {
    if (!posCut(xtalIdx, entryPoint, exitPoint))
      return;
  }
  algData.nHitsPos++;


  //-- ADD XTAL TO HIT 'FINAL' HIT LIST --//
  eventData.finalHitMap[xtalIdx] = &gcrXtal;
}

void GCRHists::processDigiEvent() {
    
  const TClonesArray* calDigiCol = eventData.digiEvent->getCalDigiCol();
  if (!calDigiCol) {
    LogStream::get() << "no calDigiCol found for event#" << eventData.eventNum << endl;
    return;
  }
                      
  TIter calDigiIter(calDigiCol);
  const CalDigi *pCalDigi = 0;
        
  // loop through each 'hit' in one event
  while ((pCalDigi = (CalDigi*)calDigiIter.Next())) {
    // skip once we've processed every digi which 
    // corresponds to a 'good' GCR hit
    if (eventData.finalHitMap.empty())
      break;
    
    const CalDigi &calDigi = *pCalDigi; // use reference to avoid -> syntax


    processDigiHit(calDigi);
  }
}

void GCRHists::processDigiHit(const CalDigi &calDigi) {
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
    RngNum maxBestRng(*(max_element(bestRng.begin(),
                                    bestRng.end())));

    //-- RETRIEVE MEAN SIGNAL LEVELS : LOOP THROUGH ALL CHANNELS --//
    CalArray<RngNum, float> meanADC;
    CalArray<DiodeNum, float> meanDAC;
    meanADC.fill(INVALID_ADC);
    meanDAC.fill(INVALID_ADC);
    
    for (RngNum rng(maxBestRng); rng.isValid(); rng++) {
      DiodeNum diode = rng.getDiode();
      
      CalArray<FaceNum, float> adc;
      CalArray<FaceNum, float> dac;
      adc.fill(INVALID_ADC);
      dac.fill(INVALID_ADC);
      
      for (FaceNum face; face.isValid(); face++) {
        // raw adc
        short tmpADC = calDigi.getAdcSelectedRange(rng.val(), 
                                                   (CalXtalId::XtalFace)face.val());
        if (tmpADC < 0) {
          LogStream::get() << "Couldn't get adc val for face=" << face.val()
                  << " rng=" << rng.val() << endl;
          break;
        }
        
        //-- RETRIEVE ADC / PED / CIDAC --//
        RngIdx rngIdx(xtalIdx,face,rng);
        float ped = algData.calPed->getPed(rngIdx);
        adc[face] = tmpADC - ped;

        // only process DAC in LEX1 / HEX8
        if (rng != LEX1 && rng != HEX8) 
          continue;
        
        // double check that all dac signals are > 0
        dac[face] = algData.dac2adc->adc2dac(rngIdx, adc[face]); 
      }

      if (find(adc.begin(), adc.end(), INVALID_ADC) != adc.end())
        continue;
      
      meanADC[rng] = sqrt(adc[POS_FACE]*adc[NEG_FACE]);

      // only process DAC in LEX1 / HEX8
      if (rng != LEX1 && rng != HEX8) 
        continue;

      if (find(dac.begin(), dac.end(), INVALID_ADC) != dac.end())
        continue;

      meanDAC[diode] = sqrt(dac[POS_FACE]*dac[NEG_FACE]);

      // pathlength correct
      meanDAC[diode] *= CalGeom::CsIHeight/gcrXtal.getPathLength();
    }

    //-- FILL HISTOGRAMS --//
    for (RngNum rng(maxBestRng); rng.isValid(); rng++) {
      //-- MEAN ADC --//
      if (meanADC[rng] == INVALID_ADC)
        continue;

      if (m_meanADCHists[rng][xtalIdx])
        m_meanADCHists[rng][xtalIdx]->Fill(meanADC[rng]);
      m_meanADCSumHist[rng]->Fill(meanADC[rng]);

      //-- ADC RATIO --//
      RngNum nextRng(rng); nextRng++;
      if (rng <= HEX8 && meanADC[nextRng] != INVALID_ADC) {
        if (m_adcRatioProfs[rng][xtalIdx])
          m_adcRatioProfs[rng][xtalIdx]->Fill(meanADC[rng], meanADC[nextRng]);
        m_adcRatioSumProf[rng]->Fill(meanADC[rng], meanADC[nextRng]);
      }

      //-- MEAN DAC --//
      // only process DAC in LEX1 / HEX8
      if (rng != LEX1 && rng != HEX8) 
        continue;

      DiodeNum diode = rng.getDiode();

      if (meanDAC[diode] == INVALID_ADC)
        continue;
      if (m_meanDACHists[diode][xtalIdx])
        m_meanDACHists[diode][xtalIdx]->Fill(meanDAC[diode]);
      m_meanDACSumHist[diode]->Fill(meanDAC[diode]);

      //-- MEAN DAC RATIO --//
      if (diode == LRG_DIODE && meanDAC[SM_DIODE] != INVALID_ADC) {
        if (m_dacRatioProfs[xtalIdx])
          m_dacRatioProfs[xtalIdx]->Fill(meanDAC[LRG_DIODE],meanDAC[SM_DIODE]);
        m_dacRatioSumProf->Fill(meanDAC[LRG_DIODE],meanDAC[SM_DIODE]);
      }
    } // for (rng) /* fill histograms */
  } // gcrXtal
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


bool GCRHists::crossedFaceCut( unsigned crossedFaces) const {
  static const unsigned topBtmOnly =
    1<<XFACE_ZTOP | 1<<XFACE_ZBOT;

  return (crossedFaces == topBtmOnly);

}


bool GCRHists::angleCut(const TVector3 &pathVec,
                      DirNum dir) const {
  TVector3 pathDir = pathVec.Unit();

  float xSin = abs(pathDir.x());
  float ySin = abs(pathDir.y());

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

bool GCRHists::posCut(XtalIdx xtalIdx,
                    const TVector3 &entry,
                    const TVector3 &exit) const {
  TwrNum twr = xtalIdx.getTwr();
  DirNum dir = xtalIdx.getLyr().getDir();

  float xtalCtr = xtalLongCtr(dir,twr);

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

void GCRHists::fitHists(CalMPD &calMPD) {
  // PER XTAL LOOP
  for (DiodeNum diode; diode.isValid(); diode++)
    for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
      if (!m_meanDACHists[diode][xtalIdx])
        continue;

      TH1S& hist = *m_meanDACHists[diode][xtalIdx];
      // skip empty histograms
      if (hist.GetEntries() == 0) 
        continue;
    }
}

void GCRHists::loadHists(const string &filename) {
  TFile histFile(filename.c_str(), "READ");

  //-- INIT SUMMARY HISTS  --//
  string histname;

  for (DiodeNum diode; diode.isValid(); diode++) {
    histname = genMeanDACSumHistName(diode);
    m_meanDACSumHist[diode] = CGCUtil::retrieveHist<TH1S>(histFile, histname);
    if (m_meanDACSumHist[diode])
      // move histogram into Global ROOT memory
      // so it is not deleted when input file is closed.
      // this may be a memory leak, i don't think
      // anyone cares.
      m_meanDACSumHist[diode]->SetDirectory(0);
  }

  for (RngNum rng; rng.isValid(); rng++) {
    histname = genMeanADCSumHistName(rng);
    m_meanADCSumHist[rng] = CGCUtil::retrieveHist<TH1S>(histFile, histname);
    if (m_meanADCSumHist[rng])
      m_meanADCSumHist[rng]->SetDirectory(0);
  }

  histname = genDACRatioSumProfName();
  m_dacRatioSumProf = CGCUtil::retrieveHist<TProfile>(histFile, histname);
  if (m_dacRatioSumProf)
    m_dacRatioSumProf->SetDirectory(0);

  for (RngNum rng; rng <= HEX8; rng++) {
    histname = genADCRatioSumProfName(rng);
    m_adcRatioSumProf[rng] = CGCUtil::retrieveHist<TProfile>(histFile, histname);
    if (m_adcRatioSumProf[rng])
      m_adcRatioSumProf[rng]->SetDirectory(0);
  }


  for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
    for (DiodeNum diode; diode.isValid(); diode++) {
      histname = genMeanDACHistName(diode, xtalIdx);
      TH1S *hist = CGCUtil::retrieveHist<TH1S>(histFile, histname);
      if (!hist) continue;
    
      // move histogram into Global ROOT memory
      // so it is not deleted when input file is closed.
      // this may be a memory leak, i don't think
      // anyone cares.
      hist->SetDirectory(0);

      m_meanDACHists[diode][xtalIdx] = hist;
    }

    for (RngNum rng; rng.isValid(); rng++) {
      histname = genMeanADCHistName(rng, xtalIdx);

      TH1S *hist = CGCUtil::retrieveHist<TH1S>(histFile, histname);
      if (!hist) continue;
      hist->SetDirectory(0);

      m_meanADCHists[rng][xtalIdx] = hist;
    }

    histname = genDACRatioProfName(xtalIdx);

    TProfile *hist = CGCUtil::retrieveHist<TProfile>(histFile, histname);
    if (!hist) continue;
    hist->SetDirectory(0);

    m_dacRatioProfs[xtalIdx] = hist;

    for (RngNum rng; rng <= HEX8; rng++) {
      histname = genADCRatioProfName(rng, xtalIdx);
      
      TProfile *hist = CGCUtil::retrieveHist<TProfile>(histFile, histname);
      if (!hist) continue;
      hist->SetDirectory(0);
      
      m_adcRatioProfs[rng][xtalIdx] = hist;
    }
  }

}


unsigned GCRHists::getMinEntries() const {
  unsigned retVal = ULONG_MAX;

  unsigned long sum = 0;
  unsigned n        = 0;
  unsigned maxHits  = 0;

  for (DiodeNum diode; diode.isValid(); diode++)
    for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
      unsigned nEntries = (unsigned)m_meanDACHists[diode][xtalIdx]->GetEntries();
    
      // only count histograms that have been filled 
      // (some histograms will never be filled if we are
      // not using all 16 towers)
      if (nEntries != 0) {
        sum += nEntries;
        n++;
        retVal = min(retVal,nEntries);
        maxHits = max(maxHits,nEntries);
      }
    }


  LogStream::get() << " Channels Detected: "  << n
          << " Avg Hits/channel: " << ((n) ? (double)sum/n : 0)
          << " Max: " << maxHits
          << endl;

  // case where there are no fills at all
  if (retVal == ULONG_MAX)
    return 0;

  return retVal;
}

void GCRHists::summarizeHists(ostream &ostrm) const {
  ostrm << "CHANNEL\tNHITS" << endl;
  for (DiodeNum diode; diode.isValid(); diode++) 
    for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++)
      if (m_meanDACHists[diode][xtalIdx])
        ostrm << genMeanDACHistName(diode, xtalIdx) << "\t"
              << m_meanDACHists[diode][xtalIdx]->GetEntries()
              << endl;
  
}

string GCRHists::genMeanDACHistName(DiodeNum diode, 
                                  XtalIdx xtalIdx) const {
  ostringstream tmp;
  tmp << "meanDAC_" << xtalIdx.val()
      << "_" << diode.val();
  return tmp.str();
}
  
string GCRHists::genMeanADCHistName(RngNum rng, 
                                  XtalIdx xtalIdx) const {
  ostringstream tmp;
  tmp << "meanADC_" << xtalIdx.val()
      << "_" << rng.val();
  return tmp.str();
}
  
string GCRHists::genDACRatioProfName(XtalIdx xtalIdx) const {
  ostringstream tmp;
  tmp << "dacRatio_" << xtalIdx.val();
  return tmp.str();
}
  
string GCRHists::genADCRatioProfName(RngNum rng,
                                   XtalIdx xtalIdx) const {
  ostringstream tmp;
  tmp << "adcRatio_" << xtalIdx.val()
      << "_" << rng.val();
  return tmp.str();
}
  
string GCRHists::genMeanDACSumHistName(DiodeNum diode) const {
  ostringstream tmp;
  tmp << "meanDACSum_" << diode.val();
  return tmp.str();
}

string GCRHists::genMeanADCSumHistName(RngNum rng) const {
  ostringstream tmp;
  tmp << "meanADCSum_" << rng.val();
  return tmp.str();
}

string GCRHists::genDACRatioSumProfName() const {
  ostringstream tmp;
  tmp << "dacRatioSum";
  return tmp.str();
}

string GCRHists::genADCRatioSumProfName(RngNum rng) const {
  ostringstream tmp;
  tmp << "adcRatioSum_" << rng.val();
  return tmp.str();
}

void GCRHists::trimHists() {
  for (DiodeNum diode; diode.isValid(); diode++)
    if (!m_meanDACSumHist[diode]) 
      if (m_meanDACSumHist[diode]->GetEntries() == 0) {
        delete m_meanDACSumHist[diode];
        m_meanDACSumHist[diode] = 0;
      }

    
  for (RngNum rng; rng.isValid(); rng++)
    if (m_meanADCSumHist[rng])
      if (m_meanADCSumHist[rng]->GetEntries() == 0) {
        delete m_meanADCSumHist[rng];
        m_meanADCSumHist[rng] = 0;
      }

  if (m_dacRatioSumProf)
    if (m_dacRatioSumProf->GetEntries() == 0) {
      delete m_dacRatioSumProf;
      m_dacRatioSumProf = 0;
    }


  for (RngNum rng; rng <= HEX8; rng++) 
    if (m_adcRatioSumProf[rng])
      if (m_adcRatioSumProf[rng]->GetEntries() == 0) {
        delete m_adcRatioSumProf[rng];
        m_adcRatioSumProf[rng] = 0;
      }

                              
  //-- INIT PER-XTAL HISTS --//
  for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
    for (DiodeNum diode; diode.isValid(); diode++)
      if (m_meanDACHists[diode][xtalIdx])
        if (m_meanDACHists[diode][xtalIdx]->GetEntries() == 0) {
          delete m_meanDACHists[diode][xtalIdx];
          m_meanDACHists[diode][xtalIdx] = 0;
        }

    for (RngNum rng; rng.isValid(); rng++)
      if (m_meanADCHists[rng][xtalIdx])
        if (m_meanADCHists[rng][xtalIdx]->GetEntries() == 0) {
          delete m_meanADCHists[rng][xtalIdx];
          m_meanADCHists[rng][xtalIdx] = 0;
        }

    if (m_dacRatioProfs[xtalIdx])
      if (m_dacRatioProfs[xtalIdx]->GetEntries() == 0) {
        delete m_dacRatioProfs[xtalIdx];
        m_dacRatioProfs[xtalIdx] = 0;
      }


    for (RngNum rng; rng <= HEX8; rng++)
      if (m_adcRatioProfs[rng][xtalIdx])
        if (m_adcRatioProfs[rng][xtalIdx]->GetEntries() == 0) {
          delete m_adcRatioProfs[rng][xtalIdx];
          m_adcRatioProfs[rng][xtalIdx] = 0;
        }
  } 
}


