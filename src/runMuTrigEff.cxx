// LOCAL INCLUDES
#include "RootFileAnalysis.h"

#include "MtCfg.h"
#include "CalDefs.h"

// GLAST INCLUDES

// EXTLIB INCLUDES
#include "TF1.h"
#include "TGraph.h"
#include "TSpline.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TNtuple.h"

// STD INCLUDES
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

using namespace std;
using namespace CalDefs;


//////////////////////////////////////////////////////
// class MtData - represents the set of histograms to define the trigger efficiency for muons
//////////////////////////////////////////////////////
class MtData {
  friend class RootMuTrig;
  friend class RootCiTrig;

public:

  MtData(MtCfg &cfg);
  ~MtData() {}
 
  void openHistFile(const string &filename); ///< opens new histogram file.  closes current m_histFile if it is open
  void flushHists(); ///< writes histograms to file & closes file if m_histFile is open.  deletes all open histograms

  void fillMuHist(FaceIdx face, int adc){ (m_muHists[face])->Fill(adc);}
  void fillTrigHist(FaceIdx face, int adc){ (m_trigHists[face])->Fill(adc);}
  float getPed(RngIdx rng){return m_calPed[rng];}
  void readCalPeds(const string &filename); ///< read 4-range pedestals from .txt file created in muonCalib w/ WritePedsTXT
  void initHists();
  void FitData();
  void writeThreshTXT(const string &filename);

private:
  MtCfg &m_cfg;
  CalVec<FaceIdx, TH1F*> m_muHists;
  CalVec<FaceIdx, TH1F*> m_trigHists;

  CalVec<FaceIdx, TH1F*> m_ciAdcHists;
  CalVec<FaceIdx, TH1F*> m_ciEffHists;


  CalVec<FaceIdx,float> m_muThresh;
  CalVec<FaceIdx,float> m_muThreshWidth;
  CalVec<FaceIdx,float> m_ciThresh;
  CalVec<FaceIdx,float> m_ciThreshWidth;

  CalVec<RngIdx, vector<float> >  m_adcSum;
  CalVec<RngIdx, vector<float> >  m_adcN;
  CalVec<RngIdx, vector<float> >  m_adcMean;
  CalVec<FaceIdx, vector<float> >  m_trigSum;


  CalVec<RngIdx, float> m_calPed; ///< final pedestal values, all 4 ranges, indexed by ADC getNRng()

  auto_ptr<TFile> m_histFile;  ///< Current histogram file
  string m_histFilename;  ///< name of the current output histogram ROOT file
};
void MtData::readCalPeds(const string &filename) {
  m_calPed.resize(RngIdx::N_VALS);

  ifstream infile(filename.c_str());
  if (!infile.is_open())
    throw string("Unable to open " + filename);

  unsigned nRead = 0;
  while(infile.good()) {
    float av,err;
    short lyr;
    short col;
    short face;
    short rng;
    
    infile >> lyr
           >> col
           >> face
           >> rng
           >> av
           >> err;
    if (infile.fail()) break; // quit once we can't read any more values
    nRead++;

    RngIdx rngIdx(0,lyr,col,face,rng);
    m_calPed[rngIdx]= av;
  }

  if (nRead != m_calPed.size()) {
    ostringstream temp;
    temp << "CalPed file '" << filename << "' is incomplete: " << nRead
         << " pedestal values read, " << m_calPed.size() << " vals required.";
    throw temp.str();
  }
}

void MtData::flushHists() {
  // write current histograms to file & close if we have an open file.
  if (m_histFile.get()) {
    m_histFile->Write();
    m_histFile->Close(); // all histograms deleted.
    delete m_histFile.release();
  }

  // clear pointer arrays
  m_muHists.clear();
  m_trigHists.clear();
}

void MtData::openHistFile(const string &filename) {
  if (m_histFile.get()) flushHists();

  m_histFilename = filename;

  m_histFile.reset(new TFile(m_histFilename.c_str(), "RECREATE","MuonTriggerHistograms",9));
}

void MtData::initHists() {
  // DEJA VU?
  if (m_muHists.size() == 0){ 
    m_muHists.resize(FaceIdx::N_VALS);

    for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++) {
      ostringstream muHistName;
      muHistName << "mu_" << faceIdx;

      m_muHists[faceIdx] = new TH1F(muHistName.str().c_str(),
                                          muHistName.str().c_str(),
                                          100,0,2000);
    }
  }else // clear existing histsograms
	  for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++){
			m_muHists[faceIdx]->Reset();
			
	  }

   if (m_trigHists.size() == 0){ 
      m_trigHists.resize(FaceIdx::N_VALS);

    for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++) {
      ostringstream trigHistName;
      trigHistName << "trig_" << faceIdx;

      m_trigHists[faceIdx] = new TH1F(trigHistName.str().c_str(),
                                          trigHistName.str().c_str(),
                                          100,0,2000);
    }
  }else // clear existing histsograms
	  for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++){
			m_trigHists[faceIdx]->Reset();
			
	  }
  if (m_ciAdcHists.size() == 0){ 
      m_ciAdcHists.resize(FaceIdx::N_VALS);

    for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++) {
      ostringstream ciAdcHistName;
      ciAdcHistName << "ciAdc_" << faceIdx;

      m_ciAdcHists[faceIdx] = new TH1F(ciAdcHistName.str().c_str(),
                                          ciAdcHistName.str().c_str(),
                                          m_cfg.nDACs,0,m_cfg.nDACs);
    }
  }else // clear existing histsograms
	  for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++){
			m_ciAdcHists[faceIdx]->Reset();
			
	  }
  if (m_ciEffHists.size() == 0){ 
      m_ciEffHists.resize(FaceIdx::N_VALS);

    for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++) {
      ostringstream ciEffHistName;
      ciEffHistName << "ciEff_" << faceIdx;

      m_ciEffHists[faceIdx] = new TH1F(ciEffHistName.str().c_str(),
                                          ciEffHistName.str().c_str(),
                                          m_cfg.nDACs,0,m_cfg.nDACs);
    }
  }else // clear existing histsograms
	  for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++){
			m_ciEffHists[faceIdx]->Reset();
			
	  }

}
MtData::MtData(MtCfg &cfg) :
  m_cfg(cfg),
  m_adcSum(RngIdx::N_VALS, vector<float>(cfg.nDACs,0)),
  m_adcN(RngIdx::N_VALS, vector<float>(cfg.nDACs,0)),
  m_adcMean(RngIdx::N_VALS, vector<float>(cfg.nDACs,0)),
  m_trigSum(FaceIdx::N_VALS, vector<float>(cfg.nDACs,0))

{
	      // open new histogram file
      if (m_cfg.genHistfile) openHistFile(m_cfg.histFile);
	  initHists();
      m_cfg.ostr << "Reading pedestals from " << m_cfg.pedFileTXT << endl;
      readCalPeds(m_cfg.pedFileTXT);

 }

// smooth test lines & print output.
void MtData::FitData() {
	if (m_muThresh.size() == 0)m_muThresh.resize(FaceIdx::N_VALS);
	if (m_muThreshWidth.size() == 0)m_muThreshWidth.resize(FaceIdx::N_VALS);
	if (m_ciThresh.size() == 0)m_ciThresh.resize(FaceIdx::N_VALS);
	if (m_ciThreshWidth.size() == 0)m_ciThreshWidth.resize(FaceIdx::N_VALS);

	RngNum rng = LEX8;
	for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++) {

      RngIdx rngIdx(faceIdx,rng);
	  float a0=0.0;
	  float x[400],ex[400],ey[400],eff[400];


// calculation of trigger efficiency for charge injection
      // get pedestal
//      float ped     = m_adcSum[rngIdx][0] /
//        m_adcN[rngIdx][0];

      //calculate ped-subtracted means.
	  for (int i = 0; i < m_cfg.nDACs; i++){
        x[i] = 
          m_adcSum[rngIdx][i] /
//          m_adcN[rngIdx][i] - ped;
          m_adcN[rngIdx][i] - m_calPed[rngIdx];
	    ex[i]=1.0;
	    float trig = m_trigSum[faceIdx][i];
		float ntot = m_adcN[rngIdx][i];
		float notrig = ntot - trig;
			float strig=sqrt((trig>0.9) ? trig : 1);
			float snotrig=sqrt((notrig>0.9) ? notrig : 1);
			ey[i]=(strig*notrig + snotrig*trig)/(ntot*ntot+0.1);
			eff[i] = trig/(ntot+0.01);
			(m_ciEffHists[faceIdx])->SetBinContent(i,eff[i]);
			(m_ciEffHists[faceIdx])->SetBinError(i,ey[i]);
			(m_ciAdcHists[faceIdx])->SetBinContent(i,x[i]);
			(m_ciAdcHists[faceIdx])->SetBinError(i,ex[i]);
	  }
		for(int i=0;i<m_cfg.nDACs;i++) if(eff[i] > 0.5) {a0=x[i]; break;}
		TGraphErrors* geff = new TGraphErrors(m_cfg.nDACs,x,eff,ex,ey);
		TF1* step = new TF1("step","1.0/(1.0+exp(-[1]*(x-[0])))",0,1000);
		step->SetParameters(a0,0.1);
		geff->Fit(step,"WQN");
		m_ciThresh[faceIdx] = step->GetParameter(0);
		m_ciThreshWidth[faceIdx] = step->GetParameter(1);
		delete step;
		delete geff;

// calculation of trigger efficiency for muons

		float* ymu =(m_muHists[faceIdx])->GetArray();
		float* ytrig =(m_trigHists[faceIdx])->GetArray();
		a0=0.0;
		for(int i=0;i<100;i++){
			x[i]=20*(i+0.5);
			ex[i]=1.0;
			float trig=ytrig[i+1];
			float mu=ymu[i+1];
			float notrig=mu-trig;
			float strig=sqrt((trig>0.9) ? trig : 1);
			float snotrig=sqrt((notrig>0.9) ? notrig : 1);
			ey[i]=(strig*notrig + snotrig*trig)/(mu*mu+0.1);
			eff[i] = trig/(mu+0.01);
		}
		for(int i=0;i<100;i++) if(eff[i] > 0.5) {a0=x[i]; break;}
		geff = new TGraphErrors(100,x,eff,ex,ey);
		step = new TF1("step","1.0/(1.0+exp(-[1]*(x-[0])))",0,1000);
		step->SetParameters(a0,0.1);
		geff->Fit(step,"WQN");
		m_muThresh[faceIdx] = step->GetParameter(0);
		m_muThreshWidth[faceIdx] = step->GetParameter(1);
		delete step;
		delete geff;
	}
   
}

void MtData::writeThreshTXT(const string &filename) {
  ofstream outfile(filename.c_str());
  if (!outfile.is_open())
    throw string("Unable to open " + filename);

  TNtuple* ntp = new TNtuple("ci_mu_thresh_ntp","ci_mu_thresh_ntp","lyr:col:face:muThresh:muThreshWidth:ciThresh:ciThreshWidth");
  float xntp[7];

  for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++) {
    LyrNum lyr = faceIdx.getLyr();
    ColNum col = faceIdx.getCol();
    FaceNum face = faceIdx.getFace();
    xntp[0]= lyr;
    xntp[1]= col;
    xntp[2]= face;
    xntp[3]= m_muThresh[faceIdx];
    xntp[4]= m_muThreshWidth[faceIdx];
    xntp[5]= m_ciThresh[faceIdx];
    xntp[6]= m_ciThreshWidth[faceIdx];

	ntp->Fill(xntp);

    outfile  << " " << lyr
             << " " << col
             << " " << face
             << " " << m_muThresh[faceIdx]
             << " " << m_muThreshWidth[faceIdx]
             << " " << m_ciThresh[faceIdx]
             << " " << m_ciThreshWidth[faceIdx]
             << " " << m_muThresh[faceIdx]/m_ciThresh[faceIdx]
             << endl;
  }
   ntp->Write();

}


class RootCiTrig : public RootFileAnalysis {
public:
  // @enum Diode Specify LE, HE, BOTH_DIODES
  typedef enum Diode {
    LE,
    HE,
    BOTH_DIODES};

  // Standard ctor, where user provides the names of the input root files
  // and optionally the name of the output ROOT histogram file
  RootCiTrig(vector<string> &digiFileNames,
         MtData  &data, MtCfg &cfg);

  // standard dtor
  ~RootCiTrig();

  // processes one 'event', as a collection of interactions.
  // loads up histograms.
  void DigiCal();

  // loops through all events in file
  void Go(Int_t nEvtAsked);

  void SetDiode(RootCiTrig::Diode d) {m_curDiode = d;}

private:
  bool   isRngEnabled(RngNum rng);          // checks rng against m_curDiode setting
  Diode m_curDiode;

  MtData &m_mtData;
  MtCfg  &m_cfg;
  bool m_fle[2][8];

  int m_evtId;
};


RootCiTrig::RootCiTrig(vector<string> &digiFileNames, MtData  &data, MtCfg &cfg) :
  RootFileAnalysis(vector<string>(0), digiFileNames, vector<string>(0)),
  m_curDiode(LE),
  m_mtData(data),
  m_cfg(cfg) {}

// default dstor
RootCiTrig::~RootCiTrig() {
}


// checks rng against m_curDiode setting
bool  RootCiTrig::isRngEnabled(RngNum rng) {
  if (m_curDiode == BOTH_DIODES) return true;
  if (m_curDiode == LE && (rng == LEX8 || rng == LEX1)) return true;
  if (m_curDiode == HE && (rng == HEX8 || rng == HEX1)) return true;
  return false;
}

// compiles stats for each test type.
void RootCiTrig::DigiCal() {
  // Determine test config for this event (which xtal?, which dac?)
  int testCol   = m_evtId/m_cfg.nPulsesPerXtal;
  int testDAC   = (m_evtId%m_cfg.nPulsesPerXtal)/m_cfg.nPulsesPerDAC;


	for(int layer=0;layer<8;layer++)for(int side=0;side<2;side++)m_fle[side][layer]=false;
    const TClonesArray* calDiagCol = m_digiEvt->getCalDiagnosticCol();
  TIter calDiagIter(calDiagCol);
 // Loop through diagnostics for all layers
  CalDiagnosticData *pdiag = 0;
  while ((pdiag = (CalDiagnosticData*)calDiagIter.Next())) {  //loop through each 'hit' in one event
    CalDiagnosticData &cdiag = *pdiag; // use ref to reduce '->'
  
	int layer = cdiag.layer();
	for (int side=0;side<2;side++)m_fle[side][layer] = cdiag.low(side);  
  }


  const TObjArray* calDigiCol = m_digiEvt->getCalDigiCol();
  if (!calDigiCol) {
    ostringstream temp;
    temp << "Empty calDigiCol event #" << m_evtId;
    throw temp.str();
  }
  TIter calDigiIter(calDigiCol);

  // Loop through each xtal interaction
  CalDigi *pdig = 0;
  while ((pdig = (CalDigi*)calDigiIter.Next())) {  //loop through each 'hit' in one event
    CalDigi &cdig = *pdig; // use ref to reduce '->'

    CalXtalId id = cdig.getPackedId();  // get interaction information
    ColNum col = id.getColumn();
    if (col != testCol) continue;

    TwrNum twr = id.getTower();
    LyrNum lyr = id.getLayer();

    // Loop through each readout on current xtal
    int numRo = cdig.getNumReadouts();
    for (int iRo=0; iRo<numRo; iRo++){
      const CalXtalReadout &acRo = *(cdig.getXtalReadout(iRo));
      for (FaceNum face; face.isValid(); face++) {
        RngNum rng = acRo.getRange((CalXtalId::XtalFace)(short)face);
        // only interested in current diode!
        if (!isRngEnabled(rng)) continue;

        int adc    = acRo.getAdc((CalXtalId::XtalFace)(short)face);
 
        FaceIdx faceIdx(twr,lyr,col,face);
        RngIdx rngIdx(twr,lyr,col,face,rng);
        
        // assign to table
        m_mtData.m_adcSum[rngIdx][testDAC] += adc;
        m_mtData.m_adcN[rngIdx][testDAC]++;
		if(rng == LEX8 && m_fle[face][lyr])m_mtData.m_trigSum[faceIdx][testDAC]++;
      } // foreach face
    } // foreach readout
  } // foreach xtal
}

void RootCiTrig::Go(Int_t nEvtAsked)
{
  // Purpose and Method:  Event Loop

  //
  //  COMMENT OUT ANY BRANCHES YOU ARE NOT INTERESTED IN.
  //
  if (m_digiEnabled) {
    m_digiChain.SetBranchStatus("*",0);  // disable all branches
    // activate desired brances
    m_digiChain.SetBranchStatus("m_cal*",1);
    m_digiChain.SetBranchStatus("m_eventId", 1);
  }

  //
  // DO WE HAVE ENOUGH EVENTS IN FILE?
  //
  Int_t nEvts = getEntries();
  m_cfg.ostr << "\nNum Events in File is: " << nEvts << endl;
  Int_t curI;
  Int_t nMax = min(nEvtAsked+m_startEvt,nEvts);

  if (nEvtAsked+m_startEvt >  nEvts) {
    ostringstream temp;
    temp << " not enough entries in file to proceed, we need " << nEvtAsked;
    throw temp.str();
  }

  // BEGINNING OF EVENT LOOP
  for (Int_t iEvt=m_startEvt; iEvt<nMax; iEvt++, curI=iEvt) {
    if (m_digiEvt) m_digiEvt->Clear();

    getEvent(iEvt);
    // Digi ONLY analysis
    if (m_digiEvt) {
      m_evtId = m_digiEvt->getEventId();
      if(m_evtId%1000 == 0)
        m_cfg.ostr << " event " << m_evtId << endl;

      DigiCal();
    }
  }  // end analysis code in event loop

  m_startEvt = curI;
}

//////////////////////////////////////////////////////
// class RootMuTrig - derived from RootFileAnalysis - represents all Root input //
// needed for populating the MtData class                                //
//////////////////////////////////////////////////////

class RootMuTrig : public RootFileAnalysis {
public:
	typedef enum { EvenRowsEvenColumns = 0, OddRowsEvenColumns = 1} TRIGCONFIG;
	
  // Standard ctor, where user provides the names of the input root files
  // and optionally the name of the output ROOT histogram file
  RootMuTrig(vector<string> &digiFileNames,
         MtData  &data, MtCfg &cfg, RootMuTrig::TRIGCONFIG trigConf);

  // standard dtor
  ~RootMuTrig();

  // processes one 'event', as a collection of interactions.
  // loads up histograms.
  void DigiCal();

  // loops through all events in file
  void Go();


private:

  MtData &m_mtData;
  MtCfg  &m_cfg;
  TRIGCONFIG m_trigConfig;
  int m_evtId;
  CalVec<FaceIdx,float> m_adc;
  bool m_fle[2][8];
};


RootMuTrig::RootMuTrig(vector<string> &digiFileNames, MtData  &data, MtCfg &cfg,
					   RootMuTrig::TRIGCONFIG trigConf) :
  RootFileAnalysis(vector<string>(0), digiFileNames, vector<string>(0)),
  m_mtData(data),
  m_cfg(cfg),
  m_trigConfig(trigConf)
  {}

// default dstor
RootMuTrig::~RootMuTrig() {
}

// compiles stats for each test type.
void RootMuTrig::DigiCal() {
	for(int layer=0;layer<8;layer++)for(int side=0;side<2;side++)m_fle[side][layer]=false;
    const TClonesArray* calDiagCol = m_digiEvt->getCalDiagnosticCol();
  TIter calDiagIter(calDiagCol);
 // Loop through diagnostics for all layers
  CalDiagnosticData *pdiag = 0;
  while ((pdiag = (CalDiagnosticData*)calDiagIter.Next())) {  //loop through each 'hit' in one event
    CalDiagnosticData &cdiag = *pdiag; // use ref to reduce '->'
  
	int layer = cdiag.layer();
	for (int side=0;side<2;side++)m_fle[side][layer] = cdiag.low(side);  
  
  }
  const TObjArray* calDigiCol = m_digiEvt->getCalDigiCol();
  if (!calDigiCol) {
    ostringstream temp;
    temp << "Empty calDigiCol event #" << m_evtId;
    throw temp.str();
  }
  TIter calDigiIter(calDigiCol);
    if (m_adc.size() == 0) 
    m_adc.resize(FaceIdx::N_VALS);

  // Loop through each xtal interaction
  CalDigi *pdig = 0;
  while ((pdig = (CalDigi*)calDigiIter.Next())) {  //loop through each 'hit' in one event
    CalDigi &cdig = *pdig; // use ref to reduce '->'

    CalXtalId id = cdig.getPackedId();  // get interaction information
    ColNum col = id.getColumn();

    TwrNum twr = id.getTower();
    LyrNum lyr = id.getLayer();
    // Loop through each readout on current xtal
    int numRo = cdig.getNumReadouts();
    for (int iRo=0; iRo<numRo; iRo++){
      const CalXtalReadout &acRo = *(cdig.getXtalReadout(iRo));
      for (FaceNum face; face.isValid(); face++) {
        RngNum rng = acRo.getRange((CalXtalId::XtalFace)(short)face);
        // only interested in LEX8 range
		if(rng == 0){
			FaceIdx faceIdx(twr,lyr,col,face);
			RngIdx rng(faceIdx,rng);
			float ped = m_mtData.getPed(rng);
			m_adc[faceIdx]  = acRo.getAdc((CalXtalId::XtalFace)(short)face)-ped;
		}

      } // foreach face
    } // foreach readout
  } // foreach xtal

	bool odd = !m_trigConfig;
	for (int l = 0;l<8;l++){
		
		if ( (l+1) % 2) odd = !odd;
		int st = odd;
		for (int c = st;c<12;c=c+2){
			bool trg_other_layers = false;
			for (int s=0;s<2;s++)
				for(int ll=0;ll<8;ll++)
					if(l != ll)trg_other_layers = trg_other_layers || m_fle[s][ll];
			for (int s=0;s<2;s++){
				bool no_trg_layer = trg_other_layers;
				for (int cc = st;cc<12;cc=cc+2){
					if (cc != c){
						FaceIdx face(0,l,cc,s);
						no_trg_layer = no_trg_layer && (m_adc[face] < 50);
					}
				}
				FaceIdx faceIdx(0,l,c,s);
				if(no_trg_layer){
					m_mtData.fillMuHist(faceIdx,m_adc[faceIdx]);
					if(m_fle[s][l])m_mtData.fillTrigHist(faceIdx,m_adc[faceIdx]);
				}
			}
		}
	}
}

void RootMuTrig::Go()
{
  // Purpose and Method:  Event Loop

  //
  //  COMMENT OUT ANY BRANCHES YOU ARE NOT INTERESTED IN.
  //
  if (m_digiEnabled) {
    m_digiChain.SetBranchStatus("*",0);  // disable all branches
    // activate desired brances
    m_digiChain.SetBranchStatus("m_cal*",1);
    m_digiChain.SetBranchStatus("m_eventId", 1);
  }

  //
  // DO WE HAVE ENOUGH EVENTS IN FILE?
  //
  Int_t nEvts = getEntries();
  m_cfg.ostr << "\nNum Events in File is: " << nEvts << endl;
  Int_t curI;
 

  // BEGINNING OF EVENT LOOP
  for (Int_t iEvt=m_startEvt; iEvt<nEvts; iEvt++, curI=iEvt) {
    if (m_digiEvt) m_digiEvt->Clear();

    getEvent(iEvt);
    // Digi ONLY analysis
    if (m_digiEvt) {
      m_evtId = m_digiEvt->getEventId();
      if(m_evtId%1000 == 0)
        m_cfg.ostr << " event " << m_evtId << endl;

      DigiCal();
    }
  }  // end analysis code in event loop

  m_startEvt = curI;
}

int main(int argc, char **argv) {
  // Load xml config file
  string cfgPath;
  if(argc > 1) cfgPath = argv[1];
  else cfgPath = "../src/MuTrigEff_option.xml";

  MtCfg cfg;
  try {
    cfg.readCfgFile(cfgPath);

    // ID calibGenCAL package version
    cfg.ostr << "calibGenCAL CVS Tag: " << CGCUtil::CVS_TAG << endl << endl;
    
    // insert quoted config file into log stream //
    { 
      string temp;
      ifstream cfgFile(cfgPath.c_str());
      cfg.ostr << "--- Begin cfg_file: " << cfgPath << " ---" << endl;
      while (cfgFile.good()) {
        getline(cfgFile, temp);
        if (cfgFile.fail()) continue; // bad get
        cfg.ostr << "> " << temp << endl;
      }
      cfg.ostr << "--- End " << cfgPath << " ---" << endl << endl;
    }
    
    MtData data(cfg);
    // reading charge injection file with diagnostic information (tack delay = 70)
    {
      vector<string> digiFileNames;
      digiFileNames.push_back(cfg.rootFileCI);
	  RootCiTrig rd(digiFileNames,data,cfg);  
      rd.SetDiode(RootCiTrig::LE);
      rd.Go(cfg.nPulsesPerRun);
   }

    // trigger configuration A :  Even Rows Even Columns
    {
      vector<string> digiFileNames;
      digiFileNames.push_back(cfg.rootFileA);
	  RootMuTrig rd(digiFileNames,data,cfg,RootMuTrig::TRIGCONFIG::EvenRowsEvenColumns);  
      rd.Go();
    }

    //  trigger configuration B :   Even Rows Odd Columns
    {
      vector<string> digiFileNames;
      digiFileNames.push_back(cfg.rootFileB);
      RootMuTrig rd(digiFileNames, data,cfg,RootMuTrig::TRIGCONFIG::OddRowsEvenColumns);
      rd.Go();
    }

    data.FitData();
    if (cfg.genTXT) data.writeThreshTXT(cfg.outputTXTPath);
 
    if (cfg.genHistfile) data.flushHists();
  
  } catch (string s) {
    // generic exception handler...  all my exceptions are simple C++ strings
    cfg.ostr << "MuTrigEff:  exception thrown: " << s << endl;
    return -1;
  }

  return 0;
}

