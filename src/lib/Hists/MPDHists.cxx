// $Header: //

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "MPDHists.h"
#include "src/lib/Util/LangauFun.h"
#include "src/lib/Util/CGCUtil.h"
#include "src/lib/Util/ROOTUtil.h"
#include "src/lib/Specs/CalResponse.h"

// GLAST INCLUDES

// EXTLIB INCLUDES
#include "TH1S.h"
#include "TH1I.h"
#include "TProfile.h"
#include "TF1.h"
#include "TGraph.h"
#include "TStyle.h"

// STD INCLUDES
#include <limits.h>
#include <string>
#include <sstream>
#include <stdexcept>
#include <map>

namespace calibGenCAL {

  using namespace std;
  using namespace CalUtil;

  /// map fitting method enum to ROOT TF1 fitting function
  typedef std::map<MPDHists::FitMethods::FitMethod, TF1 *> FitFuncMap;

  /// map fitting method enum to ROOT TF1 fitting function
  static FitFuncMap m_fitFuncMap;

  void fillFitFuncMap() {
    m_fitFuncMap[MPDHists::FitMethods::LANDAU] = new TF1("mpdLandau", "landau");;
    m_fitFuncMap[MPDHists::FitMethods::LANGAU] = &(LangauFun::getLangauDAC());
  }

  MPDHists::MPDHists(const FitMethods::FitMethod fitMethod) :
    m_dacLLSumHist(0),
    m_perLyr(0),
    m_perTwr(0),
    m_perXtal(0),
    m_fitMethod(fitMethod)
  {
    if (m_fitFuncMap.empty())
      fillFitFuncMap();

    m_fitFunc = m_fitFuncMap[m_fitMethod];
  }

  void MPDHists::initHists() {
    string histname;


    histname       = "dacLLSum";
    m_dacLLSumHist = new TH1I(histname.c_str(),
                              histname.c_str(),
                              400, 0, 100);

    for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
      histname = genHistName("dacL2S", xtalIdx.toStr());
      m_dacL2SHists[xtalIdx] = new TH1S(histname.c_str(),
                                        histname.c_str(),
                                        400, 0, .4);

      histname = genHistName("dacL2S_slope", xtalIdx.toStr());
      m_dacL2SSlopeProfs[xtalIdx] = new TProfile(histname.c_str(),
                                                 histname.c_str(),
                                                 N_L2S_PTS,
                                                 L2S_MIN_LEDAC,
                                                 L2S_MAX_LEDAC);

      histname = genHistName("dacLL", xtalIdx.toStr());
      m_dacLLHists[xtalIdx] = new TH1S(histname.c_str(),
                                       histname.c_str(),
                                       200, 0, 100);
    }

    m_perLyr  = new TH1I("hitsPerLyr",
                         "hitsPerLyr",
                         LyrNum::N_VALS,
                         0, LyrNum::N_VALS);

    m_perTwr  = new TH1I("hitsPerTwr",
                         "hitsPerTwr",
                         TwrNum::N_VALS,
                         0, TwrNum::N_VALS);

    m_perXtal = new TH1S("hitsPerXtal",
                         "hitsPerXtal",
                         XtalIdx::N_VALS,
                         0, XtalIdx::N_VALS);

    for (TwrNum twr; twr.isValid(); twr++) {
      histname = genHistName("hitsPerTwrLyr",
                             twr.val());
      m_perTwrLyr[twr] = new TH1I(histname.c_str(),
                                  histname.c_str(),
                                  LyrNum::N_VALS,
                                  0, LyrNum::N_VALS);

      histname = genHistName("hitsPerTwrCol",
                             twr.val() );
      m_perTwrCol[twr] = new TH1I(histname.c_str(),
                                  histname.c_str(),
                                  ColNum::N_VALS,
                                  0, ColNum::N_VALS);
    }
  }

  void MPDHists::fitHists(CalMPD &calMPD) {
    TGraph graph;

    //LogStrm::get() << "Muon Peak Fit Results: " << endl;
    //LogStrm::get() << " SCALE\tXTAL\tMPV\tLanWid\tGauWid\tTotalWid\tBckgnd" << endl;

    // PER XTAL LOOP
    for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
      //for (XtalIdx xtalIdx; xtalIdx.val() < 10; xtalIdx++) {
      if (!m_dacLLHists[xtalIdx])
        continue;

      TH1S & histLL = *m_dacLLHists[xtalIdx];
      // skip empty histograms
      if (histLL.GetEntries() == 0)
        continue;

      ///////////////////////////////////
      //-- MeV Per Dac (Lrg Diode) --//
      ///////////////////////////////////

      float mpv, width;
      fitChannel(histLL, mpv, width);
      LogStrm::get() << xtalIdx.val() << " "
                       << mpv << " "
                       << width << " "
                       << endl;

      // create histogram of residual after fit
      //createResidHist(histLL);

      const float mpdLrg    = CalResponse::CsIMuonPeak/mpv;
      calMPD.setMPD(xtalIdx, LRG_DIODE, mpdLrg);

      // keep width proportional to new scale
      const float mpdErrLrg = mpdLrg * width/mpv;
      calMPD.setMPDErr(xtalIdx, LRG_DIODE, mpdErrLrg);

      ////////////////////
      //-- (Sm Diode) --//
      ////////////////////

      // LRG 2 SM Ratio
      TH1S *histL2S = m_dacL2SHists[xtalIdx];
      // skip if we have no small diode info for this channel
      if (!histL2S)
        continue;
      if (!histL2S->GetEntries())
        continue;
    

      // trim outliers - 3 times cut out anything outside 3 sigma
      for (unsigned short iter = 0; iter < 3; iter++) {
        // get current mean & RMS
        const float av  = histL2S->GetMean();
        const float rms = histL2S->GetRMS();

        // trim new histogram limits
        histL2S->SetAxisRange(av - 3*rms, av + 3*rms);
      }

      // fit gaussian to get mean ratio
      histL2S->Fit("gaus", "Q");
      // mean ratio of smDac/lrgDac
      float sm2lrg = ((TF1&)*histL2S->GetFunction("gaus")).GetParameter(1);
      float s2lsig = ((TF1&)*histL2S->GetFunction("gaus")).GetParameter(2);

      //-- NOTES:
      // MPDLrg     = MeV/LrgDAC
      // sm2lrg  = SmDAC/LrgDAC
      // MPDSm     = MeV/SmDAC = (MeV/LrgDAC)*(LrgDAC/SmDAC)
      //              = MPDLrg/sm2lrg

      const float mpdSm = mpdLrg/sm2lrg;
      calMPD.setMPD(xtalIdx, SM_DIODE, mpdSm);

      //-- Propogate errors
      // in order to combine slope & MPD error for final error
      // I need the relative error for both values - so sayeth sasha
      const float relLineErr = s2lsig/sm2lrg;
      const float relMPDErr  = mpdErrLrg/mpdLrg;

      const float mpdErrSm   = mpdSm *
        sqrt(relLineErr *relLineErr + relMPDErr *relMPDErr);

      calMPD.setMPDErr(xtalIdx, SM_DIODE, mpdErrSm);

      ////////////////////
      //-- L2S Slope  --//
      ////////////////////

      // LRG 2 SM Ratio
      TProfile & p = *m_dacL2SSlopeProfs[xtalIdx];    // get profile

      // Fill scatter graph w/ smDAC vs lrgDAC points
      unsigned nPts = 0;
      graph.Set(nPts);                                // start w/ empty graph
      for (unsigned i = 0; i < N_L2S_PTS; i++) {
        // only insert a bin if it has entries
        if (!(p.GetBinEntries(i+1) > 0)) continue;    // bins #'d from 1
        nPts++;

        // retrieve sm & lrg dac vals
        const float smDAC  = p.GetBinContent(i+1);
        const float lrgDAC = p.GetBinCenter(i+1);

        // update graphsize & set point
        graph.Set(nPts);
        graph.SetPoint(nPts-1, lrgDAC, smDAC);
      }

      // bail if for some reason we didn't get any points
      if (nPts < 2) {
        LogStrm::get() << __FILE__  << ":"     << __LINE__ << " "
                         << "Not enough points to find sm diode MPD slope for xtal="
                         << xtalIdx.val() << endl;
        continue;
      }

      // fit straight line to get mean ratio
      graph.Fit("pol1", "WQN");
    }
  }

  void MPDHists::fitChannel(TH1 &hist,
                            float &mpv,
                            float &width) {

    /// MPV
    m_fitFunc->SetParameter(1, 30);
    /// landau area
    m_fitFunc->SetParameter(2, hist.GetEntries()*hist.GetBinWidth(1));
    /// gaussian width ( currently fixed)
    //m_fitFunc->SetParameter(3, 0.6);
    /// background height 
    m_fitFunc->SetParameter(4, 0.0);
    const int fitResult = hist.Fit(m_fitFunc, "QL");
    if (fitResult !=0)
      LogStrm::get() << "MPD ROOT fitting error code: " << fitResult
                     << " " << hist.GetName() << endl;
    mpv = m_fitFunc->GetParameter(1);

    switch (m_fitMethod) {
    case FitMethods::LANDAU:
      width  = m_fitFunc->GetParameter(2);
      break;

    case FitMethods::LANGAU:
      width  = sqrt(pow(m_fitFunc->GetParameter(0), 2) +
                    pow(m_fitFunc->GetParameter(3), 2));
      // width is calc'd as fraction of mpv
      // i want to save it in dac units.
      width *= mpv;
      break;

    default:
      throw invalid_argument("invalid fit_method");
    }
  }

  void MPDHists::loadHists(const TDirectory &readDir) {
    string histname;

    for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
      //-- DAC_LL HISTOGRAMS --//
      histname = genHistName("dacLL", xtalIdx.toStr());
      TH1S *hist_LL = retrieveROOTObj < TH1S > (readDir, histname);
      if (!hist_LL) continue;

      m_dacLLHists[xtalIdx] = hist_LL;

      //-- DAC_L2S HISTOGRAMS --//
      histname = genHistName("dacL2S", xtalIdx.toStr());
      TH1S *hist_L2S = retrieveROOTObj < TH1S > (readDir, histname);
      if (!hist_L2S) continue;

      m_dacL2SHists[xtalIdx] = hist_L2S;

      //-- DAC_L2S HISTOGRAMS --//
      histname = genHistName("dacL2S_slope", xtalIdx.toStr());
      TProfile *hist_L2S_slope = retrieveROOTObj < TProfile > (readDir, histname);
      if (!hist_L2S_slope) continue;

      m_dacL2SSlopeProfs[xtalIdx] = hist_L2S_slope;
    }
  }

  unsigned MPDHists::getMinEntries() const {
    unsigned      retVal   = ULONG_MAX;

    unsigned long sum      = 0;
    unsigned      n        = 0;
    unsigned      maxHits  = 0;


    for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
      const unsigned nEntries = (unsigned)m_dacLLHists[xtalIdx]->GetEntries();

      // only count histograms that have been filled
      // (some histograms will never be filled if we are
      // not using all 16 towers)
      if (nEntries != 0) {
        sum    += nEntries;
        n++;
        retVal  = min(retVal, nEntries);
        maxHits = max(maxHits, nEntries);
      }
    }

    LogStrm::get() << " Channels Detected: "  << n
                     << " Avg Hits/channel: " << ((n) ? (double)sum/n : 0)
                     << " Max: " << maxHits
                     << endl;

    // case where there are no fills at all
    if (retVal == ULONG_MAX)
      return 0;

    return retVal;
  }

  void MPDHists::trimHists() {
    for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
      if (m_dacLLHists[xtalIdx])
        if (!m_dacLLHists[xtalIdx]->GetEntries()) {
          delete m_dacLLHists[xtalIdx];
          m_dacLLHists[xtalIdx] = 0;
        }

      if (m_dacL2SHists[xtalIdx])
        if (!m_dacL2SHists[xtalIdx]->GetEntries()) {
          delete m_dacL2SHists[xtalIdx];
          m_dacL2SHists[xtalIdx] = 0;
        }

      if (m_dacL2SSlopeProfs[xtalIdx])
        if (!m_dacL2SSlopeProfs[xtalIdx]->GetEntries()) {
          delete m_dacL2SSlopeProfs[xtalIdx];
          m_dacL2SSlopeProfs[xtalIdx] = 0;
        }
    }
  }

  void MPDHists::fillDacLL(CalUtil::XtalIdx xtalIdx,
                           float dac) {
    m_dacLLHists[xtalIdx]->Fill(dac);
    m_dacLLSumHist->Fill(dac);

    //-- alg statistics histograms
    const LyrNum lyr(xtalIdx.getLyr());

    const TwrNum twr(xtalIdx.getTwr());

    const ColNum col(xtalIdx.getCol());

    m_perLyr->Fill(lyr.val());
    m_perTwr->Fill(twr.val());
    m_perXtal->Fill(xtalIdx.val());
    m_perTwrLyr[twr]->Fill(lyr.val());
    m_perTwrCol[twr]->Fill(col.val());
  }

  void MPDHists::buildTuple() {
    TNtuple *tuple = 0;


    switch (m_fitMethod) {
    case FitMethods::LANDAU:
      break;

    case FitMethods::LANGAU:
      tuple = &(LangauFun::buildTuple());
      break;

    default:
      throw invalid_argument("invalid fit_method");
    }

    for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
      switch (m_fitMethod) {
      case FitMethods::LANDAU:
        break;

      case FitMethods::LANGAU:
	if(m_dacLLHists[xtalIdx])
	  LangauFun::fillTuple(xtalIdx,
                             *(m_dacLLHists[xtalIdx]),
                             *tuple);
        break;
      }
    }
  }

}; // namespace calibGenCAL
