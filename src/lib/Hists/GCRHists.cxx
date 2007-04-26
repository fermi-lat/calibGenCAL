// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Hists/GCRHists.cxx,v 1.7 2007/04/24 16:45:07 fewtrell Exp $

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "GCRHists.h"
#include "../Util/CGCUtil.h"
#include "../Util/MultiPeakGaus.h"

// GLAST INCLUDES

// EXTLIB INCLUDES
#include "TH1S.h"
#include "TProfile.h"

// STD INCLUDES
#include <cmath>
#include <sstream>

using namespace std;
using namespace CalUtil;
using namespace CGCUtil;

GCRHists::GCRHists(bool summaryMode) :
  m_meanDACHists(DiodeNum::N_VALS),
  m_adcHists(RngIdx::N_VALS),
  m_dacRatioProfs(FaceIdx::N_VALS),
  // only need index of 3 (0vs1,1vs2,2vs3)
  m_adcRatioProfs(3),
  // only need index of 3 (0vs1,1vs2,2vs3)
  m_adcRatioSumProf(3),
  summaryMode(summaryMode)
{
	for (DiodeNum diode; diode.isValid(); diode++)
		m_meanDACHists[diode].fill(0);

	for (RngNum rng; rng <= HEX8; rng++)
		m_adcRatioProfs[rng].fill(0);

	m_meanDACSumHist.fill(0);
	m_meanADCSumHist.fill(0);
}

void GCRHists::initHists() {
  //-- INIT SUMMARY HISTS  --//
  string histname;

  for (DiodeNum diode; diode.isValid(); diode++) {
    histname = genMeanDACSumHistName(diode);
    m_meanDACSumHist[diode] = new TH1S(histname.c_str(),
                                       histname.c_str(),
                                       4096, 0, 4095);
  }

  for (RngNum rng; rng.isValid(); rng++) {
    histname = genMeanADCSumHistName(rng);
    m_meanADCSumHist[rng] = new TH1S(histname.c_str(),
                                     histname.c_str(),
                                     4096, 0, 4095);
  }

  histname = genDACRatioSumProfName();
  m_dacRatioSumProf = new TProfile(histname.c_str(),
                                   histname.c_str(),
                                   4096, 0, 4095);

  for (RngNum rng; rng <= HEX8; rng++) {
    histname = genADCRatioSumProfName(rng);
    m_adcRatioSumProf[rng] = new TProfile(histname.c_str(),
                                          histname.c_str(),
                                          4096, 0, 4095);
  }

  if (!summaryMode)
    //-- INIT PER-XTAL HISTS --//
    for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
      for (DiodeNum diode; diode.isValid(); diode++) {
        histname = genMeanDACHistName(diode, xtalIdx);
        m_meanDACHists[diode][xtalIdx] = new TH1S(histname.c_str(),
                                                  histname.c_str(),
                                                  4096, 0, 4095);
      }

      for (FaceNum face; face.isValid(); face++) {
        FaceIdx faceIdx(xtalIdx, face);
        histname = genDACRatioProfName(faceIdx);
        m_dacRatioProfs[faceIdx] = new TProfile(histname.c_str(),
                                                histname.c_str(),
                                                4096, 0, 4095);
      }

      for (RngNum rng; rng.isValid(); rng++) 
        for (FaceNum face; face.isValid(); face++)
          {
            RngIdx rngIdx(xtalIdx, face,rng);
            histname = genADCHistName(rngIdx);
            m_adcHists[rngIdx] = new TH1S(histname.c_str(),
                                          histname.c_str(),
                                          4096, 0, 4095);

            if (rng <= HEX8) {
              FaceIdx faceIdx(xtalIdx,face);
              histname = genADCRatioProfName(rng, faceIdx);
              m_adcRatioProfs[rng][faceIdx] = new TProfile(histname.c_str(),
                                                           histname.c_str(),
                                                           4096, 0, 4095);
            }
          }
    }
}


void GCRHists::loadHists(const string &filename) {
  TFile histFile(filename.c_str(), "READ");

  //-- INIT SUMMARY HISTS  --//
  string histname;

  for (DiodeNum diode; diode.isValid(); diode++) {
    histname = genMeanDACSumHistName(diode);
    m_meanDACSumHist[diode] = CGCUtil::retrieveHist < TH1S > (histFile, histname);
    if (m_meanDACSumHist[diode])
      // move histogram into Global ROOT memory
      // so it is not deleted when input file is closed.
      // this may be a memory leak, i don't think
      // anyone cares.
      m_meanDACSumHist[diode]->SetDirectory(0);
  }

  for (RngNum rng; rng.isValid(); rng++) {
    histname = genMeanADCSumHistName(rng);
    m_meanADCSumHist[rng] = CGCUtil::retrieveHist < TH1S > (histFile, histname);
    if (m_meanADCSumHist[rng])
      m_meanADCSumHist[rng]->SetDirectory(0);
  }

  histname = genDACRatioSumProfName();
  m_dacRatioSumProf = CGCUtil::retrieveHist < TProfile > (histFile, histname);
  if (m_dacRatioSumProf)
    m_dacRatioSumProf->SetDirectory(0);

  for (RngNum rng; rng <= HEX8; rng++) {
    histname = genADCRatioSumProfName(rng);
    m_adcRatioSumProf[rng] = CGCUtil::retrieveHist < TProfile > (histFile, histname);
    if (m_adcRatioSumProf[rng])
      m_adcRatioSumProf[rng]->SetDirectory(0);
  }

  for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
    for (DiodeNum diode; diode.isValid(); diode++) {
      histname = genMeanDACHistName(diode, xtalIdx);
      TH1S *hist = CGCUtil::retrieveHist < TH1S > (histFile, histname);
      if (!hist) continue;
        
      // move histogram into Global ROOT memory
      // so it is not deleted when input file is closed.
      // this may be a memory leak, i don't think
      // anyone cares.
      hist->SetDirectory(0);

      m_meanDACHists[diode][xtalIdx] = hist;
    }

    for (RngNum rng; rng.isValid(); rng++) 
      for (FaceNum face; face.isValid(); face++) 
        {
          RngIdx rngIdx(xtalIdx, face, rng);
                
          histname = genADCHistName(rngIdx);

          TH1S *hist = CGCUtil::retrieveHist < TH1S > (histFile, histname);
          if (!hist) continue;
          hist->SetDirectory(0);

          m_adcHists[rngIdx] = hist;
        }

    for (FaceNum face; face.isValid(); face++) {
      FaceIdx faceIdx(xtalIdx, face);
      histname = genDACRatioProfName(faceIdx);

      TProfile *hist = CGCUtil::retrieveHist < TProfile > (histFile, histname);
      if (!hist) continue;
      hist->SetDirectory(0);

      m_dacRatioProfs[faceIdx] = hist;
    }

    for (RngNum rng; rng <= HEX8; rng++) 
      for (FaceNum face; face.isValid(); face++) {
        FaceIdx faceIdx(xtalIdx, face);
        histname = genADCRatioProfName(rng, faceIdx);

        TProfile *hist = CGCUtil::retrieveHist < TProfile > (histFile, histname);
        if (!hist) continue;
        hist->SetDirectory(0);

        m_adcRatioProfs[rng][faceIdx] = hist;
      }
  }
}

unsigned GCRHists::getMinEntries() const {
  unsigned      retVal   = ULONG_MAX;
  unsigned long sum      = 0;
  unsigned      n        = 0;
  unsigned      maxHits  = 0;


  for (DiodeNum diode; diode.isValid(); diode++)
    for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {

      unsigned nEntries = (summaryMode) ? 
        (unsigned)m_meanDACSumHist[diode]->GetEntries()
        : (unsigned)m_meanDACHists[diode][xtalIdx]->GetEntries();

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

void GCRHists::summarizeHists(ostream &ostrm) const {
  ostrm << "CHANNEL\tNHITS" << endl;
  for (DiodeNum diode; diode.isValid(); diode++)
    for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++)
      if (m_meanDACHists[diode][xtalIdx])
        ostrm << genMeanDACHistName(diode, xtalIdx) << "\t"
              << m_meanDACHists[diode][xtalIdx]->GetEntries()
              << endl;
}

string GCRHists::genMeanDACHistName(DiodeNum diode,
                                    XtalIdx xtalIdx) const {
  ostringstream tmp;

  tmp << "meanDAC_" << xtalIdx.toStr()
      << "_" << diode.toStr();
  return tmp.str();
}

string GCRHists::genADCHistName(RngIdx rngIdx) const {
  ostringstream tmp;

  tmp << "meanADC_" << rngIdx.toStr();
  return tmp.str();
}

string GCRHists::genDACRatioProfName(FaceIdx faceIdx) const {
  ostringstream tmp;

  tmp << "dacRatio_" << faceIdx.toStr();
  return tmp.str();
}

string GCRHists::genADCRatioProfName(RngNum rng,
                                     FaceIdx faceIdx) const {
  ostringstream tmp;

  tmp << "adcRatio_" << faceIdx.toStr()
      << "_" << rng.toStr();
  return tmp.str();
}

string GCRHists::genMeanDACSumHistName(DiodeNum diode) const {
  ostringstream tmp;

  tmp << "meanDACSum_" << diode.toStr();
  return tmp.str();
}

string GCRHists::genMeanADCSumHistName(RngNum rng) const {
  ostringstream tmp;

  tmp << "meanADCSum_" << rng.toStr();
  return tmp.str();
}

string GCRHists::genDACRatioSumProfName() const {
  ostringstream tmp;

  tmp << "dacRatioSum";
  return tmp.str();
}

string GCRHists::genADCRatioSumProfName(RngNum rng) const {
  ostringstream tmp;

  tmp << "adcRatioSum_" << rng.toStr();
  return tmp.str();
}

void GCRHists::trimHists() {
  for (DiodeNum diode; diode.isValid(); diode++)
    if (m_meanDACSumHist[diode])
      if (m_meanDACSumHist[diode]->GetEntries() == 0) {
        delete m_meanDACSumHist[diode];
        m_meanDACSumHist[diode] = 0;
      }

  for (RngNum rng; rng.isValid(); rng++)
    if (m_meanADCSumHist[rng])
      if (m_meanADCSumHist[rng]->GetEntries() == 0) {
        delete m_meanADCSumHist[rng];
        m_meanADCSumHist[rng] = 0;
      }

  if (m_dacRatioSumProf)
    if (m_dacRatioSumProf->GetEntries() == 0) {
      delete m_dacRatioSumProf;
      m_dacRatioSumProf = 0;
    }

  for (RngNum rng; rng <= HEX8; rng++)
    if (m_adcRatioSumProf[rng])
      if (m_adcRatioSumProf[rng]->GetEntries() == 0) {
        delete m_adcRatioSumProf[rng];
        m_adcRatioSumProf[rng] = 0;
      }

  //-- INIT PER-XTAL HISTS --//
  for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
    for (DiodeNum diode; diode.isValid(); diode++)
      if (m_meanDACHists[diode][xtalIdx])
        if (m_meanDACHists[diode][xtalIdx]->GetEntries() == 0) {
          delete m_meanDACHists[diode][xtalIdx];
          m_meanDACHists[diode][xtalIdx] = 0;
        }

    for (XtalRng xRng; xRng.isValid(); xRng++) {
      RngIdx rngIdx(xtalIdx, xRng);
      if (m_adcHists[rngIdx])
        if (m_adcHists[rngIdx]->GetEntries() == 0) {
          delete m_adcHists[rngIdx];
          m_adcHists[rngIdx] = 0;
        }
    }

    for (FaceNum face; face.isValid(); face++) {
      FaceIdx faceIdx(xtalIdx, face);
      if (m_dacRatioProfs[faceIdx])
        if (m_dacRatioProfs[faceIdx]->GetEntries() == 0) {
          delete m_dacRatioProfs[faceIdx];
          m_dacRatioProfs[faceIdx] = 0;
        }
    }

    for (RngNum rng; rng <= HEX8; rng++)
      for (FaceNum face; face.isValid(); face++) {
        FaceIdx faceIdx(xtalIdx, face);
        if (m_adcRatioProfs[rng][faceIdx])
          if (m_adcRatioProfs[rng][faceIdx]->GetEntries() == 0) {
            delete m_adcRatioProfs[rng][faceIdx];
            m_adcRatioProfs[rng][faceIdx] = 0;
          }
      }
  }
}

void GCRHists::fitHists(CalMPD &calMPD, const set<unsigned short> &zList) {
  // build peak ratio initial values & fitting limits

  // PER XTAL LOOP
  for (DiodeNum diode; diode.isValid(); diode++) {
    // temp process lrg diode only
    if (diode == SM_DIODE)
      continue;

    /// rough mevperdac guess.
    const float initialMPD = (diode == LRG_DIODE) ? .3 : 20;

    for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
      if (!m_meanDACHists[diode][xtalIdx])
        continue;

      TH1S & hist = *m_meanDACHists[diode][xtalIdx];
      // skip empty histograms
      if (hist.GetEntries() == 0)
        continue;

      MultiPeakGaus::fitHist(hist, zList, initialMPD);
      hist.Write();
    }

    if (!m_meanDACSumHist[diode])
      continue;

    TH1S &hist = *m_meanDACSumHist[diode];
    
    if (hist.GetEntries() == 0)
      continue;
    
    MultiPeakGaus::fitHist(hist, zList, initialMPD);
  }
}

void GCRHists::fillAdcRatio(RngIdx rngIdx, 
                            float thisADC,
                            float nextADC) {
  RngNum rng(rngIdx.getRng());
  FaceIdx faceIdx(rngIdx.getFaceIdx());

  if (m_adcRatioProfs[rng][faceIdx])
    m_adcRatioProfs[rng][faceIdx]->Fill(thisADC, nextADC);

  m_adcRatioSumProf[rng]->Fill(thisADC, nextADC);
}

void GCRHists::fillDacRatio(FaceIdx faceIdx,
                            float leDAC,
                            float heDAC) {
  if (m_dacRatioProfs[faceIdx])
    m_dacRatioProfs[faceIdx]->Fill(leDAC, heDAC);

  m_dacRatioSumProf->Fill(leDAC, heDAC);
}


void GCRHists::fillADCHit(RngIdx rngIdx,
                          float adc) {
  RngNum rng(rngIdx.getRng());
        
  if (!summaryMode)
    if (m_adcHists[rngIdx])
      m_adcHists[rngIdx]->Fill(adc);
        
  m_meanADCSumHist[rng]->Fill(adc);

}

void GCRHists::fillMeanCIDAC(DiodeNum diode, XtalIdx xtalIdx, float cidac) {
  if (!summaryMode)
    if (m_meanDACHists[diode][xtalIdx])
      m_meanDACHists[diode][xtalIdx]->Fill(cidac);

  m_meanDACSumHist[diode]->Fill(cidac);
}

void GCRHists::setHistDir(TDirectory *dir) {
  for (DiodeNum diode; diode.isValid(); diode++)
    if (m_meanDACSumHist[diode])
        m_meanDACSumHist[diode]->SetDirectory(dir);

  for (RngNum rng; rng.isValid(); rng++)
    if (m_meanADCSumHist[rng])
        m_meanADCSumHist[rng]->SetDirectory(dir);

  if (m_dacRatioSumProf)
      m_dacRatioSumProf->SetDirectory(dir);

  for (RngNum rng; rng <= HEX8; rng++)
    if (m_adcRatioSumProf[rng])
        m_adcRatioSumProf[rng]->SetDirectory(dir);

  //-- INIT PER-XTAL HISTS --//
  for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
    for (DiodeNum diode; diode.isValid(); diode++)
      if (m_meanDACHists[diode][xtalIdx])
          m_meanDACHists[diode][xtalIdx]->SetDirectory(dir);

    for (XtalRng xRng; xRng.isValid(); xRng++) {
      RngIdx rngIdx(xtalIdx, xRng);
      if (m_adcHists[rngIdx])
          m_adcHists[rngIdx]->SetDirectory(dir);
    }

    for (FaceNum face; face.isValid(); face++) {
      FaceIdx faceIdx(xtalIdx, face);
      if (m_dacRatioProfs[faceIdx])
          m_dacRatioProfs[faceIdx]->SetDirectory(dir);
    }

    for (RngNum rng; rng <= HEX8; rng++)
      for (FaceNum face; face.isValid(); face++) {
        FaceIdx faceIdx(xtalIdx, face);
        if (m_adcRatioProfs[rng][faceIdx])
            m_adcRatioProfs[rng][faceIdx]->SetDirectory(dir);
      }
  }
}
