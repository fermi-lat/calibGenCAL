#include <cmath>
#include <fstream>
#include "TF1.h"
#include "lightTaperCalib.h"

lightTaperCalib::lightTaperCalib(): m_reconFile(0), m_reconTree(0), 
    m_reconEvent(0), m_digiFile(0), m_digiTree(0),
    m_digiEvent(0), m_taperFile(0)
{
  for(int iLayer = 0; iLayer != g_nLayer; ++iLayer) {
    for(int iCol = 0; iCol != g_nCol; ++iCol) {
      for(int iFace = 0; iFace != g_nFace; ++iFace) {

	char name[] = "pos0000";
	sprintf(name,"pos%1d%02d%1d", iLayer, iCol, iFace);
	m_taperPos[iLayer][iCol][iFace] = new TGraphErrors(g_nDiv);
	m_taperPos[iLayer][iCol][iFace]->SetName(name);

	for(int iDiv = 0; iDiv != g_nDiv; ++iDiv) {

	char name[] = "tap0000000";
	sprintf(name,"tap%1d%02d%1d%03d", iLayer, iCol, iFace, iDiv);

	m_taper[iLayer][iCol][iFace][iDiv] = new TH1F(name, name, 2000, 0, 2000);

	}
      }
    }
  }

  m_eneHist = new TH1F("ene", "ene", 100, -10, 100);

  m_tuple = new TNtuple("tuple", "tuple", "ene:layer:col:event");

}

lightTaperCalib::~lightTaperCalib() 
{
  if(m_taperFile == 0) return;

  m_taperFile->cd();
  //  gDirectory->Write(0, TObject::kOverwrite);
  
  for(int iLayer = 0; iLayer != g_nLayer; ++iLayer) {
    for(int iCol = 0; iCol != g_nCol; ++iCol) {
      for(int iFace = 0; iFace != g_nFace; ++iFace) {

		m_taperPos[iLayer][iCol][iFace]->Write(0, TObject::kOverwrite);

	for(int iDiv = 0; iDiv != g_nDiv; ++iDiv) {

	m_taper[iLayer][iCol][iFace][iDiv]->Write(0, TObject::kOverwrite);

	}
      }
    }
  }

  m_eneHist->Write(0, TObject::kOverwrite);
  m_tuple->Write(0, TObject::kOverwrite);
 
  m_taperFile->Close();
}


void lightTaperCalib::genLightTaper(const char* digi, const char* recon, 
				    const char* outputTxt, 
				    const char* outputRoot)
{
  m_txtOutput = outputTxt;
  m_taperFile = new TFile(outputRoot, "RECREATE");

  m_reconFile = new TFile(recon, "READ");
  if(m_reconFile->IsZombie()) {
    m_reconFile = 0;
    std::cout << "recon file " << recon << " does not exist! abort!" <<
      std::endl;
    exit(1);
  }

  if(m_reconFile) {
    m_reconTree = (TTree*) m_reconFile->Get("Recon");
    m_reconEvent = 0;
    m_reconTree->SetBranchAddress("ReconEvent", &m_reconEvent);
  }

  m_digiFile = new TFile(digi, "READ");
  if(m_digiFile->IsZombie()) {
    m_digiFile = 0;
    std::cout << "digi file " << digi << " does not exist! abort!" <<
      std::endl;
    exit(1);
  }

  if(m_digiFile) {
    m_digiTree = (TTree*) m_digiFile->Get("Digi");
    m_digiEvent = 0;
    m_digiTree->SetBranchAddress("DigiEvent", &m_digiEvent);
  }

  int nEvent, nRecon, nDigi;
  if(m_reconFile) {
    nRecon = (int) m_reconTree->GetEntries();
    cout << "No of events in " << recon << " : " << nRecon << endl;
    nEvent = nRecon;
  }
  if(m_digiFile) {
    nDigi = (int) m_digiTree->GetEntries();
    cout << "No of events in " << digi << " : " << nDigi << endl;
    nEvent = nDigi;
  }

  if(nDigi != nRecon) {
    std::cout << "No. of events in the digi file is not equal to no. of events in the recon file! abort!" << std::endl;
    exit(1);
  }

  // nEvent = 10000;

  analyzeEvent(nEvent);
  /*
  for(int i = 0; i != 10; ++i) {
    m_taperPos[0][0][0]->SetPoint(i, i, 2*i);
    m_taperPos[0][0][0]->SetPointError(i, 1, 1);
  }
  */
   fitTaper();
}

void lightTaperCalib::genLightTaper(TChain* digi, TChain* recon, 
				    const char* outputTxt, 
				    const char* outputRoot)
{
  m_txtOutput = outputTxt;
  m_taperFile = new TFile(outputRoot, "RECREATE");

  m_reconTree = recon;
  m_reconEvent = 0;
  m_reconTree->SetBranchAddress("ReconEvent", &m_reconEvent);

  m_digiTree = digi;
  m_digiEvent = 0;
  m_digiTree->SetBranchAddress("DigiEvent", &m_digiEvent);

  int nEvent, nRecon, nDigi;
  nRecon = (int) m_reconTree->GetEntries();
  cout << "No of events in recon tree: " << nRecon << endl;
  nEvent = nRecon;
  
  nDigi = (int) m_digiTree->GetEntries();
  cout << "No of events in digi tree: " << nDigi << endl;
  nEvent = nDigi;

  if(nDigi != nRecon) {
    std::cout << "No. of events in the digi file is not equal to no. of events in the recon file! abort!" << std::endl;
    exit(1);
  }

  //  nEvent = 10000;

  analyzeEvent(nEvent);

  fitTaper();
}

void lightTaperCalib::analyzeEvent(int nEvent) 
{
  for(int iEvent = 0; iEvent != nEvent; ++iEvent) {

    m_reconTree->GetEntry(iEvent);
    m_digiTree->GetEntry(iEvent);

    assert(m_reconEvent != 0);
    assert(m_digiEvent != 0);

    if(! passCut()) continue;

    getCalEnergy();

    for (int iLayer = 0; iLayer != g_nLayer; ++iLayer) {
      float x, y;
      float z = g_calTop + g_calDz * iLayer;
      extrapolate(x, y, z);

      if(x < -g_calLen/2. || x > g_calLen/2. || y < -g_calLen/2. ||
	 y > g_calLen/2.) continue;

      // top layer is measure Y
      int iCol;
      if( iLayer & 1 ) { // measure X
	iCol = int( (x+g_calLen/2.) / (g_calLen/g_nCol) );
	fillCALAdc(iLayer, iCol, y);
      }
      else {
	iCol = int( (y+g_calLen/2.) / (g_calLen/g_nCol) );
	fillCALAdc(iLayer, iCol, x);
      }
      
      m_eneHist->Fill(m_xtalEne[iLayer][iCol]);
      //      m_tuple->Fill(m_xtalEne[iLayer][iCol], iLayer, iCol, iEvent);

    }
  }
}

void lightTaperCalib::fillCALAdc(int layer, int col, float pos) 
{
  const TObjArray* calDigiCol = m_digiEvent->getCalDigiCol();
  if (!calDigiCol) return;

  int nCalDigi = calDigiCol->GetLast()+1;

  for(int iDigi = 0; iDigi != nCalDigi; ++iDigi) {

    const CalDigi* calDigi = dynamic_cast<const CalDigi*>(calDigiCol->At(iDigi));

    assert(calDigi != 0);

    const CalXtalId id = calDigi->getPackedId();

    if(layer != id.getLayer() || col != id.getColumn()) continue;

    int div = int( (pos + g_calLen/2.) / (g_calLen/g_nDiv) );

    //    cout << calDigi->getAdc(0, CalXtalId::NEG) << " - " << m_pedestal[layer][col][0] << endl;

    m_taper[layer][col][0][div]->Fill( (calDigi->getAdc(0, CalXtalId::NEG) 
				      - m_pedestal[layer][col][0]) 
				       * fabs(m_dir.z()) );
    m_taper[layer][col][1][div]->Fill( (calDigi->getAdc(0, CalXtalId::POS)
				      - m_pedestal[layer][col][1])
				       * fabs(m_dir.z()) );

  }
}

bool lightTaperCalib::passCut() 
{
    TkrRecon* tkrRecon = m_reconEvent->getTkrRecon(); 
    assert(tkrRecon != 0);

    TObjArray* vertices = tkrRecon->getVertexCol();

    // select only 1 track event
    if(vertices->GetLast()+1 != 1) return false;

    TkrVertex* tkrVertex = dynamic_cast<TkrVertex*>(vertices->At(0));
    if(tkrVertex) {
      m_pos = tkrVertex->getPosition();
      m_dir = tkrVertex->getDirection();

      if(m_dir.Z() > -0.95) return false;
    }

    return true;
}

void lightTaperCalib::extrapolate(float& x, float& y, float z) const
{
  x = m_pos.X() + (z - m_pos.Z()) * m_dir.X() / m_dir.Z();
  y = m_pos.Y() + (z - m_pos.Z()) * m_dir.Y() / m_dir.Z();
}

void lightTaperCalib::getCalEnergy() 
{
  for(int iLayer = 0; iLayer != g_nLayer; ++iLayer) {
    for(int iCol = 0; iCol != g_nCol; ++iCol) {
      m_xtalEne[iLayer][iCol] = 0;
    }
  }

  CalRecon* calRecon = m_reconEvent->getCalRecon();

  TObjArray* calXtalRecCol = calRecon->getCalXtalRecCol();

  if(calXtalRecCol) {
    int nCalRec = calXtalRecCol->GetLast() + 1;

    for(int i = 0; i != nCalRec; ++i) {
      CalXtalRecData* calData = 
	dynamic_cast<CalXtalRecData*>(calXtalRecCol->At(i));
      if(calData) {
	CalXtalId id = calData->getPackedId();
	m_xtalEne[id.getLayer()][id.getColumn()] = calData->getEnergy();
      }

    }

  }

}

void lightTaperCalib::readPedestal(const char* fileName)
{
  std::ifstream pedF(fileName);

  int layer, col, face, range;
  float ped, pedRms;

  while(pedF >> layer >> col >> face >> range >> ped >> pedRms) {
    if(range == 0) m_pedestal[layer][col][face] = ped;
  }
}

void lightTaperCalib::fitTaper()
{

  std::ofstream output(m_txtOutput.c_str());

  for(int iLayer = 0; iLayer != g_nLayer; ++iLayer) {
    for(int iCol = 0; iCol != g_nCol; ++iCol) {
      for(int iFace = 0; iFace != g_nFace; ++iFace) {

	for(int iDiv = 0; iDiv != g_nDiv; ++iDiv) {
	
	  float ave = m_taper[iLayer][iCol][iFace][iDiv]->GetMean();
	  float rms = m_taper[iLayer][iCol][iFace][iDiv]->GetRMS();
	  m_taper[iLayer][iCol][iFace][iDiv]->Fit("landau", "", "", ave-2*rms, ave+3*rms);

	  double* par = (m_taper[iLayer][iCol][iFace][iDiv]->GetFunction("landau"))->GetParameters();

	  double* error = (m_taper[iLayer][iCol][iFace][iDiv]->GetFunction("landau"))->GetParErrors();

	  float pos = (iDiv + 0.5) * g_calLen / g_nDiv;
	  float errPos = 0.5 * g_calLen / g_nDiv;

	  float peak = float( *(par+1) );
	  float errPeak = float( *(error+1) );

	  float width = float( *(par+2) );
	  float errWidth = float( *(error+2) );

	  m_taperPos[iLayer][iCol][iFace]->SetPoint(iDiv, pos, peak);
	  m_taperPos[iLayer][iCol][iFace]->SetPointError(iDiv, errPos, errPeak);


	  output << iLayer << ' ' << iCol << ' ' << iFace << ' ' << pos
		 << ' ' << peak << ' ' << errPeak << ' ' << width << ' '
		 << errWidth << endl;

	}

	//	std::cout << "Fitting layer " << iLayer << " col " << iCol
	//	  << " face " << iFace << std::endl;

	//it is better to fit it with user defined function: a*exp(b*x)
	// "expo" function defined in root is: exp(a+b*x)
	// using default may lead to misfitting 
	// m_taperPos[iLayer][iCol][iFace]->Fit("expo", "", "", 50., 280.);

      }
    }
  }
}
