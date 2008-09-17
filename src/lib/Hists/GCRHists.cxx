// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Hists/GCRHists.cxx,v 1.15 2008/08/04 14:54:59 fewtrell Exp $

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "GCRHists.h"
#include "src/lib/Util/ROOTUtil.h"
#include "src/lib/Specs/CalResponse.h"


// GLAST INCLUDES

// EXTLIB INCLUDES
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

namespace {
  static const unsigned short MAX_INFERREDZ = 30;
}
  
namespace calibGenCAL {

  /// construct from string repr
  XtalDiodeId::XtalDiodeId(const string &name) {
    /// break name up into component parts
    const vector<string> parts(tokenize_str(name, string("_")));

    if (parts.size() != N_FIELDS)
      throw runtime_error("Invalid XtalDiodeId string repr: " + name);

    const XtalIdx xtalIdx(parts[0]);

    const DiodeNum diode(parts[1]);
    
    m_data = calc(xtalIdx, diode);
    
  }

  /// construct from string repr
  ZDiodeId::ZDiodeId(const string &name) {
    /// break name up into component parts
    const vector<string> parts(tokenize_str(name, string("_Z")));

    if (parts.size() != N_FIELDS)
      throw runtime_error("Invalid ZDiodeId string repr: " + name);

    // extract each component
    const DiodeNum diode(parts[0]);

    istringstream z_strm(parts[1]);
    unsigned short z;
    z_strm >> z;

    /// calculate index
    m_data = calc(z, diode);
  }


  namespace {
    static const string MEANDAC_HISTNAME("meanDAC");
    static const string MEV_HISTNAME("mev");
    static const string MEVSUM_HISTNAME("mevSum");
    static const string MEVSUMZ_HISTNAME("mevSumZ");
    static const string MEVSUMLYR_HISTNAME("mevSumLyr");
    static const string Z_HISTNAME("zHist");
    static const string DACRATIOSUM_HISTNAME("dacRatioSummary");
    static const string DACRATIO_HISTNAME("dacRatio");
  }


  GCRHists::GCRHists(const bool summaryMode,
                     const bool mevMode,
                     TDirectory *const writeDir,
                     TDirectory *const readDir) :
    m_dacRatioSumProf(0),
    m_summaryMode(summaryMode),
    m_mevMode(mevMode),
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
      m_dacRatioProfs->setDirectory(dir);
    }

    if (m_mevMode) {
      m_mevSumHists->setDirectory(dir);
      m_mevSumLyrHists->setDirectory(dir);
      m_mevSumZHists->setDirectory(dir);
    }

    m_zHist->SetDirectory(dir);
    m_dacRatioSumProf->SetDirectory(dir);
  }

  void GCRHists::loadHists(TDirectory &readDir) {
    // init histogram groups
    if (!m_summaryMode) {
      m_meanDACHists.reset(new MeanDACHistCol(MEANDAC_HISTNAME,
                                              m_writeDir,
                                              &readDir,
                                              4096,
                                              0,
                                              4096));
      m_dacRatioProfs.reset(new DACRatioProfCol(DACRATIO_HISTNAME,
                                                m_writeDir,
                                                &readDir,
                                                512,
                                                0,
                                                4096));
    }

    if (m_mevMode) {
      m_mevSumHists.reset(new MeVSumHistCol(MEVSUM_HISTNAME,
                                            m_writeDir,
                                            &readDir,
                                            15000,
                                            0,
                                            15000));
      m_mevSumLyrHists.reset(new MeVSumLyrHistCol(MEVSUMLYR_HISTNAME,
                                                  m_writeDir,
                                                  &readDir,
                                                  1500,
                                                  0,
                                                  15000));
      m_mevSumZHists.reset(new MeVSumZHistCol(MEVSUMZ_HISTNAME,
                                                  m_writeDir,
                                                  &readDir,
                                                  1500,
                                                  0,
                                                  15000));

    }


    m_zHist = retrieveROOTObj<TH1I>(readDir, Z_HISTNAME);
    m_dacRatioSumProf = retrieveROOTObj<TProfile>(readDir,
                                                  DACRATIOSUM_HISTNAME);
  }

  void GCRHists::initHists() {
    // init histogram groups
    if (!m_summaryMode) {
      m_meanDACHists.reset(new MeanDACHistCol(MEANDAC_HISTNAME,
                                              m_writeDir,
                                              0,
                                              4096,
                                              0,
                                              4096));
      m_dacRatioProfs.reset(new DACRatioProfCol(DACRATIO_HISTNAME,
                                                m_writeDir,
                                                0,
                                                512,
                                                0,
                                                4096));
    }
    if (m_mevMode) {
      m_mevSumHists.reset(new MeVSumHistCol(MEVSUM_HISTNAME,
                                            m_writeDir,
                                            0,
                                            15000,
                                            0,
                                            15000));
      m_mevSumLyrHists.reset(new MeVSumLyrHistCol(MEVSUMLYR_HISTNAME,
                                                  m_writeDir,
                                                  0,
                                                  1500,
                                                  0,
                                                  15000));
      m_mevSumZHists.reset(new MeVSumZHistCol(MEVSUMZ_HISTNAME,
                                                  m_writeDir,
                                                  0,
                                                  1500,
                                                  0,
                                                  15000));
    }

    m_zHist = new TH1I(Z_HISTNAME.c_str(),
                       Z_HISTNAME.c_str(),
                       MAX_INFERREDZ+1,
                       0,
                       MAX_INFERREDZ+1);
    
    m_dacRatioSumProf = new TProfile(DACRATIOSUM_HISTNAME.c_str(),
                                     DACRATIOSUM_HISTNAME.c_str(),
                                     512,
                                     0,
                                     4096,
                                     "");
  }

  void GCRHists::summarizeHists(ostream &ostrm) const {
    ostrm << "HIT SUMMARY" << endl;
    ostrm << "DIODE_CLASS\tNHITS" << endl;

    for (MeVSumHistCol::index_type idx;
         idx.isValid();
         idx++) {
      if (m_mevSumHists->getHist(idx))
        ostrm << idx.toStr() << " "
              << m_mevSumHists->getHist(idx)->GetEntries() 
              << endl;
    }
    

    if (!m_summaryMode) {
      ostrm << "PER CHANNEL HITS" << endl;
      ostrm << "XTAL\tDIODE\tNHITS" << endl;

      for (MeanDACHistCol::index_type idx;
           idx.isValid();
           idx++) {
        const XtalIdx xtalIdx(idx.getXtalIdx());
        const DiodeNum diode(idx.getDiode());

        if (m_meanDACHists->getHist(idx))
          ostrm << xtalIdx.toStr() << " "
                << diode.toStr() << " "
                << m_meanDACHists->getHist(idx)->GetEntries()
                << endl;
      }
    }
  }
  

  void GCRHists::fillDACRatio(const FaceIdx faceIdx,
                              const float leDAC,
                              const float heDAC) {
    if (!m_summaryMode)
      m_dacRatioProfs->produceHist(faceIdx).Fill(leDAC, heDAC);
  
    /// this one has only one entry, so index parm is not important.
    m_dacRatioSumProf->Fill(leDAC, heDAC);
  }


  void GCRHists::fillMeanCIDAC(const XtalIdx xtalIdx, 
                               const DiodeNum diode,
                               const float cidac) {
    if (!m_summaryMode)
      m_meanDACHists->produceHist(XtalDiodeId(xtalIdx, diode)).Fill(cidac);
  }
  

  void GCRHists::fillMeV(const XtalIdx xtalIdx, 
                         const DiodeNum diode,
                         const float mev,
                         const unsigned short inferredZ) {
    if (m_mevMode) {
      m_mevSumHists->produceHist(diode).Fill(mev);
      m_mevSumLyrHists->produceHist(LyrDiodeId(xtalIdx.getLyr(), diode)).Fill(mev);
      if (inferredZ>0)
        m_mevSumZHists->produceHist(ZDiodeId(inferredZ, diode)).Fill(mev);
    }

    if (inferredZ>0)
      m_zHist->Fill(inferredZ);

  }

}; // namespace calibGenCAL
