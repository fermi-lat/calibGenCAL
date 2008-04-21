// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Algs/MuonPedAlg.cxx,v 1.7 2008/01/22 19:40:59 fewtrell Exp $

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "src/lib/Util/RootFileAnalysis.h"
#include "LPATrigAlg.h"
#include "src/lib/Util/ROOTUtil.h"
#include "src/lib/Util/CGCUtil.h"
#include "src/lib/Hists/TrigHists.h"

// GLAST INCLUDES
#include "CalUtil/CalVec.h"
#include "CalUtil/CalDiagnosticWord.h"
#include "digiRootData/DigiEvent.h"
#include "CalUtil/SimpleCalCalib/CalPed.h"
#include "CalUtil/SimpleCalCalib/ADC2NRG.h"

// EXTLIB INCLUDES
#include "TH1F.h"

// STD INCLUDES
#include <sstream>
#include <cmath>
#include <algorithm>


//#define CGC_DEBUG // enable debugging printouts

namespace calibGenCAL {

  using namespace CalUtil;
  using namespace std;

  void LPATrigAlg::cfgBranches(RootFileAnalysis &rootFile) {
    // enable only needed branches in root file
    rootFile.getDigiChain()->SetBranchStatus("*", 0);
    rootFile.getDigiChain()->SetBranchStatus("m_calDigiCloneCol");                                                  
    rootFile.getDigiChain()->SetBranchStatus("m_calDiagnosticCloneCol");
  }

  /// fill eventData.m_diagTrigBits with diagnostic data from given event
  void LPATrigAlg::fillTrigBitArray(const DigiEvent &digiEvent) {
    //-- populate fle diagnostic info --//
    TClonesArray const*const calDiagCol = digiEvent.getCalDiagnosticCol();
    if (!calDiagCol) {
      LogStrm::get() << "no calDiagCol found for event#" << eventData.m_eventNum << endl;
      return;
    }
    TIter calDiagIter(calDiagCol);

    // CalDiagnosticData loop
    const CalDiagnosticData  *pdiag = 0;
    while ((pdiag = (dynamic_cast<CalDiagnosticData *>(calDiagIter.Next())))) {
      //loop through each 'hit' in one event
      const CalDiagnosticData &cdiag = *pdiag;    // use ref to reduce '->'
      const TwrNum twr(cdiag.tower());
      const LyrNum lyr(cdiag.layer());
      const LyrIdx lyrIdx(twr,lyr);

      for (FaceNum face; face.isValid(); face++) {
        eventData.m_diagTrigBits[lyrIdx][XtalDiode(face, LRG_DIODE)] = cdiag.low(face.val());
        eventData.m_diagTrigBits[lyrIdx][XtalDiode(face, SM_DIODE) ] = cdiag.high(face.val());
      }

    } // cal diagnostic loop
  }

  std::vector<CalUtil::ColNum> LPATrigAlg::countTrigCandidates(const CalUtil::LyrIdx lyrIdx,
                                                               const CalUtil::FaceNum face) {
    /// return value
    vector<ColNum> retList;

    const TwrNum twr(lyrIdx.getTwr());
    const LyrNum lyr(lyrIdx.getLyr());
    for (ColNum col; col.isValid(); col++) {
      const FaceIdx faceIdx(twr,lyr,col,face);

      if (channelEnabled(faceIdx))
        if (eventData.m_calSignalArray.getFaceSignal(faceIdx) > m_expectedThresh - m_safetyMargin) {
#ifdef CGC_DEBUG
          LogStrm::get() << __FILE__ << ":countTrigCandidates() "
                         << "channel above threshold" << faceIdx.toStr()
                         << " " << eventData.m_calSignalArray.getFaceSignal(faceIdx)
                         << endl;
#endif

          retList.push_back(col);
        }
    }      

    return retList;
  }


  std::string LPATrigAlg::enum_str(const TriggerPattern trigPattern) {
    switch (trigPattern) {
    case EvenRowEvenCol:
      return "EREC";
    case EvenRowOddCol:
      return "EROC";
    default:
      throw runtime_error(to_str(trigPattern) +  " is inavlid trigPattern enum");
    }
  }

  void LPATrigAlg::EventData::clear() {
    m_calSignalArray.clear();

    for (LyrIdx lyrIdx; lyrIdx.isValid(); lyrIdx++)
      fill(m_diagTrigBits[lyrIdx].begin(),
           m_diagTrigBits[lyrIdx].end(),
           false);

  }


  bool LPATrigAlg::channelEnabled(const FaceIdx faceIdx) {
    /// check that we are processing enabled channel
    GCRCNum gcrc = faceIdx.getLyr().getGCRC();
    ColNum col = faceIdx.getCol();

    switch (m_trigPattern) {
    case EvenRowEvenCol: //even cols enabled on even rows, odd cols on odd rows
      if (gcrc.val() %2 != col.val() %2)
        return false;
      break;
    case EvenRowOddCol: // odd cols on even rows, even cols  odd rows
      if (gcrc.val()%2 == col.val()%2)
        return false;
      break;
    default:
      throw runtime_error(to_str(m_trigPattern) +  " is inavlid trigPattern enum");
    }

    // we made it!
    return true;
  }


}; // namespace calibGenCAL
