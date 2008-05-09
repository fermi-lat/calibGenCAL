// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/Thresh/LPAFheAlg.cxx,v 1.1 2008/04/21 20:43:13 fewtrell Exp $

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "src/lib/Util/RootFileAnalysis.h"
#include "LPAFheAlg.h"
#include "src/lib/Util/ROOTUtil.h"
#include "src/lib/Util/CGCUtil.h"

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
  static const float NOMINAL_FHE_THRESH = 1000;

  using namespace CalUtil;
  using namespace std;

  void LPAFheAlg::fillHists(const unsigned nEntries,
                            const std::vector<std::string> &digiFileList) {
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

  /// process all hits in single layer
  ///
  /// cuts are as follows:
  /// - CUT 1: FLE enabled for this twr,lyr,face
  /// - CUT 2: only one 'enabled' channel in this layer has enough MeV to trigger
  /// - CUT 3: skip events w/ extreme asymmetry indicating direct diode deposit.
  /// - FILL 1: fill spectrum histogram
  /// - CUT 4: FHE trigger bit set in active layer
  /// - FILL 2: fill trigger histogram
  void LPAFheAlg::processLyr(const LyrIdx lyrIdx, const FaceNum face) {
    /// CUT 1 - FLE triggered for this tower, layer, and face
    if (!eventData.m_diagTrigBits[lyrIdx][XtalDiode(face, LRG_DIODE)])
      return;

    /// CUT 2
    /// count enabled xtals in layer > candidate thresh (there can only be one)
    /// must be unambiguous which channel fired
    const vector<ColNum> trigCandidates(countTrigCandidates(lyrIdx, face));
    if (trigCandidates.size() != 1)
      return;

    const ColNum col(trigCandidates[0]);
    const FaceIdx faceIdx(lyrIdx.getTwr(),lyrIdx.getLyr(),col, face);

    // CUT 3 - avoid direct diode deposit
    const FaceNum oppFace(face.oppositeFace());
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
    m_specHists.produceHist(faceIdx).Fill(eventData.m_calSignalArray.getFaceSignal(faceIdx));

    /// CUT 4 - FHE triggered for this tower, layer, and face
    if (!eventData.m_diagTrigBits[lyrIdx][XtalDiode(face, SM_DIODE)])
      return;
#ifdef CGC_DEBUG
    LogStrm::get() << __FILE__ << ":processLyr() "
                   << "trigger hit found" << faceIdx.toStr()
                   << " " << eventData.m_calSignalArray.getFaceSignal(faceIdx)
                   << endl;

#endif

    /// warn if trigger is suspiciously low
    const float faceSignal = eventData.m_calSignalArray.getFaceSignal(faceIdx);
    if (faceSignal < .75*NOMINAL_FHE_THRESH) {
      LogStrm::get() << __FILE__ << ":processLyr() "
                     << "WARNING: Likely false trigger reported: " << faceIdx.toStr()
                     << " event: " << eventData.m_eventNum 
                     << " mev: " << faceSignal
                     << " fle: " << eventData.m_diagTrigBits[lyrIdx][XtalDiode(face, LRG_DIODE)]
                     << " fhe: " << eventData.m_diagTrigBits[lyrIdx][XtalDiode(face, SM_DIODE)]
                     << endl;

      for (ColNum col; col.isValid(); col++) {
        const FaceIdx tmpIdx(lyrIdx.getTwr(), lyrIdx.getLyr(), col, face);
        const FaceNum oppFace(face.oppositeFace());
        const FaceIdx oppFaceIdx(lyrIdx.getTwr(), lyrIdx.getLyr(), col, oppFace);
        LogStrm::get() << " " << tmpIdx.toStr()
                       << " " << eventData.m_calSignalArray.getFaceSignal(tmpIdx)
                       << " " << oppFaceIdx.toStr()
                       << " " << eventData.m_calSignalArray.getFaceSignal(oppFaceIdx)
                       << endl;
      
      }
    }
    
    /// FILL 2: fill trigger histogram
    m_trigHists.produceHist(faceIdx).Fill(faceSignal);
  }

  /// check that layer matches expected configuration (mostly that we're
  /// not triggering on the wrong columns)
  bool LPAFheAlg::checkLyr(const LyrIdx lyrIdx, const CalUtil::FaceNum face) {
    /// can only check for false triggers if the trigger bit is enabled
    if (!eventData.m_diagTrigBits[lyrIdx][XtalDiode(face,SM_DIODE)])
      return true;

    /// if FHE high & FLE low, this indicates a direct diode deposit & we should bail
    if (!eventData.m_diagTrigBits[lyrIdx][XtalDiode(face, LRG_DIODE)]) {
      LogStrm::get() << "INFO: direct diode deposit detected: "
                     << " event# " << eventData.m_eventNum
                     << " channel: " << lyrIdx.toStr() << " " << face.toStr()
                     << " fle: " << eventData.m_diagTrigBits[lyrIdx][XtalDiode(face, LRG_DIODE)]
                     << " fhe: " << eventData.m_diagTrigBits[lyrIdx][XtalDiode(face, SM_DIODE)]
                     << endl;
      
#if CGC_DEBUG
      for (ColNum col; col.isValid(); col++) {
        const FaceIdx tmpIdx(lyrIdx.getTwr(), lyrIdx.getLyr(), col,face);
        const FaceNum oppFace(face.oppositeFace());
        const FaceIdx oppFaceIdx(lyrIdx.getTwr(), lyrIdx.getLyr(), col, oppFace);
        LogStrm::get() << " " << tmpIdx.toStr()
                       << " " << eventData.m_calSignalArray.getFaceSignal(tmpIdx)
                       << " " << oppFaceIdx.toStr()
                       << " " << eventData.m_calSignalArray.getFaceSignal(oppFaceIdx)
                       << endl;
      } // for colNum
#endif
      
      return false;
    } // if diode deposit

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
                     << " fle: " << eventData.m_diagTrigBits[lyrIdx][XtalDiode(face, LRG_DIODE)]
                     << " fhe: " << eventData.m_diagTrigBits[lyrIdx][XtalDiode(face, SM_DIODE)]
                     << endl;
      
      for (ColNum col; col.isValid(); col++) {
        const FaceIdx tmpIdx(lyrIdx.getTwr(), lyrIdx.getLyr(), col,face);
        const FaceNum oppFace(face.oppositeFace());
        const FaceIdx oppFaceIdx(lyrIdx.getTwr(), lyrIdx.getLyr(), col, oppFace);
        LogStrm::get() << " " << tmpIdx.toStr()
                       << " " << eventData.m_calSignalArray.getFaceSignal(tmpIdx)
                       << " " << oppFaceIdx.toStr()
                       << " " << eventData.m_calSignalArray.getFaceSignal(oppFaceIdx)
                       << endl;
      } // for colNum
      
      return false;
    } // if !channelEnabled
    
    return true;
  }


  void LPAFheAlg::processEvent(const DigiEvent &digiEvent) {
    /// load up all trigger bits from diagnostic collection
    fillTrigBitArray(digiEvent);
    
    /// load up faceSignal in mev from all digis
    eventData.m_calSignalArray.fillArray(digiEvent);
    
    for (LyrIdx lyrIdx; lyrIdx.isValid(); lyrIdx++)
      for (FaceNum face; face.isValid(); face++) {
        /// check layer for data inconsistent with expected LAT configuration.
        if (!checkLyr(lyrIdx, face))
          continue;
        processLyr(lyrIdx, face);
      }
  }


}; // namespace calibGenCAL
