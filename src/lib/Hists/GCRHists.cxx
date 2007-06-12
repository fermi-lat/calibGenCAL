// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Hists/GCRHists.cxx,v 1.10 2007/06/07 17:45:43 fewtrell Exp $

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "GCRHists.h"
#include "../Util/CGCUtil.h"


// GLAST INCLUDES

// EXTLIB INCLUDES
#include "TH1S.h"
#include "TProfile.h"

// STD INCLUDES
#include <cmath>
#include <sstream>
#include <map>
#include <cassert>
#include <iostream>
#include <algorithm>

using namespace std;
using namespace CalUtil;
  
namespace calibGenCAL {

  using namespace CGCUtil;
  using namespace singlex16;

  namespace {
    static const unsigned short MAX_INFERREDZ = 30;
    static const string MEANDAC_HISTNAME("meanDAC");
    static const string MEANDACALLZ_HISTNAME("meanDACAllZ");
    static const string MEANDACSUM_HISTNAME("meanDACSummary");
    static const string MEANDACSUMALLZ_HISTNAME("meanDACSumAllZ");
    static const string Z_HISTNAME("zHist");
    static const string DACRATIOSUM_HISTNAME("dacRatioSummary");
    static const string ADCRATIOSUM_HISTNAME("adcRatioSummary");
    static const string DACRATIO_HISTNAME("dacRatio");
    static const string ADCRATIO_HISTNAME("adcRatio");
  }


  GCRHists::GCRHists(const bool summaryMode,
                     TDirectory *const writeDir,
                     TDirectory *const readDir) :
    m_summaryMode(summaryMode),
    m_writeDir(writeDir)
  {
    if (readDir != 0)
      loadHists(*readDir);
    else
      initHists();
    
    /// move loaded histograms to output dir.
    setDirectory(writeDir);
  }

  void GCRHists::setDirectory(TDirectory *const dir) {
    if (!m_summaryMode) {
      m_meanDACHists->setDirectory(dir);
      m_meanDACAllZHists->setDirectory(dir);
      m_dacRatioProfs->setDirectory(dir);
      m_adcRatioProfs->setDirectory(dir);    
    }

    m_meanDACSumHists->setDirectory(dir);
    m_adcRatioSumProfs->setDirectory(dir);

    m_meanDACSumAllZ->SetDirectory(dir);
    m_zHist->SetDirectory(dir);
    m_dacRatioSumProf->SetDirectory(dir);
  }

  void GCRHists::loadHists(TDirectory &readDir) {
    // init histogram groups
    if (!m_summaryMode) {
      m_meanDACHists.reset(new MeanDACHistCol(MEANDAC_HISTNAME.c_str(),
                                              m_writeDir,
                                              &readDir,
                                              500,
                                              0,
                                              0));
      m_meanDACAllZHists.reset(new MeanDACAllZHistCol(MEANDACALLZ_HISTNAME.c_str(),
                                                      m_writeDir,
                                                      &readDir,
                                                      500,
                                                      0,
                                                      0));
      m_dacRatioProfs.reset(new DACRatioProfCol(DACRATIO_HISTNAME.c_str(),
                                                m_writeDir,
                                                &readDir,
                                                200,
                                                0,
                                                0));
      m_adcRatioProfs.reset(new ADCRatioProfCol(ADCRATIO_HISTNAME.c_str(),
                                                m_writeDir,
                                                &readDir,
                                                200,
                                                0,
                                                0));
    }

    m_meanDACSumHists.reset(new MeanDACSumHistCol(MEANDACSUM_HISTNAME.c_str(),
                                                  m_writeDir,
                                                  &readDir,
                                                  500,
                                                  0,
                                                  0));
    m_adcRatioSumProfs.reset(new ADCRatioSumProfCol(ADCRATIOSUM_HISTNAME.c_str(),
                                                    m_writeDir,
                                                    &readDir,
                                                    200,
                                                    0,
                                                    0));
    //load individual histograms
    m_meanDACSumAllZ = CGCUtil::retrieveROOTObj<TH1S>(readDir, MEANDACSUMALLZ_HISTNAME);
    m_zHist = CGCUtil::retrieveROOTObj<TH1S>(readDir, Z_HISTNAME);
    m_dacRatioSumProf = CGCUtil::retrieveROOTObj<TProfile>(readDir, DACRATIOSUM_HISTNAME);
  }

  void GCRHists::initHists() {
    // init histogram groups
    if (!m_summaryMode) {
      m_meanDACHists.reset(new MeanDACHistCol(MEANDAC_HISTNAME.c_str(),
                                              m_writeDir,
                                              0,
                                              500,
                                              0,
                                              0));
      m_meanDACAllZHists.reset(new MeanDACAllZHistCol(MEANDACALLZ_HISTNAME.c_str(),
                                                      m_writeDir,
                                                      0,
                                                      500,
                                                      0,
                                                      0));
      m_dacRatioProfs.reset(new DACRatioProfCol(DACRATIO_HISTNAME.c_str(),
                                                m_writeDir,
                                                0,
                                                200,
                                                0,
                                                0));
      m_adcRatioProfs.reset(new ADCRatioProfCol(ADCRATIO_HISTNAME.c_str(),
                                                m_writeDir,
                                                0,
                                                200,
                                                0,
                                                0));
    }
    m_meanDACSumHists.reset(new MeanDACSumHistCol(MEANDACSUM_HISTNAME.c_str(),
                                                  m_writeDir,
                                                  0,
                                                  500,
                                                  0,
                                                  0));
    m_adcRatioSumProfs.reset(new ADCRatioSumProfCol(ADCRATIOSUM_HISTNAME.c_str(),
                                                    m_writeDir,
                                                    0,
                                                    200,
                                                    0,
                                                    0));
    // init individual histograms
    m_meanDACSumAllZ = new TH1S(MEANDACSUMALLZ_HISTNAME.c_str(),
                                MEANDACSUMALLZ_HISTNAME.c_str(),
                                500,
                                0,
                                0);
    
    m_zHist = new TH1S(Z_HISTNAME.c_str(),
                       Z_HISTNAME.c_str(),
                       MAX_INFERREDZ+1,
                       0,
                       MAX_INFERREDZ+1);
    
    m_dacRatioSumProf = new TProfile(DACRATIOSUM_HISTNAME.c_str(),
                                     DACRATIOSUM_HISTNAME.c_str(),
                                     500,
                                     0,
                                     0,
                                     "");
  }

  void GCRHists::summarizeHists(ostream &ostrm) const {
    ostrm << "HIT SUMMARY" << endl;
    ostrm << "INFERREDZ\tDIODE_CLASS\tNHITS" << endl;

    for (MeanDACSumHistCol::const_iterator it(m_meanDACSumHists->begin());
         it != m_meanDACSumHists->end();
         it++) {
      const unsigned short inferredZ(it->first.getInferredZ());
      const DiodeNum diode(it->first.getDiode());

      ostrm << inferredZ << " " 
            << diode.toStr() << " "
            << it->second->GetEntries() 
            << endl;
    }
    

    if (!m_summaryMode) {
      ostrm << "PER CHANNEL HITS" << endl;
      ostrm << "INFERREDZ\tXTAL\tDIODE\tNHITS" << endl;

      for (MeanDACHistCol::const_iterator it(m_meanDACHists->begin());
           it != m_meanDACHists->end();
           it++) {
        const unsigned short inferredZ(it->first.getInferredZ());
        const XtalIdx xtalIdx(it->first.getXtalIdx());
        const DiodeNum diode(it->first.getDiode());


        ostrm << inferredZ << " "
              << xtalIdx.toStr() << " "
              << diode.toStr() << " "
              << it->second->GetEntries()
              << endl;
      }
    }
  }
  

  void GCRHists::fillAdcRatio(const RngIdx rngIdx, 
                              const float thisADC,
                              const float nextADC) {
    RngNum rng(rngIdx.getRng());

    /// programmer error
    assert(rng != HEX1);

    if (!m_summaryMode)
      m_adcRatioProfs->produceHist(rngIdx).Fill(thisADC, nextADC);
    m_adcRatioSumProfs->produceHist(rng).Fill(thisADC, nextADC);
  }

  void GCRHists::fillDACRatio(const FaceIdx faceIdx,
                              const float leDAC,
                              const float heDAC) {
    if (!m_summaryMode)
      m_dacRatioProfs->produceHist(faceIdx).Fill(leDAC, heDAC);
  
    /// this one has only one entry, so index parm is not important.
    m_dacRatioSumProf->Fill(leDAC, heDAC);
  }


  void GCRHists::fillMeanCIDACZ(const XtalIdx xtalIdx, 
                                const DiodeNum diode,
                                const unsigned short inferredZ, 
                                const float cidac) {
    if (!m_summaryMode)
      m_meanDACHists->produceHist(MeanDacZId(inferredZ,xtalIdx,diode)).Fill(cidac);

    m_meanDACSumHists->produceHist(ZDiodeId(inferredZ,diode)).Fill(cidac);
    m_zHist->Fill(inferredZ);
  }

  void GCRHists::fillMeanCIDAC(const XtalIdx xtalIdx, 
                               const DiodeNum diode,
                               const float cidac) {
    if (!m_summaryMode)
      m_meanDACAllZHists->produceHist(MeanDACId(xtalIdx, diode)).Fill(cidac);

    m_meanDACSumAllZ->Fill(cidac);
  }



}; // namespace calibGenCAL
