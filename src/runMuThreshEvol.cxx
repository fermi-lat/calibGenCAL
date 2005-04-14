// LOCAL INCLUDES
#include "RootFileAnalysis.h"

#include "TeCfg.h"
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
// class TeData - represents the set of histograms to define the thrigger threshold evolution for muons
//////////////////////////////////////////////////////
class TeData {
  friend class RootThreshEvol;

public:

  TeData(TeCfg &cfg);
  ~TeData() {}
 
  void openHistFile(const string &filename); ///< opens new histogram file.  closes current m_histFile if it is open
  void flushHists(); ///< writes histograms to file & closes file if m_histFile is open.  deletes all open histograms

  void fillMuHist(FaceIdx face,int itp, float adc){ (m_muHists[face][itp])->Fill(adc);}
  void fillTrigHist(FaceIdx face,int itp, float adc){ (m_trigHists[face][itp])->Fill(adc);}
  void fillPedHist(FaceIdx face,int itp, float adc){ (m_pedHists[face][itp])->Fill(adc);}
  float getPed(FaceIdx face,int itp){return m_calPed[face][itp];}
  void initHists();
  void FitData();
  void fitPedHists(); 
  void writeThreshTXT(const string &filename);

private:
  TeCfg &m_cfg;
  CalVec<FaceIdx, vector<TH1F*> > m_muHists;
  CalVec<FaceIdx, vector<TH1F*> > m_trigHists;

  CalVec<FaceIdx,vector<float> > m_muThresh;
  CalVec<FaceIdx,vector<float> > m_muThreshWidth;

  CalVec<FaceIdx,vector<float> > m_muThreshErr;
  CalVec<FaceIdx,vector<float> > m_muThreshWidthErr;

  CalVec<FaceIdx,vector<TH1F*> > m_pedHists;

  CalVec<FaceIdx, vector<float> > m_calPed; ///< pedestal values

  auto_ptr<TFile> m_histFile;  ///< Current histogram file
  string m_histFilename;  ///< name of the current output histogram ROOT file
};

void TeData::flushHists() {
  // write current histograms to file & close if we have an open file.
  if (m_histFile.get()) {
    m_histFile->Write();
    m_histFile->Close(); // all histograms deleted.
    delete m_histFile.release();
  }

  // clear pointer arrays
  m_muHists.clear();
  m_trigHists.clear();
  m_pedHists.clear();
}

void TeData::openHistFile(const string &filename) {
  if (m_histFile.get()) flushHists();

  m_histFilename = filename;

  m_histFile.reset(new TFile(m_histFilename.c_str(), "RECREATE","MuonTriggerHistograms",9));
}

void TeData::initHists() {
  // DEJA VU?
  if (m_muHists.size() == 0){ 
    m_muHists.resize(FaceIdx::N_VALS);

    for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++) {
      (m_muHists[faceIdx]).resize(m_cfg.nTimePoints);
      for(int itp = 0; itp<m_cfg.nTimePoints; itp++){
        ostringstream muHistName;
        muHistName << "mu_P" << itp <<"_"<< faceIdx;

        m_muHists[faceIdx][itp] = new TH1F(muHistName.str().c_str(),
                                           muHistName.str().c_str(),
                                           40,0,2000);
      }
    }
  }else // clear existing histsograms
    for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++)
      for(int itp = 0; itp<m_cfg.nTimePoints; itp++)
        m_muHists[faceIdx][itp]->Reset();
          

  if (m_trigHists.size() == 0){ 
    m_trigHists.resize(FaceIdx::N_VALS);

    for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++) {
      (m_trigHists[faceIdx]).resize(m_cfg.nTimePoints);
      for(int itp = 0; itp<m_cfg.nTimePoints; itp++){
        ostringstream trigHistName;
        trigHistName << "trig_P" << itp <<"_" << faceIdx;

        m_trigHists[faceIdx][itp] = new TH1F(trigHistName.str().c_str(),
                                             trigHistName.str().c_str(),
                                             40,0,2000);
      }
    }
  }else // clear existing histsograms
    for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++)
      for(int itp = 0; itp<m_cfg.nTimePoints; itp++)
        m_trigHists[faceIdx][itp]->Reset();
                        

  if (m_pedHists.size() == 0) {
    m_pedHists.resize(FaceIdx::N_VALS);

    for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++) {
      (m_pedHists[faceIdx]).resize(m_cfg.nTimePoints);
      for(int itp = 0; itp<m_cfg.nTimePoints; itp++){
        ostringstream tmp;
        tmp << "peds_P" << itp << "_" << faceIdx;

        m_pedHists[faceIdx][itp] = new TH1F(tmp.str().c_str(),
                                            tmp.str().c_str(),
                                            500,0,1000);
 
      }
    }
  }else // clear existing histsograms
    for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++)
      for(int itp = 0; itp<m_cfg.nTimePoints; itp++)
        m_pedHists[faceIdx][itp]->Reset();

}
TeData::TeData(TeCfg &cfg) :
  m_cfg(cfg)
{
  // open new histogram file
  if (m_cfg.genHistfile) openHistFile(m_cfg.histFile);
  initHists();
}

void TeData::fitPedHists() {
  m_calPed.resize(FaceIdx::N_VALS);

  for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++){
    m_calPed[faceIdx].resize(m_cfg.nEvtsPerPoint);
    for (int itp=0;itp<m_cfg.nTimePoints; itp++){
      // select histogram from list
      TH1F &h= *m_pedHists[faceIdx][itp];

      // trim outliers
      float av = h.GetMean();float err =h.GetRMS();
      for( int iter=0; iter<3;iter++ ) {
        h.SetAxisRange(av-3*err,av+3*err);
        av = h.GetMean(); err= h.GetRMS();
      }
    
      // gaussian fit
      h.Fit("gaus", "Q","", av-3*err, av+3*err );
      h.SetAxisRange(av-150,av+150);

      // assign values to permanent arrays
      float ped =
        ((TF1&)*h.GetFunction("gaus")).GetParameter(1);
      m_calPed[faceIdx][itp] = ped;
    }
  }
}

void TeData::FitData() {
  if (m_muThresh.size() == 0)m_muThresh.resize(FaceIdx::N_VALS);
  if (m_muThreshWidth.size() == 0)m_muThreshWidth.resize(FaceIdx::N_VALS);
  if (m_muThreshErr.size() == 0)m_muThreshErr.resize(FaceIdx::N_VALS);
  if (m_muThreshWidthErr.size() == 0)m_muThreshWidthErr.resize(FaceIdx::N_VALS);

  for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++) {
                
    float a0=0.0;
    float x[400],ex[400],ey[400],eff[400];
    if (m_muThresh[faceIdx].size() == 0)m_muThresh[faceIdx].resize(m_cfg.nTimePoints);
    if (m_muThreshWidth[faceIdx].size() == 0)m_muThreshWidth[faceIdx].resize(m_cfg.nTimePoints);
    if (m_muThreshErr[faceIdx].size() == 0)m_muThreshErr[faceIdx].resize(m_cfg.nTimePoints);
    if (m_muThreshWidthErr[faceIdx].size() == 0)m_muThreshWidthErr[faceIdx].resize(m_cfg.nTimePoints);
                
    for (int itp=0; itp < m_cfg.nTimePoints; itp++){

      // calculation of trigger efficiency for muons

      float* ymu =(m_muHists[faceIdx][itp])->GetArray();
      float* ytrig =(m_trigHists[faceIdx][itp])->GetArray();
      a0=0.0;
      for(int i=0;i<40;i++){
        x[i]=50*(i+0.5);
        ex[i]=1.0;
        float trig=ytrig[i+1];
        float mu=ymu[i+1];
        float notrig=mu-trig;
        float strig=(trig>0.9) ? trig : 1;
        float snotrig=(notrig>0.9) ? notrig : 1;
        ey[i]=sqrt(strig*notrig*notrig + snotrig*trig*trig)/(mu*mu+0.1);
        eff[i] = trig/(mu+0.01);
      }
      for(int i=0;i<40;i++) if(eff[i] > 0.5) {a0=x[i]; break;}
      TGraphErrors* geff = new TGraphErrors(40,x,eff,ex,ey);
      TF1* step = new TF1("step","1.0/(1.0+exp(-[1]*(x-[0])))",0,1500);
      step->SetParameters(a0,0.1);
      geff->Fit(step,"QN");
      m_muThresh[faceIdx][itp] = step->GetParameter(0);
      m_muThreshWidth[faceIdx][itp] = step->GetParameter(1);
      m_muThreshErr[faceIdx][itp] = step->GetParError(0);
      m_muThreshWidthErr[faceIdx][itp] = step->GetParError(1);
      delete step;
      delete geff;
    }
  }
   
}

void TeData::writeThreshTXT(const string &filename) {
  ofstream outfile(filename.c_str());
  if (!outfile.is_open())
    throw string("Unable to open " + filename);

  //  TNtuple* ntp = new TNtuple("ci_mu_thresh_ntp","ci_mu_thresh_ntp","lyr:col:face:muThresh:muThreshWidth:ciThresh:ciThreshWidth");
  //  float xntp[7];

  for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++) {
    LyrNum lyr = faceIdx.getLyr();
    ColNum col = faceIdx.getCol();
    FaceNum face = faceIdx.getFace();

    for( int itp=0; itp < m_cfg.nTimePoints; itp++)
      outfile  << " " << lyr
               << " " << col
               << " " << face
               << " " << itp
               << " " << m_muThresh[faceIdx][itp]
               << " " << m_muThreshErr[faceIdx][itp]
               << " " << m_muThreshWidth[faceIdx][itp]
               << " " << m_muThreshWidthErr[faceIdx][itp]
               << endl;
  }

}



//////////////////////////////////////////////////////
// class RootThreshEvol - derived from RootFileAnalysis - represents all Root input //
// needed for populating the TeData class                                //
//////////////////////////////////////////////////////

class RootThreshEvol : public RootFileAnalysis {
public:
        
  // Standard ctor, where user provides the names of the input root files
  // and optionally the name of the output ROOT histogram file
  RootThreshEvol(vector<string> &digiFileNames,
                 TeData  &data, TeCfg &cfg);

  // standard dtor
  ~RootThreshEvol();

  // processes one 'event', as a collection of interactions.
  // loads up histograms.
  void DigiCal();

  // loops through all events in file
  void fillThreshHists();  // filling threshold histograms
  void fillPedHists(); // filling pedestal histograms

  /// check to see if there are nEvt following the current event.  
  /// return # of events left to process (up to Req amount)
  int chkForEvts(int nEvt);
  /// retrieve new event from digi root file
  UInt_t getEvent(UInt_t iEvt);


private:

  TeData &m_teData;
  TeCfg  &m_cfg;
  int m_evtId;
  CalVec<FaceIdx,float> m_adc;
  bool m_fle[2][8];
};


RootThreshEvol::RootThreshEvol(vector<string> &digiFileNames, TeData  &data, TeCfg &cfg)
  : RootFileAnalysis(vector<string>(0), digiFileNames, vector<string>(0)),
    m_teData(data),
    m_cfg(cfg)
{
  
  //
  //  COMMENT OUT ANY BRANCHES YOU ARE NOT INTERESTED IN.
  //
  if (m_digiEnabled) {
    m_digiChain.SetBranchStatus("*",0);  // disable all branches
    // activate desired brances
    m_digiChain.SetBranchStatus("m_cal*",1);
    m_digiChain.SetBranchStatus("m_eventId", 1);
  }

  
}

// default dstor
RootThreshEvol::~RootThreshEvol() {
}

// compiles stats for each test type.

void RootThreshEvol::fillThreshHists()
{
  // Purpose and Method:  Event Loop

 
  for( int itp=0; itp<m_cfg.nTimePoints; itp++){
    int lastEvt = chkForEvts(m_cfg.nEvtsPerPoint);
 

    // BEGINNING OF EVENT LOOP
    for (Int_t iEvt=m_startEvt; iEvt<lastEvt; iEvt++) {
      if (!getEvent(iEvt)) {
        m_ostrm << "Warning, event " << iEvt << " not read." << endl;
        continue;
      }
      // Digi ONLY analysis
      if (m_digiEvt){
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

          commonRootData::CalXtalId id = cdig.getPackedId();  // get interaction information
          ColNum col = id.getColumn();

          TwrNum twr = id.getTower();
          LyrNum lyr = id.getLayer();
          // Loop through each readout on current xtal
          int numRo = cdig.getNumReadouts();
          for (int iRo=0; iRo<numRo; iRo++){
            const CalXtalReadout &acRo = *(cdig.getXtalReadout(iRo));
            for (FaceNum face; face.isValid(); face++) {
              RngNum rng = acRo.getRange((commonRootData::CalXtalId::XtalFace)(short)face);
              // only interested in LEX8 range
              if(rng == 0){
                FaceIdx faceIdx(twr,lyr,col,face);
                float ped = m_teData.getPed(faceIdx,itp);
                m_adc[faceIdx]  = acRo.getAdc((commonRootData::CalXtalId::XtalFace)(short)face)-ped;
              }

            } // foreach face
          } // foreach readout
        } // foreach xtal

        for (int l = 0;l<8;l++){                
          for (int c = 0;c<12;c++){
            bool trg_other_layers = false;
            for (int s=0;s<2;s++)
              for(int ll=0;ll<8;ll++)
                if(l != ll)trg_other_layers = trg_other_layers || m_fle[s][ll];
            for (int s=0;s<2;s++){
              bool no_trg_layer = trg_other_layers;
              for (int cc = 0;cc<12;cc++){
                if (cc != c){
                  FaceIdx face(0,l,cc,s);
                  no_trg_layer = no_trg_layer && (m_adc[face] < 50);
                }
              }
              FaceIdx faceIdx(0,l,c,s);
              if(no_trg_layer){
                m_teData.fillMuHist(faceIdx,itp,m_adc[faceIdx]);
                if(m_fle[s][l])m_teData.fillTrigHist(faceIdx,itp,m_adc[faceIdx]);
              }
            }
          }
        }
                
      }
   
    }  // end analysis code in event loop
  }
}
void RootThreshEvol::fillPedHists(){
  for( int itp=0; itp<m_cfg.nTimePoints; itp++){
    int lastEvt = chkForEvts(m_cfg.nEvtsPerPoint);

    // Basic digi-event loop
    for (Int_t iEvt = m_startEvt; iEvt < lastEvt; iEvt++) {
      if (!getEvent(iEvt)) {
        m_ostrm << "Warning, event " << iEvt << " not read." << endl;
        continue;
      }

      const TObjArray* calDigiCol = m_digiEvt->getCalDigiCol();
      if (!calDigiCol) {
        cerr << "no calDigiCol found for event#" << iEvt << endl;
        continue;
      }

      CalDigi *pCalDigi = 0;

      for( int cde_nb=0; (pCalDigi=(CalDigi*) calDigiCol->At(cde_nb)); cde_nb++ ) { //loop through each 'hit' in one event
        CalDigi &calDigi = *pCalDigi; // use reference to avoid -> syntax
        const CalXtalReadout& readout=*calDigi.getXtalReadout(LEX8); // get LEX8 data

        // check that we are in the expected readout mode
        RngNum rngP = readout.getRange(commonRootData::CalXtalId::POS);
        RngNum rngN = readout.getRange(commonRootData::CalXtalId::NEG);
        if (rngP != LEX8 || rngN != LEX8) {
          ostringstream tmp;
          tmp << __FILE__  << ":" << __LINE__ << " " 
              << "Event# " << m_evtId << " 1st range shold be LEX8, unexpected trigger mode!";
          throw tmp.str();
        }
      
        commonRootData::CalXtalId id(calDigi.getPackedId()); // get interaction information
        LyrNum lyr = id.getLayer();
        //int tower = id.getTower();
        ColNum col = id.getColumn();

        float adcP = readout.getAdc(commonRootData::CalXtalId::POS);
        float adcN = readout.getAdc(commonRootData::CalXtalId::NEG);

        FaceIdx faceIdx(0,lyr,col,POS_FACE);
        m_teData.fillPedHist(faceIdx,itp,adcP);

        faceIdx.setFace(NEG_FACE);
        m_teData.fillPedHist(faceIdx,itp,adcN);

      }
    }
  }
}
int RootThreshEvol::chkForEvts(int nEvts) {
  Int_t nEntries = getEntries();
  m_ostrm << "\nTotal num Events in File is: " << nEntries << endl;

  if (nEntries <= 0 || m_startEvt == nEntries) // have already reached end of chain.
    throw string("No more events available for processing");

  int lastReq = nEvts + m_startEvt;

  // CASE A: we have enough events
  if (lastReq <= nEntries) return lastReq;

  // CASE B: we don't have enough
  int evtsLeft = nEntries - m_startEvt;
  m_ostrm << " EOF before " << nEvts << ". Will process remaining " <<
    evtsLeft << " events." << endl;
  return evtsLeft;
}

UInt_t RootThreshEvol::getEvent(UInt_t ievt) {
  if (m_digiEvt) m_digiEvt->Clear();

  int nb = RootFileAnalysis::getEvent(ievt);
  if (m_digiEvt && nb) { //make sure that m_digiEvt is valid b/c we will assume so after this
    m_evtId = m_digiEvt->getEventId();
    if(m_evtId%1000 == 0)
      m_ostrm << " event " << m_evtId << endl;
  }

  return nb;
}


int main(int argc, char **argv) {
  // Load xml config file
  string cfgPath;
  if(argc > 1) cfgPath = argv[1];
  else cfgPath = "../src/ThreshEvol_option.xml";

  
  TeCfg cfg;
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
    
    TeData data(cfg);

    {
      vector<string> digiFileNames;
      digiFileNames.push_back(cfg.rootFile);
      RootThreshEvol rd(digiFileNames,data,cfg);  
      rd.fillPedHists();
    }
    data.fitPedHists();

    {
      vector<string> digiFileNames;
      digiFileNames.push_back(cfg.rootFile);
      RootThreshEvol rd(digiFileNames,data,cfg);  
      rd.fillThreshHists();
    }

    data.FitData();
    if (cfg.genTXT) data.writeThreshTXT(cfg.outputTXTPath);
 
    if (cfg.genHistfile) data.flushHists();
  
  } catch (string s) {
    // generic exception handler...  all my exceptions are simple C++ strings
    cfg.ostr << "ThreshEvol:  exception thrown: " << s << endl;
    return -1;
  }

  return 0;
}

