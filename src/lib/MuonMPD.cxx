// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/MuonMPD.cxx,v 1.9 2006/09/26 18:57:24 fewtrell Exp $
/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "MuonMPD.h"
#include "RootFileAnalysis.h"
#include "CIDAC2ADC.h"
#include "TwrHodoscope.h"
#include "CalGeom.h"
#include "CalPed.h"
#include "CalAsym.h"
#include "CalMPD.h"

// GLAST INCLUDES
#include "digiRootData/DigiEvent.h"

// EXTLIB INCLUDES
#include "TProfile.h"
#include "TH2S.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TFile.h"
#include "TH1S.h"


// STD INCLUDES
#include <sstream>

using namespace std;
using namespace CalUtil;
using namespace CalGeom;

const float MuonMPD::MUON_ENERGY = 11.2;


MuonMPD::MuonMPD(ostream &ostrm) :
  m_ostrm(ostrm),
  m_dacL2SHists(XtalIdx::N_VALS),
  m_dacL2SSlopeProfs(XtalIdx::N_VALS),
  m_dacLLHists(XtalIdx::N_VALS)
{
}


void MuonMPD::initHists(){
  for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
    string tmp = genHistName("dacLL", xtalIdx);
    m_dacLLHists[xtalIdx] = new TH1S(tmp.c_str(),
                                     tmp.c_str(),
                                     200,0,100);

    tmp = genHistName("dacL2S", xtalIdx);
    m_dacL2SHists[xtalIdx] = new TH1S(tmp.c_str(),
                                      tmp.c_str(),
                                      400,0,.4);


    tmp = genHistName("dacL2S_slope", xtalIdx);
    m_dacL2SSlopeProfs[xtalIdx] = new TProfile(tmp.c_str(),
                                               tmp.c_str(),
                                               N_L2S_PTS,
                                               L2S_MIN_LEDAC,
                                               L2S_MAX_LEDAC);

    
  } 
}

void MuonMPD::fillHists(unsigned nEntries,
                        const vector<string> &rootFileList, 
                        const CalPed &peds,
                        const CalAsym &asym,
                        const CIDAC2ADC &dac2adc){
  initHists();
  
  RootFileAnalysis rootFile(0, &rootFileList, 0);
  // enable only needed branches in root file
  rootFile.getDigiChain()->SetBranchStatus("*",0);
  rootFile.getDigiChain()->SetBranchStatus("m_calDigiCloneCol");
  rootFile.getDigiChain()->SetBranchStatus("m_summary");


  unsigned nEvents = rootFile.getEntries();
  m_ostrm << __FILE__ << ": Processing: " << nEvents << " events." << endl;

  ////////////////////////////////////////////////////
  // INITIALIZE ROOT PLOTTING OBJECTS FOR LINE FITS //
  ////////////////////////////////////////////////////
  // viewHist is used to set scale before drawing TGraph
  TH2S viewHist("viewHist","viewHist",
                8,-0.5,7.5, //X-limits lyr
                12,-0.5,11.5); //Y-limits col
  TCanvas canvas("canvas","event display",800,600);
  viewHist.Draw();
  TGraph graph(4); graph.Draw("*");
  canvas.Update();
  TF1 lineFunc("line","pol1",0,8);

  // SUMMARY COUNTERS //
  unsigned nXEvts  = 0; // count total # of X events used
  unsigned nYEvts  = 0; // count total # of Y events used
  long nXtals = 0; // count total # of xtals measured

  // NUMERIC CONSTANTS
  // converts between lyr/col units & mm
  // real trig is needed for pathlength calculation
  static const float slopeFactor = CalGeom::CELL_HOR_PITCH/CalGeom::CELL_VERT_PITCH;


  // need one hodo scope per tower
  CalVec<TwrNum, TwrHodoscope> hscopes(TwrNum::N_VALS, TwrHodoscope(peds, dac2adc));

  ///////////////////////////////////////////
  // DIGI Event Loop - Fill Twr Hodoscopes //
  ///////////////////////////////////////////
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

    // loop through each 'hit' in one event
    while ((pCalDigi = (CalDigi*)calDigiIter.Next())) {
      const CalDigi &calDigi = *pCalDigi; // use reference to avoid -> syntax
      
      //-- XtalId --//
      idents::CalXtalId id(calDigi.getPackedId()); // get interaction information
      // retrieve tower info
      TwrNum twr = id.getTower();
      
      // add hit to appropriate hodoscope.
      hscopes[twr].addHit(calDigi);

    }
    
    ///////////////////////////////////////////
    // Search Twr Hodoscopes for good events //
    ///////////////////////////////////////////
    for (TwrNum twr; twr.isValid(); twr++) {
      TwrHodoscope &hscope = hscopes[twr];
      
      // summarize the event for each hodoscope
      hscope.summarizeEvent();

      // CHECK BOTH DIRECTIONS FOR USABLE EVENT
      /** Note: 'direction' refers to the direction of xtals which have vertical
          'connect-4' deposits.  For MevPerDAC, the orthogonal hits will be used to
          determine the pathlength for these 4 hits.
      */

      for (DirNum dir; dir.isValid(); dir++) {
        // skip if we don't have a good track
        if (dir == X_DIR && !passCutX(hscope)) continue;
        else if (dir == Y_DIR && !passCutY(hscope)) continue;

        // count used events

        // copy hit lists to local var since
        // i will be removing some hits and
        // the lists may need to be reused for
        // the next oritentation (X vs Y)
        vector<XtalIdx> hitList;
        vector<XtalIdx> hitListOrtho;

        if (dir == X_DIR) {
          nXEvts++;
          hitList = hscope.hitListX;
          hitListOrtho = hscope.hitListY;
        } else {
          nYEvts++;
          hitList = hscope.hitListY;
          hitListOrtho = hscope.hitListX;
        }

        //-- GET HODOSCOPIC TRACK FROM ORTHOGONAL XTALS --//
        graph.Set(hitListOrtho.size());

        // fill in each point val
        for (unsigned i = 0; i < hitListOrtho.size(); i++) {
          XtalIdx xtalIdx = hitListOrtho[i];
          LyrNum lyr = xtalIdx.getLyr();
          ColNum col = xtalIdx.getCol();
          graph.SetPoint(i,lyr,col);
        }

        // fit straight line through graph
        graph.Fit(&lineFunc,"WQN");

        // throw out events which are greater than about 30 deg from vertical
        float lineSlope = lineFunc.GetParameter(1);
        if (abs(lineSlope) > 0.5) continue;


        //-- THROW OUT HITS NEAR END OF XTAL --//
        // loop through each hit in X direction, remove bad xtals
        // bad xtals have energy centroid at col 0 or 11 (-5 or +5 since center of xtal is 0)
        for (unsigned i = 0; i < hitList.size(); i++) {
          XtalIdx xtalIdx = hitList[i];
          LyrNum lyr = xtalIdx.getLyr();

          float hitPos = lineFunc.Eval(lyr); // find column for given lyr

          //throw out event if energy centroid is in column 0 or 11 (3cm from end)
          if (hitPos < 1 || hitPos > 10) {
            hitList.erase(hitList.begin()+i);
            i--;
          }
        }

        // occasionally there will be no good hits!
        if (hitList.size() < 1) continue;

        // improve hodoscopic slope if there are enough good points left 
        // to fit a straight line
        if (hitList.size() > 1) {
          //-- IMPROVE TRACK W/ ASYMMETRY FROM GOOD XTALS --//
          // now that we have eliminated events on the ends of xtals, we can use
          // asymmetry to get a higher precision slope
          graph.Set(hitList.size());  // reset graph size in case we removed any invalid points.
          for (unsigned i = 0; i < hitList.size(); i++) {
            XtalIdx xtalIdx = hitList[i];
            LyrNum lyr = xtalIdx.getLyr();
          
            // calcuate the log ratio = log(POS/NEG)
            float dacP = hscope.dac[tDiodeIdx(xtalIdx.getTXtalIdx(), POS_FACE, LRG_DIODE)];
            float dacN = hscope.dac[tDiodeIdx(xtalIdx.getTXtalIdx(), NEG_FACE, LRG_DIODE)];
            float asymLL = log(dacP/dacN);

            // get new position from asym
            float hitPos = asym.asym2pos(xtalIdx, LRG_DIODE, asymLL);

            graph.SetPoint(i,lyr,hitPos);
          }

          graph.Fit(&lineFunc,"WQN");
          lineSlope = lineFunc.GetParameter(1);
        }


        //-- Pathlength Correction --//
        //slope = rise/run = dy/dx = colPos/lyrNum
        float tan = lineSlope*slopeFactor;
        float sec = sqrt(1 + tan*tan); //sec proportional to hyp which is pathlen.

        // poulate histograms & apply pathlength correction
        unsigned nHits = hitList.size();
        for (unsigned i = 0; i < nHits; i++) {
          XtalIdx xtalIdx = hitList[i];

          // calculate meanDAC for each diode size.
          CalArray<DiodeNum, float> meanDAC;
          for (DiodeNum diode; diode.isValid(); diode++) {
            meanDAC[diode] = sqrt(hscope.dac[tDiodeIdx(xtalIdx.getTXtalIdx(), POS_FACE, diode)] *
                                  hscope.dac[tDiodeIdx(xtalIdx.getTXtalIdx(), NEG_FACE, diode)]);

            meanDAC[diode] /= sec;
          }
        
          // Load meanDAC Histogram
          m_dacLLHists[xtalIdx]->Fill(meanDAC[LRG_DIODE]);

          // load dacL2S profile
          m_dacL2SHists[xtalIdx]->Fill(meanDAC[SM_DIODE]/meanDAC[LRG_DIODE]);
          // load dacL2S profile
          m_dacL2SSlopeProfs[xtalIdx]->Fill(meanDAC[LRG_DIODE],meanDAC[SM_DIODE]);

          nXtals++;      
        }
      }
    }
  }
}

void MuonMPD::fitHists(CalMPD &calMPD) {
  ////////////////////////////////////////////////////
  // INITIALIZE ROOT PLOTTING OBJECTS FOR LINE FITS //
  ////////////////////////////////////////////////////
  // viewHist is used to set scale before drawing TGraph
  TH2S viewHist("viewHist","viewHist",
                N_L2S_PTS, 0, L2S_MAX_LEDAC, // X-LIMITS LRG
                N_L2S_PTS, 0, L2S_MAX_LEDAC); // Y-LIMITS SM
  TCanvas canvas("canvas","event display",800,600);
  viewHist.Draw();
  TGraph graph(N_L2S_PTS); 
  graph.Draw("*");
  canvas.Update();
  TF1 lineFunc("line","pol1", 
               L2S_MIN_LEDAC, 
               L2S_MAX_LEDAC);

  // PER XTAL LOOP
  for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
    if (!m_dacLLHists[xtalIdx])
      continue;
    // retrieve Lrg diode DAC histogram
    if (!m_dacLLHists[xtalIdx])
      continue;
    TH1S& histLL = *m_dacLLHists[xtalIdx];
    // skip empty histograms
    if (histLL.GetEntries() == 0) 
      continue;

    ///////////////////////////////////
    //-- MeV Per Dac (Lrg Diode) --//
    ///////////////////////////////////
    
    // LANDAU fit for muon peak (limit outliers by n*err)
    float ave = histLL.GetMean();
    float err = histLL.GetRMS();
    histLL.Fit("landau", "Q", "", ave-2*err, ave+3*err);
    float mean  = (histLL.GetFunction("landau"))->GetParameter(1);
    float sigma = (histLL.GetFunction("landau"))->GetParameter(2);

    float mpdLrg = MUON_ENERGY/mean;
    calMPD.setMPD(xtalIdx, LRG_DIODE, mpdLrg);

    // keep sigma proportional to extrapolated val
    float mpdErrLrg = mpdLrg * sigma/mean;
    calMPD.setMPDErr(xtalIdx, LRG_DIODE, mpdErrLrg); 
    
    ////////////////////
    //-- (Sm Diode) --//
    ////////////////////
        
    // LRG 2 SM Ratio
    TH1S &histL2S = *m_dacL2SHists[xtalIdx]; // get profile

    // trim outliers - 3 times cut out anything outside 3 sigma
    for (unsigned short iter = 0; iter < 3; iter++) {
      // get current mean & RMS
      float av  = histL2S.GetMean(); 
      float rms = histL2S.GetRMS();
      
      // trim new histogram limits
      histL2S.SetAxisRange(av - 3*rms, av + 3*rms);
    }

    // fit straight line to get mean ratio
    histL2S.Fit("gaus","Q");
    // mean ratio of smDac/lrgDac
    float sm2lrg = ((TF1&)*histL2S.GetFunction("gaus")).GetParameter(1);
    float s2lsig = ((TF1&)*histL2S.GetFunction("gaus")).GetParameter(2);
    
    //-- NOTES:
    // MPDLrg     = MeV/LrgDAC
    // sm2lrg  = SmDAC/LrgDAC
    // MPDSm     = MeV/SmDAC = (MeV/LrgDAC)*(LrgDAC/SmDAC) 
    //              = MPDLrg/sm2lrg
    
    float mpdSm = mpdLrg/sm2lrg;
    calMPD.setMPD(xtalIdx, SM_DIODE, mpdSm);
    
    //-- Propogate errors
    // in order to combine slope & MPD error for final error
    // I need the relative error for both values - so sayeth sasha
    float relLineErr = s2lsig/sm2lrg;
    float relMPDErr  = mpdErrLrg/mpdLrg;

    float mpdErrSm = mpdSm*
      sqrt(relLineErr*relLineErr + relMPDErr*relMPDErr);
    calMPD.setMPDErr(xtalIdx, SM_DIODE, mpdErrSm);

    ////////////////////
    //-- L2S Slope  --//
    ////////////////////

    // LRG 2 SM Ratio
    TProfile& p = *m_dacL2SSlopeProfs[xtalIdx]; // get profile

    // Fill scatter graph w/ smDAC vs lrgDAC points
    unsigned nPts = 0;
    graph.Set(nPts); // start w/ empty graph
    for (unsigned i = 0; i < N_L2S_PTS; i++) {
      // only insert a bin if it has entries
      if (!(p.GetBinEntries(i+1) > 0)) continue; // bins #'d from 1
      nPts++;

      // retrieve sm & lrg dac vals
      float smDAC = p.GetBinContent(i+1);
      float lrgDAC = p.GetBinCenter(i+1);

      // update graphsize & set point
      graph.Set(nPts);
      graph.SetPoint(nPts-1,lrgDAC,smDAC);
    }

    // bail if for some reason we didn't get any points
    if (!nPts) {
      m_ostrm << __FILE__  << ":"     << __LINE__ << " "
              << "Unable to find sm diode MPD for xtal=" << xtalIdx.val()
              << " due to empty histogram." << endl;
      continue;
    }

    // fit straight line to get mean ratio
    graph.Fit(&lineFunc,"WQN");
  }
}

void MuonMPD::loadHists(const string &filename) {
  TFile histFile(filename.c_str(), "READ");

  for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {

    //-- DAC_LL HISTOGRAMS --//
    string histname = genHistName("dacLL", xtalIdx);
    TH1S *hist_LL = CGCUtil::retrieveHist<TH1S>(histFile, histname);
    if (!hist_LL) continue;
    

    // move histogram into Global ROOT memory
    // so it is not deleted when input file is closed.
    // this may be a memory leak, i don't think
    // anyone cares.
    hist_LL->SetDirectory(0);

    m_dacLLHists[xtalIdx] = hist_LL;


    //-- DAC_L2S HISTOGRAMS --//
    histname = genHistName("dacL2S", xtalIdx);
    TH1S *hist_L2S = CGCUtil::retrieveHist<TH1S>(histFile, histname);
    if (!hist_L2S) continue;

    hist_L2S->SetDirectory(0);

    m_dacL2SHists[xtalIdx] = hist_L2S;

    //-- DAC_L2S HISTOGRAMS --//
    histname = genHistName("dacL2S_slope", xtalIdx);
    TProfile *hist_L2S_slope = CGCUtil::retrieveHist<TProfile>(histFile, histname);
    if (!hist_L2S_slope) continue;

    hist_L2S_slope->SetDirectory(0);

    m_dacL2SSlopeProfs[xtalIdx] = hist_L2S_slope;

  }

}

string MuonMPD::genHistName(const string &type,
                            XtalIdx xtalIdx) {
  ostringstream tmp;
  tmp <<  type 
      << "_" << xtalIdx.val();
  return tmp.str();
}

unsigned MuonMPD::getMinEntries() {
  unsigned retVal = ULONG_MAX;

  unsigned long sum = 0;
  unsigned n        = 0;
  unsigned maxHits  = 0;

  for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
    unsigned nEntries = (unsigned)m_dacLLHists[xtalIdx]->GetEntries();
    
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

void MuonMPD::writeADC2NRG(const string &filename,
                           const CalAsym &asym,
                           const CIDAC2ADC &dac2adc,
                           const CalMPD &calMPD) {
  ofstream outfile(filename.c_str());

  if (!outfile.is_open())
    throw runtime_error(string("Unable to open " + filename));

  for (RngIdx rngIdx; rngIdx.isValid(); rngIdx++) {
    TwrNum twr = rngIdx.getTwr();
    LyrNum lyr = rngIdx.getLyr();
    ColNum col = rngIdx.getCol();
    FaceNum face = rngIdx.getFace();
    RngNum rng = rngIdx.getRng();
    DiodeNum diode = rng.getDiode();
    XtalIdx xtalIdx = rngIdx.getXtalIdx();
    float mpd = calMPD.getMPD(xtalIdx, diode);

    if (mpd == CalMPD::INVALID_MPD)
      continue;

    // get asymmetry measured at ctr of xtal (gives us relative gain between xtal faces)

    // 0.25 would normally be 0.5, but it is applied equally to both sides
    // so we split it in half.
    float asym_ctr = 0.25*asym.pos2asym(xtalIdx,
                                        diode,
                                        6.0); // is center of xtal, coords range from 0-12
    asym_ctr = exp(asym_ctr);

    float dac = MUON_ENERGY/mpd;

    dac = (face == POS_FACE) ?
      dac * asym_ctr :
      dac / asym_ctr;

    float adc     = dac2adc.dac2adc(rngIdx, dac*asym_ctr);
    float adc2nrg = MUON_ENERGY/adc;
    outfile << twr
            << " " << lyr 
            << " " << col
            << " " << face.val()
            << " " << rng.val()
            << " " << adc2nrg
            << " " << 0 // error calc
            << endl;
  }
}

bool MuonMPD::passCutX(const TwrHodoscope &hscope) {
  // max 2 hits on any layer
  if (hscope.maxPerLyr > 2) 
    return false;

  // need vertical connect 4 in X dir
  if (hscope.nLyrsX != 4 || hscope.nColsX != 1) 
    return false;
  
  // need at least 2 points to get an orthogonal track
  if (hscope.nLyrsY < 2) return false;

  return true;
}

bool MuonMPD::passCutY(const TwrHodoscope &hscope) {
  // max 2 hits on any layer
  if (hscope.maxPerLyr > 2) 
    return false;

  // need vertical connect 4 in Y dir
  if (hscope.nLyrsY != 4 || hscope.nColsY != 1) 
    return false;
  
  // need at least 2 points to get an orthogonal track
  if (hscope.nLyrsX < 2) return false;

  return true;
}
