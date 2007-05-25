// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Hists/GCRHists.cxx,v 1.8 2007/04/26 19:46:56 fewtrell Exp $

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "GCRHists.h"
#include "../Util/CGCUtil.h"
//#include "../Util/MultiPeakGaus.h"

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
                     const string &rootFilePath) :
    summaryMode(summaryMode)
  {
    /// build new empty histograms
    if (rootFilePath.empty())
      initHists();
    /// load histograms from previous run.
    else 
      loadHists(rootFilePath);
  }

  void GCRHists::loadHists(const string &filename) {
    TFile rootFile(filename.c_str(), "READ");
    // init histogram groups
    if (!summaryMode) {
      m_meanDACHists.reset(new MeanDACHistCol(MEANDAC_HISTNAME.c_str(),
                                              500,
                                              0,
                                              0,
                                              &rootFile));
      m_meanDACAllZHists.reset(new MeanDACAllZHistCol(MEANDACALLZ_HISTNAME.c_str(),
                                                      500,
                                                      0,
                                                      0,
                                                      &rootFile));

      m_dacRatioProfs.reset(new DACRatioProfCol(DACRATIO_HISTNAME.c_str(),
                                                200,
                                                0,
                                                0,
                                                &rootFile));
      m_adcRatioProfs.reset(new ADCRatioProfCol(ADCRATIO_HISTNAME.c_str(),
                                                200,
                                                0,
                                                0,
                                                &rootFile));
    }

    m_meanDACSumHists.reset(new MeanDACSumHistCol(MEANDACSUM_HISTNAME.c_str(),
                                                  500,
                                                  0,
                                                  0,
                                                  &rootFile));
    m_adcRatioSumProfs.reset(new ADCRatioSumProfCol(ADCRATIOSUM_HISTNAME.c_str(),
                                                    200,
                                                    0,
                                                    0,
                                                    &rootFile));

    //load individual histograms
    m_meanDACSumAllZ = CGCUtil::retrieveROOTObj<TH1S>(rootFile, MEANDACSUMALLZ_HISTNAME);
    m_zHist = CGCUtil::retrieveROOTObj<TH1S>(rootFile, Z_HISTNAME);
    m_dacRatioSumProf = CGCUtil::retrieveROOTObj<TProfile>(rootFile, DACRATIOSUM_HISTNAME);

    // input file will go away, need to set all histograms to global directory
    setHistDir(0);
  }

  void GCRHists::initHists() {
    // init histogram groups
    if (!summaryMode) {
      m_meanDACHists.reset(new MeanDACHistCol(MEANDAC_HISTNAME.c_str(),
                                              500,
                                              0,
                                              0));
      m_meanDACAllZHists.reset(new MeanDACAllZHistCol(MEANDACALLZ_HISTNAME.c_str(),
                                                      500,
                                                      0,
                                                      0));
      m_dacRatioProfs.reset(new DACRatioProfCol(DACRATIO_HISTNAME.c_str(),
                                                200,
                                                0,
                                                0));
      m_adcRatioProfs.reset(new ADCRatioProfCol(ADCRATIO_HISTNAME.c_str(),
                                                200,
                                                0,
                                                0));
    }
    m_meanDACSumHists.reset(new MeanDACSumHistCol(MEANDACSUM_HISTNAME.c_str(),
                                                  500,
                                                  0,
                                                  0));
    m_adcRatioSumProfs.reset(new ADCRatioSumProfCol(ADCRATIOSUM_HISTNAME.c_str(),
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
      const unsigned short inferredZ(it->first.inferredZ);
      const DiodeNum diode(it->first.diode);

      ostrm << inferredZ << " " 
            << diode.toStr() << " "
            << it->second->GetEntries() 
            << endl;
    }
    

    if (!summaryMode) {
      ostrm << "PER CHANNEL HITS" << endl;
      ostrm << "INFERREDZ\tXTAL\tDIODE\tNHITS" << endl;

      for (MeanDACHistCol::const_iterator it(m_meanDACHists->begin());
           it != m_meanDACHists->end();
           it++) {
        const unsigned short inferredZ(it->first.inferredZ);
        const XtalIdx xtalIdx(it->first.xtalIdx);
        const DiodeNum diode(it->first.diode);


        ostrm << inferredZ << " "
              << xtalIdx.toStr() << " "
              << diode.toStr() << " "
              << it->second->GetEntries()
              << endl;
      }
    }
  }

  void GCRHists::fitHists(CalMPD &calMPD) {
    // build peak ratio initial values & fitting limits

    // PER XTAL LOOP
    for (DiodeNum diode; diode.isValid(); diode++) {

      if (!summaryMode)
        for (MeanDACHistCol::iterator it(m_meanDACHists->begin());
             it != m_meanDACHists->end();
             it++) {
           
          //const unsigned short inferredZ(it->first.inferredZ);
          const DiodeNum diode(it->first.diode);
          TH1S &hist(*(it->second));
           
          //MultiPeakGaus::fitHist(hist, diode, inferredZ);
          hist.Fit("gaus","Q");
          hist.Write();
          cout << hist.GetName() << " " 
               << hist.GetFunction("gaus")->GetParameter(1) << " "
               << hist.GetFunction("gaus")->GetParameter(2) << " "
               << endl;
           
        }

      for (MeanDACSumHistCol::iterator it(m_meanDACSumHists->begin());
           it != m_meanDACSumHists->end();
           it++) {

        //const unsigned short inferredZ(it->first.inferredZ);
        const DiodeNum diode(it->first.diode);
        TH1S &hist = *(it->second);
      
        //MultiPeakGaus::fitHist(hist, diode, inferredZ);
        hist.Fit("gaus","Q");
        hist.Write();
        cout << hist.GetName() << " " 
             << hist.GetFunction("gaus")->GetParameter(1) << " "
             << hist.GetFunction("gaus")->GetParameter(2) << " "
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

    if (!summaryMode)
      m_adcRatioProfs->checkNGet(rngIdx).Fill(thisADC, nextADC);
    m_adcRatioSumProfs->checkNGet(rng).Fill(thisADC, nextADC);
  }

  void GCRHists::fillDACRatio(const FaceIdx faceIdx,
                              const float leDAC,
                              const float heDAC) {
    if (!summaryMode)
      m_dacRatioProfs->checkNGet(faceIdx).Fill(leDAC, heDAC);
  
    /// this one has only one entry, so index parm is not important.
    m_dacRatioSumProf->Fill(leDAC, heDAC);
  }


  void GCRHists::fillMeanCIDACZ(const XtalIdx xtalIdx, 
                                const DiodeNum diode,
                                const unsigned short inferredZ, 
                                const float cidac) {
    if (!summaryMode)
      m_meanDACHists->checkNGet(MeanDACHistId(inferredZ,xtalIdx,diode)).Fill(cidac);

    m_meanDACSumHists->checkNGet(MeanDACSumHistId(inferredZ,diode)).Fill(cidac);
    m_zHist->Fill(inferredZ);
  }

  void GCRHists::fillMeanCIDAC(const XtalIdx xtalIdx, 
                               const DiodeNum diode,
                               const float cidac) {
    if (!summaryMode)
      m_meanDACAllZHists->checkNGet(MeanDACAllZHistId(xtalIdx, diode)).Fill(cidac);

    m_meanDACSumAllZ->Fill(cidac);
  }


  void GCRHists::setHistDir(TDirectory *const dir) {
    if (m_meanDACHists.get() != 0)
      m_meanDACHists->setHistDir(dir);

    if (m_meanDACAllZHists.get() != 0)
      m_meanDACAllZHists->setHistDir(dir);

    if (m_dacRatioProfs.get() != 0)
      m_dacRatioProfs->setHistDir(dir);

    if (m_adcRatioProfs.get() != 0)
      m_adcRatioProfs->setHistDir(dir);

  
    m_meanDACSumHists->setHistDir(dir);

    m_meanDACSumAllZ->SetDirectory(dir);

    m_zHist->SetDirectory(dir);

    m_dacRatioSumProf->SetDirectory(dir);

    m_adcRatioSumProfs->setHistDir(dir);
  }

  string GCRHists::MeanDACHistId::toStr() const {
    ostringstream tmp;
    tmp << xtalIdx.toStr() + "_" + diode.toStr() + "_Z" << inferredZ;
    return tmp.str();
  }

  string GCRHists::MeanDACAllZHistId::toStr() const {
    ostringstream tmp;
    tmp << xtalIdx.toStr() + "_" + diode.toStr();
    return tmp.str();
  }

  string GCRHists::MeanDACSumHistId::toStr() const {
    ostringstream tmp;
    tmp << diode.toStr() + "_Z" << inferredZ;
    return tmp.str();
  }

}; // namespace calibGenCAL
