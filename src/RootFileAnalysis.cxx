// TODO
// added ROOT version checking into mc & recon chains as well as digi.

#include <iostream>
#include <vector>
#include <string>

//ROOT INCLUDES
#include <TFile.h>
#include <TChainElement.h>
#include <TStreamerInfo.h>

#include "RootFileAnalysis.h"

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
  m_ostr(ostr)
{
  
  zeroMembers();

  // add mc file list into mc ROOT chain
  if (mcFilenames.size() != 0) {
    m_mcEnabled = true;

    for(vector<string>::const_iterator itr = mcFilenames.begin();
	itr != mcFilenames.end(); ++itr) {
      m_ostr << "mc file added to chain: " << *itr << endl;
      m_mcChain.Add(itr->c_str());
    }
    m_mcChain.SetBranchAddress("McEvent",&m_mc);
    m_chainArr.Add(&m_mcChain);
  }

  // add digi file list into mc ROOT chain
  if (digiFilenames.size() != 0) {
    m_digiEnabled = true;

    for(vector<string>::const_iterator itr = digiFilenames.begin();
	itr != digiFilenames.end(); ++itr) {
      m_ostr << "digi file added to chain: " << *itr << endl;
      m_digiChain.Add(itr->c_str());
    }
    m_digiChain.SetBranchAddress("DigiEvent",&m_evt);
    m_chainArr.Add(&m_digiChain);
 
    // Check DigiEvent structure Ver
    TIter fileListIter(m_digiChain.GetListOfFiles());
    int   codeDigiEventVer =
      DigiEvent::Class()->GetClassVersion();

    while (TChainElement *curElement = (TChainElement*)fileListIter.Next()) {
      const char *curFilenames = curElement->GetTitle();
      TFile       curFile(curFilenames);
      int fileDigiEventVer = ((TStreamerInfo*)curFile.GetStreamerInfoList()->FindObject("DigiEvent"))->GetClassVersion();

      if (fileDigiEventVer != codeDigiEventVer) {
	m_ostr << "WARNING: digFile=" << curFilenames << " created with DigiEvent version"
	     << fileDigiEventVer << " code is linked to DigiEvent version"
	     << codeDigiEventVer << endl;
      }
    }
  }

  // add recon file list into mc ROOT chain
  if (recFilenames.size() != 0) {
    m_recEnabled = true;

    for(vector<string>::const_iterator itr = recFilenames.begin();
	itr != recFilenames.end(); ++itr) {
      m_ostr << "rec file added to chain: " << *itr << endl;
      m_recChain.Add(itr->c_str());
    }
    m_recChain.SetBranchAddress("RecEvent",&m_rec);
    m_chainArr.Add(&m_recChain);
  }
}

RootFileAnalysis::~RootFileAnalysis() {
  if (m_mc) {
    m_mc->Clear();
    delete m_mc;
    m_mc = 0;
  }

  if (m_evt) {
    m_evt->Clear();
    delete m_evt;
    m_evt = 0;
  }
   
  if (m_rec) {
    m_rec->Clear();
    delete m_rec;
    m_rec = 0;
  }
}

UInt_t RootFileAnalysis::getEvent(UInt_t ievt) {
  // Purpose and Method:  Get the event, ievt, for all trees

  UInt_t nBytes = 0;
  // if using chains, check the array of chains and move
  // the event pointer to the requested event
  for (Int_t i = 0; i < m_chainArr.GetEntries(); i++) {
    nBytes += ((TChain*)m_chainArr.At(i))->GetEvent(ievt);
  }

  m_startEvent++;
  return nBytes;
}

UInt_t RootFileAnalysis::getEntries() const {
  // Purpose and Method:  Determine the number of events to iterate over
  //   checking to be sure that the requested number of events is less than
  //   the min number of events in all files

  UInt_t nEntries = 0;
  nEntries = (int)(((TChain*)m_chainArr.At(0))->GetEntries());
  for (Int_t i = 1; i < m_chainArr.GetEntries(); i++)
    nEntries = TMath::Min(nEntries, (UInt_t)((TChain*)m_chainArr.At(i))->GetEntries());
  return nEntries;
}

// Initializes all members to zero, does NOT free memory, for use in constructor.
void RootFileAnalysis::zeroMembers() {
  m_mcEnabled    = m_digiEnabled = m_recEnabled = false;
  m_startEvent   = 0;
  m_evt = 0;
  m_mc = 0;
  m_rec = 0;
}
