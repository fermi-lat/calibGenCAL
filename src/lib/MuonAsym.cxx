// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/MuonAsym.cxx,v 1.6 2006/08/03 13:06:48 fewtrell Exp $
/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "MuonAsym.h"
#include "CalAsym.h"
#include "RootFileAnalysis.h"
#include "TwrHodoscope.h"
#include "CGCUtil.h"

// GLAST INCLUDES

// EXTLIB INCLUDES
#include "TKey.h"

// STD INCLUDES
#include <sstream>

using namespace std;
using namespace CGCUtil;

MuonAsym::MuonAsym(ostream &ostrm) :
  m_ostrm(ostrm)
{
}


void MuonAsym::initHists(){
  m_histograms.resize(AsymType::N_VALS);

  // fill in min & max histogram levels.
  CalArray<AsymType, float> asymMin;
  CalArray<AsymType, float> asymMax;

  asymMin[ASYM_LL] = -2;
  asymMax[ASYM_LL] = 2;
  asymMin[ASYM_LS] = -1;
  asymMax[ASYM_LS] = 7;
  asymMin[ASYM_SL] = -7;
  asymMax[ASYM_SL] = 1;
  asymMin[ASYM_SS] = -2;
  asymMax[ASYM_SS] = 2;
    
  for (AsymType asymType; asymType.isValid(); asymType++)
    // PER XTAL LOOP
    for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
      string tmp = genHistName(asymType, xtalIdx);
      // columns are #'d 0-11, hist contains 1-10. 
      // .5 & 10.5 limit puts 1-10 at center of bins
      m_histograms[asymType][xtalIdx] = new TH2S(tmp.c_str(),
                                                 tmp.c_str(),
                                                 CalAsym::N_ASYM_PTS, 
                                                 .5, 
                                                 10.5,
                                                 (unsigned)(100*(asymMax[asymType] -
                                                            asymMin[asymType])),
                                                 asymMin[asymType],
                                                 asymMax[asymType]);
      
    } 
}

void MuonAsym::fillHists(unsigned nEntries,
                         const vector<string> &rootFileList, 
                         const CalPed &peds,
                         const CIDAC2ADC &dac2adc) {
  initHists();

  RootFileAnalysis rootFile(0, &rootFileList, 0);
  // enable only needed branches in root file
  rootFile.getDigiChain()->SetBranchStatus("*",0);
  rootFile.getDigiChain()->SetBranchStatus("m_calDigiCloneCol");
  rootFile.getDigiChain()->SetBranchStatus("m_summary");

  unsigned nEvents = rootFile.getEntries();
  m_ostrm << __FILE__ << ": Processing: " << nEvents << " events." << endl;

  unsigned  nGoodDirs  = 0; // count total # of events used
  unsigned  nXDirs     = 0;
  unsigned  nYDirs     = 0;
  long nHits      = 0; // count total # of xtals measured
  unsigned  nBadHits   = 0;

  // need one hodo scope per tower
  CalVec<TwrNum, TwrHodoscope> hscopes(TwrNum::N_VALS, TwrHodoscope(peds, dac2adc));

  // Basic digi-event loop
  
  for (unsigned eventNum = 0; eventNum < nEvents; eventNum++) {
    if (eventNum % 10000 == 0) {
      // quit if we have enough entries in each histogram
      unsigned currentMin = getMinEntries();
      if (currentMin >= nEntries) break;

      m_ostrm << "Event: " << eventNum 
              << " min entries per histogram: " << currentMin
              << endl;
      m_ostrm.flush();
    }

    if (!rootFile.getEvent(eventNum)) {
      m_ostrm << "Warning, event " << eventNum << " not read." << endl;
      continue;
    }

    DigiEvent *digiEvent = rootFile.getDigiEvent();
    if (!digiEvent) {
      m_ostrm << __FILE__ << ": Unable to read DigiEvent " << eventNum  << endl;
      continue;
    }

    // check that we are in 4 range mode
    EventSummaryData &summary = digiEvent->getEventSummaryData();
    if (!summary.readout4())
      continue;

    const TClonesArray* calDigiCol = digiEvent->getCalDigiCol();
    if (!calDigiCol) {
      m_ostrm << "no calDigiCol found for event#" << eventNum << endl;
      continue;
    }

    TIter calDigiIter(calDigiCol);
    const CalDigi *pCalDigi = 0;

    // clear all hodoscopes
    for (TwrNum twr; twr.isValid(); twr++)
      hscopes[twr].clear();

    //loop through each 'hit' in one event
    while ((pCalDigi = (CalDigi*)calDigiIter.Next())) {
      const CalDigi &calDigi = *pCalDigi; // use reference to avoid -> syntax
      
      //-- XtalId --//
      idents::CalXtalId id(calDigi.getPackedId()); // get interaction information
      // retrieve tower info
      TwrNum twr = id.getTower();
      
      // add hit to appropriate hodoscope.
      hscopes[twr].addHit(calDigi);

    }
    
    // process each tower for possible good muon event
    for (TwrNum twr; twr.isValid(); twr++) {
      TwrHodoscope &hscope = hscopes[twr];
      
      // summarize the event for each hodoscope
      hscope.summarizeEvent();
      
      for (DirNum dir; dir.isValid(); dir++) {
        unsigned short pos;
        vector<XtalIdx> *pHitList, *pHitListOrtho;

        // DIRECTION SPECIFIC SETUP //
        /** Note: 'direction' refers to the direction of xtals which have vertical
            'connect-4' deposits.  For asymmetry, we use this vertical column
            to calibrate the signal in the orthogonal crystals.  
        */
        if (dir == X_DIR) {
          if (!passCutX(hscope)) continue;  // skip this direction if track is bad
          pos = hscope.firstColX;
          pHitList = &hscope.hitListX; // hit list in test direction 
          pHitListOrtho = &hscope.hitListY; // ortho direction
          nXDirs++;
        } else { // Y_DIR
          if (!passCutY(hscope)) continue; // skip this direction if track is bad
          pos = hscope.firstColY;
          pHitList = &hscope.hitListY; // hit list in test direction 
          pHitListOrtho = &hscope.hitListX; // ortho direction
          nYDirs++;
        }

        nGoodDirs++;
        
        // use references to avoid -> notation
        vector<XtalIdx> &hitListOrtho = *pHitListOrtho; 
        
        // loop through each orthogonal hit
        for (unsigned i = 0; i < hitListOrtho.size(); i++) {
          XtalIdx xtalIdx = hitListOrtho[i];
          
          // calcuate the 4 log ratios = log(POS/NEG)
          for (AsymType asymType; asymType.isValid(); asymType++) {
            float dacP = hscope.dac[tDiodeIdx(xtalIdx.getTXtalIdx(), POS_FACE, asymType.getDiode(POS_FACE))];
            float dacN = hscope.dac[tDiodeIdx(xtalIdx.getTXtalIdx(), NEG_FACE, asymType.getDiode(NEG_FACE))];
            float asym = log(dacP/dacN);

            m_histograms[asymType][xtalIdx]->Fill(pos, asym);
          }
          //           m_ostrm << "HIT: " << eventNum 
          //                   << " " << xtalIdx.val() 
          //                   << " " << m_histograms[ASYM_SS][xtalIdx]->GetEntries()
          //                   << endl;
          
          
          nHits++;
        } // per hit loop
      } // per direction loop
    } // per tower loop
  } // per event loop
  
  m_ostrm << "Asymmetry histograms filled nEvents=" << nGoodDirs
          << " nXDirs="               << nXDirs
          << " nYDirs="               << nYDirs << endl;
  m_ostrm << " nHits measured="       << nHits
          << " Bad hits="             << nBadHits
          << endl;

}

void MuonAsym::summarizeHists(ostream &ostrm) {
  ostrm << "XTAL\tNHITS" << endl;
  for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++)
    if (m_histograms[ASYM_SS][xtalIdx]->GetEntries() > 0)
      ostrm << xtalIdx.val() << "\t"
            << m_histograms[ASYM_SS][xtalIdx]->GetEntries()
            << endl;
  
}

void MuonAsym::fitHists(CalAsym &calAsym) {
  for (AsymType asymType; asymType.isValid(); asymType++) {
    
    // PER XTAL LOOP
    for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
      // skip non existant hists
      if (!m_histograms[asymType][xtalIdx])
        continue;

      // loop through all N_ASYM_PTS bins in asymmetry profile

      // skip empty histograms
      TH2S &h = *(m_histograms[asymType][xtalIdx]);
      if (h.GetEntries() == 0) 
        continue;

      for (unsigned short i = 0; i < CalAsym::N_ASYM_PTS; i++) {
        // get slice of 2D histogram for each X bin
        // HISTOGRAM BINS START AT 1 NOT ZERO! (hence 'i+1') 
        TH1D &slice = *(h.ProjectionY("slice", i+1,i+1));

        // point local references to output values
        float av;
        float rms;

        // trim outliers - 3 times cut out anything outside 3 sigma
        for (unsigned short iter = 0; iter < 3; iter++) {
          // get current mean & RMS
          av  = slice.GetMean(); 
          rms = slice.GetRMS();
        
          // trim new histogram limits
          slice.SetAxisRange(av - 3*rms, av + 3*rms);
        }

        // update new mean & sigma
        av = slice.GetMean(); rms = slice.GetRMS();

        calAsym.getPtsAsym(xtalIdx,asymType).push_back(av);
        calAsym.getPtsErr(xtalIdx,asymType).push_back(rms);

        // evidently ROOT doesn't like reusing the slice 
        // histograms as much as they claim they do.
        slice.Delete();
      }
    }
  }
}

void MuonAsym::loadHists(const string &filename) {
  m_histograms.resize(AsymType::N_VALS);
  TFile histFile(filename.c_str(), "READ");

  for (AsymType asymType; asymType.isValid(); asymType++)
    // PER XTAL LOOP
    for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
      string histname = genHistName(asymType, xtalIdx);
      TH2S *hist_ptr = retrieveHist<TH2S>(histFile, histname);
      if (!hist_ptr) 
        continue;

      hist_ptr->SetDirectory(0);

      m_histograms[asymType][xtalIdx] = hist_ptr;
    }

}

string MuonAsym::genHistName(AsymType asymType, XtalIdx xtalIdx) {
  ostringstream tmp;
  tmp <<  "asym" 
      << asymType.getDiode(POS_FACE).val() 
      << asymType.getDiode(NEG_FACE).val() 
      << "_" << xtalIdx.val();
  return tmp.str();
}


unsigned MuonAsym::getMinEntries() {
  unsigned retVal = ULONG_MAX;

  unsigned long sum = 0;
  unsigned n = 0;
  unsigned maxHits = 0;

  for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
    unsigned nEntries = (unsigned)m_histograms[ASYM_SS][xtalIdx]->GetEntries();

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

  m_ostrm << " Channels Detected: "  << n
          << " Avg Hits/channel: " << ((n) ? (double)sum/n : 0)
          << " Max: " << maxHits
          << endl;

  // case where there are no fills at all
  if (retVal == ULONG_MAX)
    return 0;

  return retVal;
}

bool MuonAsym::passCutX(const TwrHodoscope &hscope) {
  // max 2 hits on any layer
  if (hscope.maxPerLyr > 2) 
    return false;

  // need vertical connect 4 in X dir
  if (hscope.nLyrsX != 4 || hscope.nColsX != 1) 
    return false;

  // skip extreme ends of xtal, as variance is high.
  if (hscope.firstColX == 0 || hscope.firstColX == 11) 
    return false;

  // need at least one hit in ortho direction,
  // otherwise there is nothing to calibrate
  if (!hscope.nLyrsY) return false;

  return true;
}

bool MuonAsym::passCutY(const TwrHodoscope &hscope) {
  // max 2 hits on any layer
  if (hscope.maxPerLyr > 2) 
    return false;

  // need vertical connect 4 in Y dir
  if (hscope.nLyrsY != 4 || hscope.nColsY != 1) 
    return false;

  // skip extreme ends of xtal, as variance is high.
  if (hscope.firstColY == 0 || hscope.firstColY == 11) 
    return false;

  // need at least one hit in ortho direction,
  // otherwise there is nothing to calibrate
  if (!hscope.nLyrsX) return false;

  return true;
}
