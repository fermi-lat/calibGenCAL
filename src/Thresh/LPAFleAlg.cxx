// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Algs/MuonPedAlg.cxx,v 1.7 2008/01/22 19:40:59 fewtrell Exp $

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "src/lib/Util/RootFileAnalysis.h"
#include "LPAFleAlg.h"
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

  static const float NOMINAL_FLE_THRESH = 100;

  using namespace CalUtil;
  using namespace std;

  void LPAFleAlg::fillHists(const unsigned nEntries,
                            const std::vector<std::string> &digiFileList) {
    m_trigHists.initHists(100,300);

    /////////////////////////////////////////
    /// Open ROOT Event File  ///////////////
    /////////////////////////////////////////
    RootFileAnalysis rootFile(0,             // mc
                              &digiFileList);

    // configure active branches from input ROOT trees
    cfgBranches(rootFile);


    const unsigned nEvents = rootFile.getEntries();
    LogStrm::get() << __FILE__ << ": Processing: " << nEvents << " events." << endl;

    /////////////////////////////////////////
    /// Event Loop //////////////////////////
    /////////////////////////////////////////
    for (eventData.init(); eventData.m_eventNum < nEvents; eventData.nextEvent()) {
      /////////////////////////////////////////
      /// Load new event //////////////////////
      /////////////////////////////////////////
      if (eventData.m_eventNum % 2000 == 0) {
        // quit if we have enough entries in each histogram
        const unsigned currentMin = m_trigHists.getMinEntries();
        if (currentMin >= nEntries) break;
        LogStrm::get() << "Event: " << eventData.m_eventNum
                       << " min entries per histogram: " << currentMin
                       << endl;
        LogStrm::get().flush();
      }

      if (!rootFile.getEvent(eventData.m_eventNum)) {
        LogStrm::get() << "Warning, event " << eventData.m_eventNum << " not read." << endl;
        continue;
      }

      DigiEvent const*const digiEvent = rootFile.getDigiEvent();
      if (!digiEvent) {
        LogStrm::get() << __FILE__ << ": Unable to read DigiEvent " << eventData.m_eventNum  << endl;
        continue;
      }

      processEvent(*digiEvent);
    }
  }

  /// check that layer matches expected configuration (mostly that we're
  /// not triggering on the wrong columns
  bool LPAFleAlg::checkLyr(const LyrIdx lyrIdx, const FaceNum face) {
    /// can only check for false triggers if the trigger bit is enabled
    if (!eventData.m_diagTrigBits[lyrIdx][XtalDiode(face,LRG_DIODE)])
      return true;

    /// counter var
    unsigned short nCandidateXtals = 0;
    /// counter var
    FaceIdx candidateChannel;
    
    // first find layers w/ only one trigger enabled.
    const TwrNum twr(lyrIdx.getTwr());
    const LyrNum lyr(lyrIdx.getLyr());
    for (ColNum col; col.isValid(); col++) {
      const FaceIdx faceIdx(twr,lyr,col,face);
      
      if (eventData.m_calSignalArray.getFaceSignal(faceIdx) > m_expectedThresh - m_safetyMargin) {
#ifdef CGC_DEBUG
        LogStrm::get() << __FILE__ << ":checkLyr() "
                       << "channel above threshold" << faceIdx.toStr()
                       << " " << eventData.m_calSignalArray.getFaceSignal(faceIdx)
                       << endl;
#endif
        nCandidateXtals++;
        candidateChannel = faceIdx;
      }
    }
    
    /// if we have only one candidate channel, it _better_ be an enabled channel
    if (nCandidateXtals == 1 && !channelEnabled(candidateChannel)) {
      LogStrm::get() << "WARNING: trigger found in disabled channel: " << candidateChannel.toStr()
                     << " event# " << eventData.m_eventNum
                     << " mev " <<  eventData.m_calSignalArray.getFaceSignal(candidateChannel)
                     << endl;
      return false;
    }

    return true;
  }

  /// process all hits in single layer
  ///
  /// cuts are as follows:
  /// - CUT 1: trigger bit set in opposite layer
  /// - CUT 2: only one 'enabled' channel in this layer has enough MeV to trigger
  /// - CUT 3: extreme asymmetry indicates direct deposit - bail
  /// - FILL 1: now fill spectrum histogram
  /// - CUT 4: trigger bit set in active layer
  /// - FILL 2: now fill trigger histogram
  void LPAFleAlg::processLyr(const LyrIdx lyrIdx, const FaceNum face) {
    /// CUT 1 - confirm that opposite face is triggered
    const FaceNum oppFace(face.oppositeFace());
    if (!eventData.m_diagTrigBits[lyrIdx][XtalDiode(oppFace,LRG_DIODE)])
      return;

    /// CUT 2
    /// count enabled xtals in layer > .5 thresh (there can only be one)
    /// must be unambiguous which channel fired
    const vector<ColNum> trigCandidates(countTrigCandidates(lyrIdx,face));
    if (trigCandidates.size() != 1)
      return;

    /// pick 'trigger candidate' channel
    const ColNum col(trigCandidates[0]);
    const FaceIdx faceIdx(lyrIdx.getTwr(),lyrIdx.getLyr(),col,face);
    
    /// CUT 3 - extreme asymmetry (diode deposit)
    const FaceIdx oppFaceIdx(lyrIdx.getTwr(), lyrIdx.getLyr(), col, oppFace);
    /// 'healthy assymmetry maxes around 2:1, so we'll cut anything above 3:1
    if (eventData.m_calSignalArray.getFaceSignal(faceIdx) /
        eventData.m_calSignalArray.getFaceSignal(oppFaceIdx) > 3) {
      LogStrm::get() << "INFO: direct diode deposit detected: "
                     << " event# " << eventData.m_eventNum
                     << " " << faceIdx.toStr() << " " << eventData.m_calSignalArray.getFaceSignal(faceIdx)
                     << " " << oppFaceIdx.toStr() << " " << eventData.m_calSignalArray.getFaceSignal(oppFaceIdx)
                     << endl;
      return;
    }
      

    /// FILL 1: fill spectrum hist
    m_trigHists.getSpecHist(faceIdx)->Fill(eventData.m_calSignalArray.getFaceSignal(faceIdx));

    /// CUT 4 - trigger assertied
    if (!eventData.m_diagTrigBits[lyrIdx][XtalDiode(face,LRG_DIODE)])
      return;
#ifdef CGC_DEBUG
    LogStrm::get() << __FILE__ << ":processLyr() "
                   << "trigger hit found" << faceIdx.toStr()
                   << " " << eventData.m_calSignalArray.getFaceSignal(faceIdx)
                   << endl;

#endif

    const float faceSignal = eventData.m_calSignalArray.getFaceSignal(faceIdx);

    if (faceSignal < .75*NOMINAL_FLE_THRESH) {
      LogStrm::get() << __FILE__ << ":processLyr() "
                     << "WARNING: Likely false trigger reported: " << faceIdx.toStr()
                     << " event: " << eventData.m_eventNum 
                     << " mev:  " << faceSignal
                     << endl;

      for (ColNum col; col.isValid(); col++) {
        const FaceIdx tmpIdx(lyrIdx.getTwr(), lyrIdx.getLyr(), col, face);
        LogStrm::get() << " " << tmpIdx.toStr()
                       << " " << eventData.m_calSignalArray.getFaceSignal(tmpIdx)
                       << endl;
      
      }
    }
    
    /// FILL 2: trigger histogram
    m_trigHists.getTrigHist(faceIdx)->Fill(faceSignal);
  }

  bool LPAFleAlg::channelEnabled(const FaceIdx faceIdx) {
    /// check that current face is enabled
    if (faceIdx.getFace() != m_face)
      return false;

    /// otherwise use standard trigger pattern check
    return LPATrigAlg::channelEnabled(faceIdx);
  }

  void LPAFleAlg::processEvent(const DigiEvent &digiEvent) {
    /// load up all trigger bits from diagnostic collection
    fillTrigBitArray(digiEvent);

    /// load up faceSignal in mev from all digis
    eventData.m_calSignalArray.fillArray(digiEvent);

    for (LyrIdx lyrIdx; lyrIdx.isValid(); lyrIdx++) {
      /// check layer for data inconsistent with expected LAT configuration.
      if (!checkLyr(lyrIdx, m_face))
        continue;
      processLyr(lyrIdx, m_face);
    }
  }


}; // namespace calibGenCAL
