#include <string>
#include <algorithm>
#include <map>
#include <fstream>
#include "TFile.h"
#include "TTree.h"
#include "TNtuple.h"
#include "TH1F.h"
#include "TH2F.h"
#include "mcRootData/McEvent.h"
#include "reconRootData/ReconEvent.h"
#include "digiRootData/DigiEvent.h"

bool extractLayerView(const VolumeIdentifier& id, int& iLayer, 
				    int& iView) 
{
  // id code is formatted as the following (if only one tower is simulated):
  // Tower(0)/TowerY(0)/TowerX(0)/TKR(1)/TrayNo(0-18)/Measure(0,1)/View(0,1)/
  // Ladder/Waffer

  if(id[0] != 0) return false;

  // not Tracker
  if(id[3] != 1) return 0;

  int iTray = id[4];
  iView = id[5];  // Measure X: 0, Measure Y: 1
  int iBotTop = id[6];  // Bott: 0, Top: 1

  iLayer = iTray -1 + iBotTop;

  // check boundary condition
  if(iLayer < 0 || iLayer > 17 || iView < 0 || iView > 1) return 0;

  return 1;
}

int main()
{
  /*
  TFile f("temp.ntp", "RECREATE");
  TObjArray* rawHists = new TObjArray(192);

  for(int iLayer = 0; iLayer != 8; ++iLayer) {
    for(int iCol = 0; iCol != 12; ++iCol) {
      for(int iView = 0; iView != 2; ++iView) {

	char* histName = "raw0000";
	sprintf(histName, "raw%1d%2d%1d", iLayer, iCol, iView);

	rawHists->AddAt(new TH1F(histName, histName, 200, -300
  TFile m_digiFile("ebf030905014208_digi.root", "READ");
  if(m_digiFile.IsZombie()) {
    cout << "dig file not found" << endl;
    exit;
  }

  TTree* m_digiTree = (TTree*) m_digiFile.Get("Digi");
  DigiEvent* m_digiEvent = 0;
  TBranch* m_digiBranch = m_digiTree->GetBranch("DigiEvent");
  m_digiBranch->SetAddress(&m_digiEvent);
  
  int nEvent = (int) m_digiTree->GetEntries();

  for(int iEvent = 0; iEvent != nEvent; ++iEvent) { 

    m_digiBranch->GetEntry(iEvent);

    const TObjArray* digiCol = m_digiEvent->getCalDigiCol();

    if(digiCol == 0) continue;

    int nCol = digiCol->GetLast()+1;

    for(int i = 0; i != nCol; ++i) {

      CalDigi* cDigi =  dynamic_cast<CalDigi*>(digiCol->At(i));

      assert(cDigi != 0);

      CalXtalId id = cDigi->getPackedId();
 
      for(int iRange = 0; iRange != 4; ++iRange) {
	tuple.Fill(cDigi->getAdc(iRange, CalXtalId::POS), id.getLayer(),
		   id.getColumn(), 1, iRange);

	tuple.Fill(cDigi->getAdc(iRange, CalXtalId::NEG), id.getLayer(),
		   id.getColumn(), 0, iRange);

      }

    }

  }
 
  f.cd();
  tuple.Write();
  */
}

