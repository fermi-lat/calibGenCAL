// $Header: //

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "AsymHists.h"
#include "src/lib/Util/ROOTUtil.h"
#include "src/lib/Util/CGCUtil.h"
#include "src/lib/Specs/CalGeom.h"

// GLAST INCLUDES

// EXTLIB INCLUDES
#include "TH2S.h"
#include "TStyle.h"
#include "TDirectory.h"

// STD INCLUDES
#include <sstream>
#include <string>

using namespace CalUtil;
using namespace std;

namespace calibGenCAL {
  /// \param nSlicesPerXtal number of slices along entire crystal length
  /// \arapm nSlicesPerHist only the middle 'n' slices will actually be measued as outer most slices have very high error
  AsymHists::AsymHists(const CalResponse::CAL_GAIN_INTENT calGain,
                       const unsigned short nSlicesPerXtal,
                       const unsigned short nSlicesPerHist,
                       TDirectory *const writeDir,
                       TDirectory *const readDir) 
    :
    m_nSlicesPerXtal(nSlicesPerXtal),
    m_nSlicesPerHist(nSlicesPerHist),
    m_mmCoveredByHists(CalGeom::CsILength*nSlicesPerHist/nSlicesPerXtal),
    m_mmIgnoredOnXtalEnd((CalGeom::CsILength-m_mmCoveredByHists)/2),
    m_writeDir(writeDir),
    m_calGain(calGain)
  {
    if (readDir != 0)
      loadHists(*readDir);
    else
      initHists();
    
    /// move loaded histograms to output dir.
    if (writeDir)
      setDirectory(writeDir);
  }

  void AsymHists::initHists() {
    m_asymHists.reset(new AsymHistCol("asym",
                                      m_writeDir,
                                      0,
                                      m_nSlicesPerHist,
                                      -1*(CalGeom::CsILength/2-m_mmIgnoredOnXtalEnd),
                                      CalGeom::CsILength/2-m_mmIgnoredOnXtalEnd,
                                      400,
                                      -1,
                                      1));

  }

  void AsymHists::summarizeHists(ostream &ostrm) const {
    ostrm << "XTAL\tNHITS" << endl;
    for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
      AsymHistId histId(ASYM_SS, xtalIdx);
      TH2S const*const hist = m_asymHists->getHist(histId);
      if (hist!=0)
        if (hist->GetEntries() > 0)
          ostrm << xtalIdx.val() << "\t"
                << hist->GetEntries()
                << endl;
    }
  }

  void AsymHists::loadHists(TDirectory &readDir) {
    m_asymHists.reset(new AsymHistCol("asym",
                                      m_writeDir,
                                      &readDir,
                                      m_nSlicesPerHist,
                                      -1*(CalGeom::CsILength/2-m_mmIgnoredOnXtalEnd),
                                      CalGeom::CsILength/2-m_mmIgnoredOnXtalEnd));

  }

  void AsymHists::fitHists(CalAsym &calAsym) {
    for (AsymHistId histId; histId.isValid(); histId++) {
      TH2S *const hist = m_asymHists->getHist(histId);
      // skip non existant hists
      if (hist == 0)
        continue;

      // loop through all nSlicesPerXtal bins in asymmetry profile

      // skip empty histograms
      TH2S &h = *hist;
      if (h.GetEntries() == 0)
        continue;

      const AsymType asymType(histId.getAsymType());

      for (unsigned short i = 0; i < m_nSlicesPerHist; i++) {
        // get slice of 2D histogram for each X bin
        const unsigned short binNum = i+1;
        // HISTOGRAM BINS START AT 1 NOT ZERO! (hence 'i+1')
        TH1D &slice = *(h.ProjectionY("slice", binNum, binNum));

        // rebin HE histograms to binWidth>=.02 in log(asym) scale
        if (asymType != ASYM_LL) {
          float binWidth = slice.GetBinWidth(1);
          if (binWidth < .02)
            slice.Rebin((int)(.02/binWidth));
        }

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
        //av = slice.GetMean(); rms = slice.GetRMS();
        // fit w/ gaussian to avoid bias from outliers
        slice.Fit("gaus","Q","", av - 3*rms, av+3*rms);
        av = ((TF1&)*slice.GetFunction("gaus")).GetParameter(1);
        rms = ((TF1&)*slice.GetFunction("gaus")).GetParameter(2);

        // add nominal asymmetry slope back in
        av += nominalAsymSlope()*h.GetBinCenter(binNum);
        
        // add average asymmetry back in
        av += nominalAsymCtr(asymType, m_calGain);

        const AsymType asymType(histId.getAsymType());
        const XtalIdx xtalIdx(histId.getXtalIdx());

        LogStrm::get() << histId.toStr() << " "
                       << i   << " "
                       << av  << " "
                       << rms << " "
                       << endl;

        calAsym.getPtsAsym(xtalIdx,asymType).push_back(av);
        calAsym.getPtsErr(xtalIdx,asymType).push_back(rms);

        // evidently ROOT doesn't like reusing the slice
        // histograms as much as they claim they do.
        slice.Delete();
      }
    }
  }

  void AsymHists::fill(const CalUtil::AsymType asymType,
                       const CalUtil::XtalIdx xtalIdx,
                       const float mmFromCtrLong,
                       const float posDAC,
                       const float negDAC) {
    float asym = log(posDAC/negDAC);

    // subtract nominal asymmetry slope to get flat residual histogram
    asym -= nominalAsymSlope()*mmFromCtrLong;

    // move average asymmetry across entire crystal to 0
    asym -= nominalAsymCtr(asymType, m_calGain);

    AsymHistId histId(asymType, xtalIdx);
    TH2S &hist = m_asymHists->produceHist(histId);

    hist.Fill(mmFromCtrLong, asym);
  }
}; // namespace calibGenCAL
