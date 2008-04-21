// $Header: //

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "TrigHists.h"
#include "src/lib/Util/ROOTUtil.h"
#include "src/lib/Util/CGCUtil.h"

// GLAST INCLUDES

// EXTLIB INCLUDES
#include "TH1S.h"
#include "TStyle.h"
#include "TDirectory.h"

// STD INCLUDES
#include <sstream>
#include <string>

using namespace CalUtil;
using namespace std;

namespace calibGenCAL {

  std::string TrigHists::enum_str(const HistType histType) {
    switch (histType) {
    case SPECTRUM_HIST:
      return "specHist";
    case TRIGGER_HIST:
      return "trigHist";
    default:
      throw runtime_error(to_str(histType) +  " is inavlid histType enum");
    }
  }

  void TrigHists::initHists(const unsigned short nBins,
                            const float maxMeV) {
    for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++) {
      string tmp = genHistName(TRIGGER_HIST, faceIdx);
      m_trigHists[faceIdx] = new TH1S(tmp.c_str(),
                                      tmp.c_str(),
                                      nBins,
                                      0,
                                      maxMeV);

      tmp = genHistName(SPECTRUM_HIST, faceIdx);
      m_specHists[faceIdx] = new TH1S(tmp.c_str(),
                                      tmp.c_str(),
                                      nBins,
                                      0,
                                      maxMeV);

    }
  }
  
  void TrigHists::summarizeHists(ostream &ostrm) const {
    ostrm << "XTAL\tNHITS" << endl;
    for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++)
      if (m_trigHists[faceIdx])
        if (m_trigHists[faceIdx]->GetEntries() > 0)
          ostrm << faceIdx.val() << "\t"
                << m_trigHists[faceIdx]->GetEntries()
                << endl;
  }

  void TrigHists::loadHists(const TDirectory &readDir) {
    for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++) {
      string histname = genHistName(TRIGGER_HIST, faceIdx);
      TH1S  *hist_ptr = retrieveROOTObj < TH1S > (readDir, histname);
      if (!hist_ptr)
        continue;
      
      m_trigHists[faceIdx] = hist_ptr;

      histname = genHistName(SPECTRUM_HIST, faceIdx);
      hist_ptr = retrieveROOTObj < TH1S > (readDir, histname);
      if (!hist_ptr)
        continue;
      
      m_specHists[faceIdx] = hist_ptr;

    }
  }

  string TrigHists::genHistName(const HistType histType,
                                const FaceIdx faceIdx) {
    ostringstream tmp;

    tmp << enum_str(histType)
        << "_" << faceIdx.toStr();
    return tmp.str();
  }

  unsigned TrigHists::getMinEntries() const {
    unsigned      retVal  = ULONG_MAX;

    unsigned long sum     = 0;
    unsigned      n = 0;
    unsigned      maxHits = 0;


    for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++) {
      const unsigned nEntries = (unsigned)m_trigHists[faceIdx]->GetEntries();

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

  void TrigHists::trimHists() {
    for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++) {
      if (m_trigHists[faceIdx]) {
        if (m_trigHists[faceIdx]->GetEntries() == 0) {
          delete m_trigHists[faceIdx];
          m_trigHists[faceIdx] = 0;
        }

        if (m_specHists[faceIdx]->GetEntries() == 0) {
          delete m_specHists[faceIdx];
          m_specHists[faceIdx] = 0;
        }
      }
    }
  }

}; // namespace calibGenCAL
