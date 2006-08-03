// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/MuonAsym.cxx,v 1.5 2006/07/05 20:35:09 fewtrell Exp $
/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "MuonAsym.h"
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

const float MuonAsym::CSI_LEN      = 326;

MuonAsym::MuonAsym(ostream &ostrm) :
  m_asym(AsymType::N_VALS),
  m_asymErr(AsymType::N_VALS),
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
                                                 N_ASYM_PTS, 
                                                 .5, 
                                                 10.5,
                                                 (int)(100*(asymMax[asymType] -
                                                            asymMin[asymType])),
                                                 asymMin[asymType],
                                                 asymMax[asymType]);
      
    } 
}

void MuonAsym::fillHists(unsigned nEntries,
                         const vector<string> &rootFileList, 
                         const MuonPed &peds,
                         const CIDAC2ADC &dac2adc) {
  initHists();

  RootFileAnalysis rootFile(0, &rootFileList, 0);
  // enable only needed branches in root file
  rootFile.getDigiChain()->SetBranchStatus("*",0);
  rootFile.getDigiChain()->SetBranchStatus("m_calDigiCloneCol");
  rootFile.getDigiChain()->SetBranchStatus("m_summary");

  unsigned nEvents = rootFile.getEntries();
  m_ostrm << __FILE__ << ": Processing: " << nEvents << " events." << endl;

  int  nGoodDirs  = 0; // count total # of events used
  int  nXDirs     = 0;
  int  nYDirs     = 0;
  long nHits      = 0; // count total # of xtals measured
  int  nBadHits   = 0;

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
        int pos;
        vector<XtalIdx> *pHitList, *pHitListOrtho;

        // DIRECTION SPECIFIC SETUP //
        if (dir == X_DIR) {
          if (!hscope.goodXTrack) continue;  // skip this direction if track is bad
          pos = hscope.firstColX;
          pHitList = &hscope.hitListX; // hit list in test direction 
          pHitListOrtho = &hscope.hitListY; // ortho direction
          nXDirs++;
        } else { // Y_DIR
          if (!hscope.goodYTrack) continue; // skip this direction if track is bad
          pos = hscope.firstColY;
          pHitList = &hscope.hitListY; // hit list in test direction 
          pHitListOrtho = &hscope.hitListX; // ortho direction
          nYDirs++;
        }

        // skip extreme ends of xtal, as variance is high.
        if (pos == 0 || pos == 11) continue;
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

void MuonAsym::fitHists() {
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

      for (int i = 0; i < N_ASYM_PTS; i++) {
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

        m_asym[asymType][xtalIdx].push_back(av);
        m_asymErr[asymType][xtalIdx].push_back(rms);

        // evidently ROOT doesn't like reusing the slice 
        // histograms as much as they claim they do.
        slice.Delete();
      }
    }
  }
}

void MuonAsym::writeTXT(const string &filename) const{
  ofstream outfile(filename.c_str());

  if (!outfile.is_open())
    throw runtime_error(string("Unable to open " + filename));

  // PER XTAL LOOP
  for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
    TwrNum twr = xtalIdx.getTwr();
    LyrNum lyr = xtalIdx.getLyr();
    ColNum col = xtalIdx.getCol();
    for (AsymType asymType; asymType.isValid(); asymType++) {
      // skip empty channels
      if (m_asym[asymType][xtalIdx].size() != N_ASYM_PTS)
        continue;

      // per point along curve
      for (int i = 0; i < N_ASYM_PTS; i++) {
        outfile << twr
                << " " << lyr 
                << " " << col
                << " " << asymType.getDiode(POS_FACE)
                << " " << asymType.getDiode(NEG_FACE)
                << " " << m_asym[asymType][xtalIdx][i]
                << " " << m_asymErr[asymType][xtalIdx][i]
                << endl;
      }
    }
  }
}

void MuonAsym::readTXT(const string &filename){
  unsigned short twr, lyr, col, pdiode, ndiode;
  float asym, sig;

  // open file
  ifstream infile(filename.c_str());
  if (!infile.is_open())
    throw runtime_error(string("Unable to open " + filename));

  // loop through each line in file
  while (infile.good()) {
    // get lyr, col (xtalId)
    infile >> twr >> lyr >> col >> pdiode >> ndiode >> asym >> sig;
    
    if (infile.fail()) break; // bad get()

    XtalIdx xtalIdx(twr, lyr,col);
    AsymType asymType(pdiode, ndiode);

    m_asym[asymType][xtalIdx].push_back(asym);
    m_asymErr[asymType][xtalIdx].push_back(sig);
  }
}


void MuonAsym::buildSplines(){
  m_a2pSplines.resize(DiodeNum::N_VALS);
  m_p2aSplines.resize(DiodeNum::N_VALS);

  for (DiodeNum diode; diode.isValid(); diode++) {
    m_a2pSplines[diode].fill(0);
    m_p2aSplines[diode].fill(0);
  }

  // create position (Y-axis) array
  // linearly extrapolate for 1st and last points (+2 points)
  double pos[N_ASYM_PTS+2];
  for (int i = 0; i < N_ASYM_PTS+2; i++) 
    pos[i] = i + 0.5; // (center of the column)
  double asym[N_ASYM_PTS+2];

  // PER XTAL LOOP
  for (DiodeNum diode; diode.isValid(); diode++) {
    AsymType asymType(diode,diode);
    for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
      // skip empty channels
      if (m_asym[asymType][xtalIdx].size() != N_ASYM_PTS)
        continue;
    
      // copy asym vector into middle of array
      vector<float> &asymVec = m_asym[asymType][xtalIdx];
      copy(asymVec.begin(), asymVec.end(), asym+1);

      // extrapolate 1st & last points
      asym[0] = 2*asym[1] - asym[2];
      asym[N_ASYM_PTS+1] = 2*asym[N_ASYM_PTS]-asym[N_ASYM_PTS-1];

      { 
        //generate splinename
        ostringstream name;
        name << "asym2pos_" << xtalIdx.val() << "_" << diode.val();

        // create spline object
        TSpline3 *mySpline= new TSpline3(name.str().c_str(), 
                                         asym, pos, N_ASYM_PTS+2);
        mySpline->SetName(name.str().c_str());

        
        
        m_a2pSplines[diode][xtalIdx] = mySpline;
      }

      // create inverse spline object
      {
        //generate splinename
        ostringstream name;
        name << "pos2asym_" << xtalIdx.val() << "_" << diode.val();

        // create spline object
        TSpline3 *mySpline= new TSpline3(name.str().c_str(), 
                                         pos, asym, N_ASYM_PTS+2);
        mySpline->SetName(name.str().c_str());

//         cout << mySpline->GetName() << " ";
//         for (int i = 0; i < N_ASYM_PTS+2; i++)
//           cout << pos[i] << " " << asym[i] << " ";
//         cout << endl;

        m_p2aSplines[diode][xtalIdx] = mySpline;
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
