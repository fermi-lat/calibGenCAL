// LOCAL INCLUDES
#include "RootFileAnalysis.h"

// GLAST INCLUDES

//EXTLIB INCLUDES
#include "TFile.h"
#include "TChainElement.h"
#include "TStreamerInfo.h"

// STD INCLUDES
#include <iostream>
#include <vector>
#include <string>

using namespace std;

RootFileAnalysis::RootFileAnalysis(const vector<string> &mcFilenames,
                                   const vector<string> &digiFilenames,
                                   const vector<string> &recFilenames,
                                   ostream &ostr) :
  m_mcChain("mc"),
  m_digiChain("Digi"),
  m_recChain("rec"),
  m_mcFilenames(mcFilenames),
  m_digiFilenames(digiFilenames),
  m_recFilenames(recFilenames),
  m_ostrm(ostr)
{
  
  zeroMembers();

  // add mc file list into mc ROOT chain
  if (mcFilenames.size() != 0) {
    m_mcEnabled = true;

    for(vector<string>::const_iterator itr = mcFilenames.begin();
        itr != mcFilenames.end(); ++itr) {
      m_ostrm << "mc file added to chain: " << *itr << endl;
      m_mcChain.Add(itr->c_str());
    }
    m_mcChain.SetBranchAddress("McEvent",&m_mcEvt);
    m_chainArr.Add(&m_mcChain);
  }

  // add digi file list into mc ROOT chain
  if (digiFilenames.size() != 0) {
    m_digiEnabled = true;

    for(vector<string>::const_iterator itr = digiFilenames.begin();
        itr != digiFilenames.end(); ++itr) {
      m_ostrm << "digi file added to chain: " << *itr << endl;
      m_digiChain.Add(itr->c_str());
    }
    m_digiChain.SetBranchAddress("DigiEvent",&m_digiEvt);
    m_chainArr.Add(&m_digiChain);
 
    // Check DigiEvt structure Ver
    TIter fileListIter(m_digiChain.GetListOfFiles());
    int   codeDigiEvtVer =
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
  if (recFilenames.size() != 0) {
    m_recEnabled = true;

    for(vector<string>::const_iterator itr = recFilenames.begin();
        itr != recFilenames.end(); ++itr) {
      m_ostrm << "rec file added to chain: " << *itr << endl;
      m_recChain.Add(itr->c_str());
    }
    m_recChain.SetBranchAddress("RecEvent",&m_recEvt);
    m_chainArr.Add(&m_recChain);
  }
}

RootFileAnalysis::~RootFileAnalysis() {
  if (m_mcEvt) {
    m_mcEvt->Clear();
    delete m_mcEvt;
    m_mcEvt = 0;
  }

  if (m_digiEvt) {
    m_digiEvt->Clear();
    delete m_digiEvt;
    m_digiEvt = 0;
  }
   
  if (m_recEvt) {
    m_recEvt->Clear();
    delete m_recEvt;
    m_recEvt = 0;
  }
}

UInt_t RootFileAnalysis::getEvent(UInt_t iEvt) {
  // Purpose and Method:  Get the event, iEvt, for all trees

  UInt_t nBytes = 0;
  // if using chains, check the array of chains and move
  // the event pointer to the Req event
  for (Int_t i = 0; i < m_chainArr.GetEntries(); i++) {
    nBytes += ((TChain*)m_chainArr.At(i))->GetEvent(iEvt);
  }

  m_startEvt++;
  return nBytes;
}

UInt_t RootFileAnalysis::getEntries() const {
  // Purpose and Method:  Determine the number of events to iterate over
  //   checking to be sure that the Req number of events is less than
  //   the min number of events in all files

  UInt_t nEntries = 0;
  nEntries = (int)(((TChain*)m_chainArr.At(0))->GetEntries());
  for (Int_t i = 1; i < m_chainArr.GetEntries(); i++)
    nEntries = min(nEntries, (UInt_t)((TChain*)m_chainArr.At(i))->GetEntries());
  return nEntries;
}

// Initializes all members to zero, does NOT free memory, for use in constructor.
void RootFileAnalysis::zeroMembers() {
  m_mcEnabled    = m_digiEnabled = m_recEnabled = false;
  m_startEvt   = 0;
  m_digiEvt = 0;
  m_mcEvt = 0;
  m_recEvt = 0;
}
