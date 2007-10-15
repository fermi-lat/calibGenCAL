// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Util/RootFileAnalysis.cxx,v 1.5 2007/06/13 22:42:13 fewtrell Exp $

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "RootFileAnalysis.h"
#include "CGCUtil.h"

// GLAST INCLUDES
#include "digiRootData/DigiEvent.h"
#include "reconRootData/ReconEvent.h"
#include "mcRootData/McEvent.h"
#include "gcrSelectRootData/GcrSelectEvent.h"

// EXTLIB INCLUDES
#include "TChainElement.h"
#include "TStreamerInfo.h"

// STD INCLUDES
#include <vector>
#include <string>
#include <ostream>

namespace calibGenCAL {

  using namespace std;

  RootFileAnalysis::RootFileAnalysis(const vector<string> *mcFilenames,
                                     const vector<string> *digiFilenames,
                                     const vector<string> *reconFilenames,
                                     const vector<string> *svacFilenames,
                                     const vector<string> *gcrSelectFilenames) :
    m_mcChain("MC"),
    m_mcEvt(0),
    m_digiChain("Digi"),
    m_digiEvt(0),
    m_reconChain("Recon"),
    m_reconEvt(0),
    m_svacChain("Output"),
    m_gcrSelectChain("GcrSelect"),
    m_gcrSelectEvt(0)

  {
    // add mc file list into mc ROOT chain
    if (mcFilenames) {
      for (vector < string > ::const_iterator itr = mcFilenames->begin();
           itr != mcFilenames->end(); ++itr) {
        LogStrm::get() << "mc file added to chain: " << *itr << endl;
        m_mcChain.Add(itr->c_str());
      }
      m_mcChain.SetBranchAddress("McEvent", &m_mcEvt);
      m_chainArr.Add(&m_mcChain);
    }

    // add digi file list into digi ROOT chain
    if (digiFilenames) {
      for (vector < string > ::const_iterator itr = digiFilenames->begin();
           itr != digiFilenames->end(); ++itr) {
        LogStrm::get() << "digi file added to chain: " << *itr << endl;
        m_digiChain.Add(itr->c_str());
      }
      m_digiChain.SetBranchAddress("DigiEvent", &m_digiEvt);
      m_chainArr.Add(&m_digiChain);
    }

    // add recon file list into recon ROOT chain
    if (reconFilenames) {
      for (vector < string > ::const_iterator itr = reconFilenames->begin();
           itr != reconFilenames->end(); ++itr) {
        LogStrm::get() << "recon file added to chain: " << *itr << endl;
        m_reconChain.Add(itr->c_str());
      }
      m_reconChain.SetBranchAddress("ReconEvent", &m_reconEvt);
      m_chainArr.Add(&m_reconChain);
    }

    // add svac file list into svac ROOT chain
    if (svacFilenames) {
      for (vector < string > ::const_iterator itr = svacFilenames->begin();
           itr != svacFilenames->end(); ++itr) {
        LogStrm::get() << "svac file added to chain: " << *itr << endl;
        m_svacChain.Add(itr->c_str());
      }
      m_chainArr.Add(&m_svacChain);
    }

    // add gcrSelect file list into mc ROOT chain
    if (gcrSelectFilenames) {
      for (vector < string > ::const_iterator itr = gcrSelectFilenames->begin();
           itr != gcrSelectFilenames->end(); ++itr) {
        LogStrm::get() << "gcrSelect file added to chain: " << *itr << endl;
        m_gcrSelectChain.Add(itr->c_str());
      }
      m_gcrSelectChain.SetBranchAddress("GcrSelectEvent", &m_gcrSelectEvt);
      m_chainArr.Add(&m_gcrSelectChain);
    }


  }

  RootFileAnalysis::~RootFileAnalysis() {
    if (m_mcEvt) {
      m_mcEvt->Clear();
      delete m_mcEvt;
    }

    if (m_digiEvt) {
      m_digiEvt->Clear();
      delete m_digiEvt;
    }

    if (m_reconEvt) {
      m_reconEvt->Clear();
      delete m_reconEvt;
    }

    if (m_gcrSelectEvt) {
      m_gcrSelectEvt->Clear();
      delete m_gcrSelectEvt;
    }
  }

  UInt_t RootFileAnalysis::getEvent(UInt_t iEvt) {
    // delete any old event data.
    if (m_mcEvt)
      m_mcEvt->Clear();

    if (m_digiEvt)
      m_digiEvt->Clear();

    if (m_reconEvt)
      m_reconEvt->Clear();

    if (m_gcrSelectEvt)
      m_gcrSelectEvt->Clear();

    UInt_t nBytes = 0;
    // if using chains, check the array of chains and move
    // the event pointer to the Req event
    for (int i = 0; i < m_chainArr.GetEntries(); i++)
      nBytes += ((TChain *)m_chainArr.At(i))->GetEvent(iEvt);

    m_nextEvt++;
    return nBytes;
  }

  UInt_t RootFileAnalysis::getEntries() const {
    // Purpose and Method:  Determine the number of events to iterate over
    //   checking to be sure that the Req number of events is less than
    //   the min number of events in all files

    UInt_t nEntries = 0;

    nEntries = (int)(((TChain *)m_chainArr.At(0))->GetEntries());
    for (int i = 1; i < m_chainArr.GetEntries(); i++)
      nEntries = min(nEntries, (UInt_t)((TChain *)m_chainArr.At(i))->GetEntries());
    return nEntries;
  }

}; // namespace calibGenCAL
