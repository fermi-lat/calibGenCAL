#include "RootFileAnalysis.h"

// TODO
// added ROOT version checking into mc & recon chains as well as digi.

//ROOT INCLUDES
#include <TFile.h>
#include <TChainElement.h>
#include <TStreamerInfo.h>

#include <iostream>
#include <vector>
#include <string>

RootFileAnalysis::RootFileAnalysis( std::vector<std::string> *digiFileNames,
												std::vector<std::string> *recFileNames,
												std::vector<std::string> *mcFileNames) {
  ZeroMembers();

  if (chainArr) delete chainArr;
  chainArr = new TObjArray();

  // add mc file list into mc ROOT chain
  if (mcFileNames != 0) {
	 m_mcChain = new TChain("mc");
	 for(std::vector<std::string>::const_iterator itr = mcFileNames->begin();
		  itr != mcFileNames->end(); ++itr) {
		std::cout << "mc file added to chain: " << *itr << std::endl;
		m_mcChain->Add(itr->c_str());
	 }
	 m_mcChain->SetBranchAddress("McEvent",&mc);
	 chainArr->Add(m_mcChain);
  }

  // add mc file list into mc ROOT chain
  if (digiFileNames != 0) {
	 m_digiChain = new TChain("Digi");
	 for(std::vector<std::string>::const_iterator itr = digiFileNames->begin();
		  itr != digiFileNames->end(); ++itr) {
		std::cout << "digi file added to chain: " << *itr << std::endl;
		m_digiChain->Add(itr->c_str());
	 }
	 m_digiChain->SetBranchAddress("DigiEvent",&evt);
	 chainArr->Add(m_digiChain);
	 
    // Check DigiEvent structure Ver
    TIter fileListIter(m_digiChain->GetListOfFiles());
    int   codeDigiEventVer =
		DigiEvent::Class()->GetClassVersion();

    while (TChainElement *curElement = (TChainElement *)fileListIter.Next()) {
      const char *curFileName = curElement->GetTitle();
      TFile       curFile(curFileName);
      int fileDigiEventVer = ((TStreamerInfo *)curFile.GetStreamerInfoList()->FindObject("DigiEvent"))->GetClassVersion();

      if (fileDigiEventVer != codeDigiEventVer) {
		  std::cout << "WARNING: digFile=" << curFileName << " created with DigiEvent version"
				 << fileDigiEventVer << " code is linked to DigiEvent version"
				 << codeDigiEventVer << std::endl;
		} else {
		  std::cout << "DigiEvent version" << fileDigiEventVer << " in " << curFileName
				 << " matches code, should be ok." << std::endl;
		}
	 }
  }

  // add mc file list into mc ROOT chain
  if (recFileNames != 0) {
	 m_recChain = new TChain("rec");
	 for(std::vector<std::string>::const_iterator itr = recFileNames->begin();
		  itr != recFileNames->end(); ++itr) {
		std::cout << "rec file added to chain: " << *itr << std::endl;
		m_recChain->Add(itr->c_str());
	 }
	 m_recChain->SetBranchAddress("RecEvent",&rec);
	 chainArr->Add(m_recChain);
  }
}

RootFileAnalysis::~RootFileAnalysis() {
  if (evt) {
	 evt->Clear();
	 delete evt;
  }
  if (rec) {
	 rec->Clear();
	 delete rec;
  }
  if (mc) {
	 mc->Clear();
	 delete mc;
  }

  if (chainArr) delete chainArr;
}

UInt_t RootFileAnalysis::GetEvent(UInt_t ievt) {
  // Purpose and Method:  Get the event, ievt, for all trees

  UInt_t nb = 0;
  // if using chains, check the array of chains and move
  // the event pointer to the requested event
  if (chainArr) {
	 for (Int_t i = 0; i < chainArr->GetEntries(); i++) {
		nb += ((TChain*)chainArr->At(i))->GetEvent(ievt);
	 }
	 return nb;
  }

  return nb;
}

UInt_t RootFileAnalysis::GetEntries() const {
  // Purpose and Method:  Determine the number of events to iterate over
  //   checking to be sure that the requested number of events is less than
  //   the min number of events in all files

  UInt_t nentries = 0;
  if (chainArr) {
	 nentries = (int)(((TChain*)chainArr->At(0))->GetEntries());
	 for (Int_t i = 1; i < chainArr->GetEntries(); i++) {
		nentries = TMath::Min(nentries, (UInt_t)((TChain*)chainArr->At(i))->GetEntries());
	 }
	 return nentries;
  }

  return nentries;
}

// Initializes all members to zero, does NOT free memory, for use in constructor.
void RootFileAnalysis::ZeroMembers() {
  m_digiChain           = 0;
  m_recChain            = 0;
  m_mcChain             = 0;
  evt                   = 0;
  rec                   = 0;
  mc                    = 0;
  chainArr              = 0;

  prevTimeStamp  = 0;
  digiEventId    = reconEventId = mcEventId = 0;
  digiRunNum     = reconRunNum = mcRunNum = 0;
  m_StartEvent    = 0;
}
