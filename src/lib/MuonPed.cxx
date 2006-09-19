// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/MuonPed.cxx,v 1.5 2006/09/15 15:02:10 fewtrell Exp $
/** @file
    @author Zachary Fewtrell
 */

// LOCAL INCLUDES
#include "RootFileAnalysis.h"
#include "MuonPed.h"
#include "CGCUtil.h"

// GLAST INCLUDES

// EXTLIB INCLUDES
#include "TF1.h"
#include "TKey.h"

// STD INCLUDES
#include <sstream>
#include <ostream>

using namespace CGCUtil;

MuonPed::MuonPed(ostream &ostrm) :
  m_ostrm(ostrm)
{
}

void MuonPed::initHists() {
  m_histograms.resize(RngIdx::N_VALS);

  for (RngIdx rngIdx; rngIdx.isValid(); rngIdx++) {
    if (m_histograms[rngIdx] == 0) {
      string histname = genHistName(rngIdx);
      
      m_histograms[rngIdx] = new TH1S(histname.c_str(),
                                      histname.c_str(),
                                      1000,0.5,1000.5);
    }
  }
}

void MuonPed::fillHists(unsigned nEntries, 
                        const vector<string> &rootFileList, 
                        const CalPed *roughPeds,
                        bool periodicTrigger) {

  /////////////////////////////////////////
  /// Initialize Histograms ///////////////
  /////////////////////////////////////////
  initHists();


  /////////////////////////////////////////
  /// Open ROOT Event File  ///////////////
  /////////////////////////////////////////
  RootFileAnalysis rootFile(0, &rootFileList, 0);

  // enable only needed branches in root file
  rootFile.getDigiChain()->SetBranchStatus("*",0);
  rootFile.getDigiChain()->SetBranchStatus("m_calDigiCloneCol");
  if (periodicTrigger) {
    rootFile.getDigiChain()->SetBranchStatus("m_gem");
    rootFile.getDigiChain()->SetBranchStatus("m_summary");
  }

  unsigned nEvents = rootFile.getEntries();
  m_ostrm << __FILE__ << ": Processing: " << nEvents << " events." << endl;

  /////////////////////////////////////////
  /// Event Loop //////////////////////////
  /////////////////////////////////////////
  bool prev4Range = true; // in periodic trigger mode we will skip these events
  bool fourRange  = true;
  for (unsigned eventNum = 0; eventNum < nEvents; eventNum++) {
    // save previous mode
    prev4Range = fourRange;

    /////////////////////////////////////////
    /// Load new event //////////////////////
    /////////////////////////////////////////
    if (eventNum % 2000 == 0) {
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
      fourRange = true;
      continue;
    }

    DigiEvent *digiEvent = rootFile.getDigiEvent();
    if (!digiEvent) {
      m_ostrm << __FILE__ << ": Unable to read DigiEvent " << eventNum  << endl;
      fourRange = true;
      continue;
    }

    /////////////////////////////////////////
    /// Event/Trigger level cuts ////////////
    /////////////////////////////////////////
    if (periodicTrigger) {
      // quick check if we are in 4-range mode
      EventSummaryData& summary = digiEvent->getEventSummaryData();
      if (&summary == 0) {
        m_ostrm << "Warning, eventSummary data not found for event: "
                << eventNum << endl;
        fourRange = true;
        continue;
      }
      fourRange = summary.readout4();

      const Gem& gem = digiEvent->getGem();
      if (&gem==0) {
        m_ostrm << "Warning, gem data not found for event: "
                << eventNum << endl;
        continue;
      }

      float gemDeltaEventTime = gem.getDeltaEventTime()*0.05;
      unsigned gemConditionsWord = gem.getConditionSummary();
      if(gemConditionsWord != 32 ||  // skip unless we are periodic trigger only
         prev4Range              ||  // avoid bias from 4 range readout in prev event
         gemDeltaEventTime < 2000) {   // avoid bias from shaped readout noise from adjacent event
        continue;
      }
    }

    const TClonesArray* calDigiCol = digiEvent->getCalDigiCol();
    if (!calDigiCol) {
      m_ostrm << "no calDigiCol found for event#" << eventNum << endl;
      continue;
    }

    TIter calDigiIter(calDigiCol);
    const CalDigi *pCalDigi = 0;

    /////////////////////////////////////////
    /// Xtal Hit Loop ///////////////////////
    /////////////////////////////////////////
    while ((pCalDigi = (CalDigi*)calDigiIter.Next())) {
      const CalDigi &calDigi = *pCalDigi; // use reference to avoid -> syntax

      //-- XtalId --//
      idents::CalXtalId id(calDigi.getPackedId()); // get interaction information
      // skip hits not for current tower.
      XtalIdx xtalIdx(id);

      unsigned nRO = calDigi.getNumReadouts();

      if (nRO != 4) {
        ostringstream tmp;
        tmp << __FILE__  << ":"     << __LINE__ << " " 
            << "Event# " << eventNum << " Invalid nReadouts, expecting 4";
        throw runtime_error(tmp.str());
      }

      // 1st look at LEX8 vals
      CalArray<FaceNum, float> adcL8;
      bool badHit = false;
      for (FaceNum face; face.isValid(); face++) {
        adcL8[face] = calDigi.getAdcSelectedRange(LEX8, (CalXtalId::XtalFace)face.val());
        // check for missing readout
        if (adcL8[face] < 0) {
          m_ostrm << "Couldn't get LEX8 readout for event=" << eventNum << endl;
          badHit = true;
          continue;
        }
      }
      if (badHit) continue;      

      /////////////////////////////////////////
      /// 'Rough' pedestal mode (fist pass) ///
      /////////////////////////////////////////
      if (!roughPeds) {
        for (FaceNum face; face.isValid(); face++) {
          RngIdx rngIdx(xtalIdx, face, LEX8);
          
          m_histograms[rngIdx]->Fill(adcL8[face]);
        }
      /////////////////////////////////////////
      /// Cut outliers (2nd pass) /////////////
      /////////////////////////////////////////
      } else  {
        // skip outliers (outside of 5 sigma on either side)
        if (fabs(adcL8[NEG_FACE] - roughPeds->getPed(RngIdx(xtalIdx,NEG_FACE,LEX8))) >=
            5*roughPeds->getPedSig(RngIdx(xtalIdx,NEG_FACE,LEX8)) ||
            fabs(adcL8[POS_FACE] - roughPeds->getPed(RngIdx(xtalIdx,POS_FACE,LEX8))) >=
            5*roughPeds->getPedSig(RngIdx(xtalIdx,POS_FACE,LEX8)))
          continue;
      
        //-- Fill histograms for all 4 ranges
        for (unsigned short n = 0; n < nRO; n++) {
          const CalXtalReadout &readout = *calDigi.getXtalReadout(n);
          
          for (FaceNum face; face.isValid(); face++) {
            // check that we are in the expected readout mode
            RngNum rng = readout.getRange((CalXtalId::XtalFace)face.val());
            unsigned short adc = readout.getAdc((CalXtalId::XtalFace)face.val());
            RngIdx rngIdx(xtalIdx, face, rng);
            m_histograms[rngIdx]->Fill(adc);
          }
        }
      }
    }
  }
}

void MuonPed::fitHists(CalPed &calPed) {
  for (RngIdx rngIdx; rngIdx.isValid(); rngIdx++) {
    // skip absent histograms
    if (!m_histograms[rngIdx])
      continue;

    // select histogram from list
    TH1S &h= *m_histograms[rngIdx];

    // skip empty histograms
    if (h.GetEntries() == 0) 
      continue;

    // trim outliers
    float av = h.GetMean();float err =h.GetRMS();
    for( unsigned short iter=0; iter<3;iter++ ) {
      h.SetAxisRange(av-3*err,av+3*err);
      av = h.GetMean(); err= h.GetRMS();
    }
    
    // gaussian fit
    h.Fit("gaus", "Q","", av-3*err, av+3*err );
    h.SetAxisRange(av-150,av+150);

    // assign values to permanent arrays
    calPed.setPed(rngIdx, 
                  ((TF1&)*h.GetFunction("gaus")).GetParameter(1));
    calPed.setPedSig(rngIdx,
                     ((TF1&)*h.GetFunction("gaus")).GetParameter(2));
  }
}

void MuonPed::loadHists(const string &filename) {
  TFile histFile(filename.c_str(), "READ");

  for (RngIdx rngIdx; rngIdx.isValid(); rngIdx++) {
    string histname = genHistName(rngIdx);
    
    TH1S *hist_ptr = retrieveHist<TH1S>(histFile, histname);
    if (!hist_ptr)
      continue;

    // move histogram into Global ROOT memory
    // so it is not deleted when input file is closed.
    // this may be a memory leak, i don't think
    // anyone cares.
    hist_ptr->SetDirectory(0);

    m_histograms[rngIdx] = hist_ptr;
  }
}


string MuonPed::genHistName(RngIdx rngIdx) {
  ostringstream tmp;
  tmp << "muonpeds_" << rngIdx.val();
  return tmp.str();
}

unsigned MuonPed::getMinEntries() {
  unsigned retVal = ULONG_MAX;

  for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
    RngIdx rngIdx(xtalIdx, POS_FACE, LEX8);
    unsigned nEntries = (unsigned)m_histograms[rngIdx]->GetEntries();
    
    // only count histograms that have been filled 
    // (some histograms will never be filled if we are
    // not using all 16 towers)
    if (nEntries != 0) {
      retVal = min(retVal,nEntries);
    }
  }

  // case where there are no fills at all
  if (retVal == ULONG_MAX)
    return 0;

  return retVal;
}
