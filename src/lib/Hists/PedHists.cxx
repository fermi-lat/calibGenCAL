// $Header: //

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "PedHists.h"

// GLAST INCLUDES
#include "CalUtil/SimpleCalCalib/CalPed.h"

// EXTLIB INCLUDES

// STD INCLUDES

using namespace CalUtil;
using namespace std;

namespace calibGenCAL {
  void PedHists::fitHists(CalPed &calPed) {
    for (RngIdx rngIdx; rngIdx.isValid(); rngIdx++) {
      TH1S *const hist = getHist(rngIdx);
      // skip non existant hists
      if (hist == 0)
        continue;

      // loop through all nSlicesPerXtal bins in pedmetry profile

      // skip empty histograms
      TH1S &h = *hist;
      if (h.GetEntries() == 0)
        continue;

      // trim outliers
      float av = h.GetMean(); float err = h.GetRMS();
      for ( unsigned short iter = 0; iter < 3; iter++ ) {
        h.SetAxisRange(av-3*err, av+3*err);
        av = h.GetMean(); err = h.GetRMS();
      }

      // gaussian fit
      h.Fit("gaus", "Q", "", av-3*err, av+3*err );
      h.SetAxisRange(av-150, av+150);

      // assign values to permanent arrays
      calPed.setPed(rngIdx,
                    ((TF1&)*h.GetFunction("gaus")).GetParameter(1));
      calPed.setPedSig(rngIdx,
                       ((TF1&)*h.GetFunction("gaus")).GetParameter(2));

    }
  }

}; // namespace calibGenCAL
