// $Header: //

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "AsymHists.h"
#include "CGCUtil.h"

// GLAST INCLUDES

// EXTLIB INCLUDES
#include "TH2S.h"
#include "TFile.h"
#include "TStyle.h"

// STD INCLUDES
#include <sstream>

using namespace CalUtil;
using namespace std;
using namespace CGCUtil;

void AsymHists::initHists() {
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

void AsymHists::summarizeHists(ostream &ostrm) {
  ostrm << "XTAL\tNHITS" << endl;
  for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++)
    if (m_histograms[ASYM_SS][xtalIdx])
      if (m_histograms[ASYM_SS][xtalIdx]->GetEntries() > 0)
        ostrm << xtalIdx.val() << "\t"
              << m_histograms[ASYM_SS][xtalIdx]->GetEntries()
              << endl;
}

void AsymHists::loadHists(const TFile &histFile) {
  for (AsymType asymType; asymType.isValid(); asymType++)
    // PER XTAL LOOP
    for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
      string histname = genHistName(asymType, xtalIdx);
      TH2S  *hist_ptr = CGCUtil::retrieveHist < TH2S > (histFile, histname);
      if (!hist_ptr)
        continue;

      m_histograms[asymType][xtalIdx] = hist_ptr;
    }
}

string AsymHists::genHistName(AsymType asymType,
                              XtalIdx xtalIdx) {
  ostringstream tmp;


  tmp <<  "asym"
      << asymType.getDiode(POS_FACE).val()
      << asymType.getDiode(NEG_FACE).val()
      << "_" << xtalIdx.val();
  return tmp.str();
}

unsigned AsymHists::getMinEntries() {
  unsigned      retVal  = ULONG_MAX;

  unsigned long sum     = 0;
  unsigned      n = 0;
  unsigned      maxHits = 0;


  for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
    unsigned nEntries = (unsigned)m_histograms[ASYM_SS][xtalIdx]->GetEntries();

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

  LogStream::get() << " Channels Detected: "  << n
                   << " Avg Hits/channel: " << ((n) ? (double)sum/n : 0)
                   << " Max: " << maxHits
                   << endl;

  // case where there are no fills at all
  if (retVal == ULONG_MAX)
    return 0;

  return retVal;
}

void AsymHists::trimHists() {
  for (AsymType asymType; asymType.isValid(); asymType++)
    for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++)
      if (m_histograms[asymType][xtalIdx])
        if (!m_histograms[asymType][xtalIdx]->GetEntries()) {
          delete m_histograms[asymType][xtalIdx];
          m_histograms[asymType][xtalIdx] = 0;
        }
}

void AsymHists::fitHists(CalAsym &calAsym) {
  for (AsymType asymType; asymType.isValid(); asymType++)

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
        TH1D &slice = *(h.ProjectionY("slice", i+1, i+1));
                
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

        calAsym.getPtsAsym(xtalIdx, asymType).push_back(av);
        calAsym.getPtsErr(xtalIdx, asymType).push_back(rms);

        // evidently ROOT doesn't like reusing the slice
        // histograms as much as they claim they do.
        slice.Delete();
      }
    }
}

