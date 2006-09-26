// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/RootFileAnalysis.cxx,v 1.3 2006/09/21 19:30:00 fewtrell Exp $
/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "RootFileAnalysis.h"

// GLAST INCLUDES
#include "digiRootData/DigiEvent.h"
#include "reconRootData/ReconEvent.h"
#include "mcRootData/McEvent.h"


// EXTLIB INCLUDES
#include "TChainElement.h"
#include "TFile.h"

// STD INCLUDES
#include <vector>
#include <string>
#include <ostream>


using namespace std;

RootFileAnalysis::RootFileAnalysis(const vector<string> *mcFilenames,
                                   const vector<string> *digiFilenames,
                                   const vector<string> *reconFilenames,
                                   ostream &ostrm) :
  m_mcChain("mc"),
  m_mcEvt(0),
  m_digiChain("Digi"),
  m_digiEvt(0),
  m_reconChain("recon"),
  m_reconEvt(0),
  m_ostrm(ostrm)
{
 
  // add mc file list into mc ROOT chain
  if (mcFilenames) {
    for(vector<string>::const_iterator itr = mcFilenames->begin();
        itr != mcFilenames->end(); ++itr) {
      m_ostrm << "mc file added to chain: " << *itr << endl;
      m_mcChain.Add(itr->c_str());
    }
    m_mcChain.SetBranchAddress("McEvent",&m_mcEvt);
    m_chainArr.Add(&m_mcChain);
  }

  // add digi file list into mc ROOT chain
  if (digiFilenames) {
    for(vector<string>::const_iterator itr = digiFilenames->begin();
        itr != digiFilenames->end(); ++itr) {
      m_ostrm << "digi file added to chain: " << *itr << endl;
      m_digiChain.Add(itr->c_str());
    }
    m_digiChain.SetBranchAddress("DigiEvent",&m_digiEvt);
    m_chainArr.Add(&m_digiChain);
 
    // Check DigiEvt structure Ver
    TIter fileListIter(m_digiChain.GetListOfFiles());
    int codeDigiEvtVer =
      DigiEvent::Class()->GetClassVersion();

    while (TChainElement *curElement = (TChainElement*)fileListIter.Next()) {
      const char *curFilenames = curElement->GetTitle();
      TFile       curFile(curFilenames);
      int fileDigiEvtVer = ((TStreamerInfo*)curFile.GetStreamerInfoList()->FindObject("DigiEvent"))->GetClassVersion();

      if (fileDigiEvtVer != codeDigiEvtVer) {
        m_ostrm << "WARNING: digFile=" << curFilenames << " created with DigiEvent version"
                << fileDigiEvtVer << " code is linked to DigiEvent version"
                << codeDigiEvtVer << endl;
      }
    }
  }

  // add recon file list into mc ROOT chain
  if (reconFilenames) {
    for(vector<string>::const_iterator itr = reconFilenames->begin();
        itr != reconFilenames->end(); ++itr) {
      m_ostrm << "recon file added to chain: " << *itr << endl;
      m_reconChain.Add(itr->c_str());
    }
    m_reconChain.SetBranchAddress("ReconEvent",&m_reconEvt);
    m_chainArr.Add(&m_reconChain);
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


}

UInt_t RootFileAnalysis::getEvent(UInt_t iEvt) {
  // delete any old event data.
  if (m_mcEvt) {
    m_mcEvt->Clear();
  }
  if (m_digiEvt) {
    m_digiEvt->Clear();
  }
  if (m_reconEvt) {
    m_reconEvt->Clear();
  }

  UInt_t nBytes = 0;
  // if using chains, check the array of chains and move
  // the event pointer to the Req event
  for (int i = 0; i < m_chainArr.GetEntries(); i++)
    nBytes += ((TChain*)m_chainArr.At(i))->GetEvent(iEvt);

  m_nextEvt++;
  return nBytes;
}

UInt_t RootFileAnalysis::getEntries() const {
  // Purpose and Method:  Determine the number of events to iterate over
  //   checking to be sure that the Req number of events is less than
  //   the min number of events in all files

  UInt_t nEntries = 0;
  nEntries = (int)(((TChain*)m_chainArr.At(0))->GetEntries());
  for (int i = 1; i < m_chainArr.GetEntries(); i++)
    nEntries = min(nEntries, (UInt_t)((TChain*)m_chainArr.At(i))->GetEntries());
  return nEntries;
}

