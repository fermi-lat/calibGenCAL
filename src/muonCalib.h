#ifndef muonCalib_h
#define muonCalib_h 1

#if !defined(__CINT__)
// Need these includes if we wish to compile this code
#include "TROOT.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TMath.h"
#include "TH1.h"
#include "TH2F.h"
#include "TF1.h"
#include "TProfile.h"
#include "TNtuple.h"
#include "TStyle.h"
#include "TCollection.h"  // Declares TIter
#include "digiRootData/DigiEvent.h"
#include "reconRootData/ReconEvent.h"
#include "mcRootData/McEvent.h"
#include <iostream>
#else  // for interactive use
//#include "iostream.h"
class DigiEvent;
class ReconEvent;
class McEvent;
#endif

class muonCalib {
public :
    /// Histogram file
    TH1F        phaArr_A[500];
    TH1F        phaArr_B[500];
    TFile       *histFile;
    /// Input digitization file
    TFile       *digiFile;   
    /// Input reconstruction file
    TFile       *reconFile;  
    /// Input monte carlo file
    TFile       *mcFile;     
    /// pointer to the digi tree
    TTree       *digiTree;    
    /// pointer to the reconstruction tree
    TTree       *reconTree;
    /// pointer to the monte carlo tree
    TTree       *mcTree;      
    /// Optional TChain input
    TChain      *m_digiChain, *m_recChain, *m_mcChain;
    /// pointer to a DigiEvent
    DigiEvent   *evt;
    /// pointer to a ReconEvent
    ReconEvent  *rec;
    /// Pointer to a McEvent
    McEvent     *mc;
    /// name of the output histogram ROOT file
    const char        *m_histFileName; 
    /// Arrays that contain pointers to the TFile, TTree, and TChains
    TObjArray   *fileArr, *treeArr, *chainArr;

    

                
    TObjArray* pedhist;
    TObjArray* corrpedhist;
    TObjArray* pdahist;
    TObjArray* corrpdahist;

    // raw Histogram actually containing pedestal corrected histograms
    // it is sum of two faces 
    TObjArray* rawhist;

    // raw adc histograms without any correction, for all 4 ranges
    TObjArray* rawAdcHist;

    // fully corrected muon adc. Correction include: pedestal, gain, cos(theta)
    // and light attenuation
    TObjArray* thrhist;

    TObjArray* adjhist;
    TObjArray* midhist;
    TObjArray* poshist;
    TObjArray* rathist;
    TObjArray* ratfull;
    TObjArray* asyhist;
    TObjArray* reshist;
    TObjArray* ratntup;
    TObjArray* asycalib;
    TCanvas* c1;

    // Graph of column(y axis) vs. layer(x axis) for a measure X layer
    TGraph* gx;     

    // Graph of column(y axis) vs. layer(x axis) for a measure Y layer
    TGraph* gy;

    // functions used to fit gx and gy
    TF1* xline;
    TF1* yline;

    // not used
    TGraphErrors* glx;
    TGraphErrors* gly;

    TF1* xlongl;
    TF1* ylongl;
    TF1* land;
    int digi_select[8][12];

    // pedestal and gain corrected acd values
    float a[8][12][2];

    // pedestal corrected adc values
    float ar[8][12][2];

         double prevTimeStamp;

        enum GO_TYPE { FILLPEDHIST, FILLMUHIST, FILLRATHIST, FILLPEDHIST4RANGES,
                       FILLCORRPEDHIST, FILLCORRPEDHIST2RANGES };
        GO_TYPE go_type;
        void SetFillPedHist() { go_type = FILLPEDHIST;}
        void SetFillPedHist4Ranges() { go_type = FILLPEDHIST4RANGES;}
        void SetFillCorrPedHist() { go_type = FILLCORRPEDHIST;}
        void SetFillCorrPedHist2Ranges() { go_type = FILLCORRPEDHIST2RANGES;}
        void SetFillMuHist() { go_type = FILLMUHIST;}
        void SetFillRatHist() { go_type = FILLRATHIST;}


  /// Default ctor, requires that that user calls muonCalib::Init
  /// to setup access to specific ROOT files.
    muonCalib(); 
    
  /// Standard ctor, where user provides the names of the input root files
  /// and optionally the name of the output ROOT histogram file
    muonCalib( 
        const char* digiFileName, 
        const char* reconFileName="", 
        const char* mcFileName="", 
        const char *histFileName="Histograms.root"); 

  /// Special ctor which accepts TChains for input files
    muonCalib( 
        TChain *digiChain, 
        TChain *recChain = 0, 
        TChain *mcChain = 0, 
        const char *histFileName="Histograms.root");

    ~muonCalib();  

    /// start next Go with this event
    void StartWithEvent(Int_t event) { m_StartEvent = event; };  
    /// reset for next Go to start at beginning of file 
    void Rewind() { m_StartEvent = 0; }; 
    /// allow the user to specify their own file name for the output ROOT file
    void SetHistFileName(const char *histFileName) 
      { m_histFileName = histFileName; }

    /// re-init with these input Root files
    void Init(  const char* digiFileName="", 
        const char* reconFileName="", 
        const char* mcFileName=""); 
    /// define user histograms, ntuples and other output objects that will be saved to output
    void HistDefine();   
    /// make list of user histograms and all objects created for output
    void MakeHistList(); 
    /// write the existing histograms and ntuples out to file
    void WriteHist(){ if (histFile) histFile->Write(); };
    /// Reset() all user histograms
    void HistClear(); 
  /// Fit calorimeter pedestal histograms
  void FitPedHist();
  /// Fit calorimeter pedestal histograms for both ranges of a same diode
  void FitCorrPedHist();
  /// Fit calorimeter log ends ratio vs position histogram
  void FitRatHist();
  ///Fit log ends signal histograms with Gaus convoluted Landau functions
  /// and log normal functions
  void FitMuHist();
  /// find peak position for Gaus convoluted Landau functions
  void GetMPV( TF1* function, Double_t precision, Double_t min, Double_t max );
  ///Write mu peak positions for all log ends into the file mupeaks.txt
  void WriteMuPeaks(const char* fileName);
  void ReadMuPeaks(const char* fileName);
  ///Write mu slopes for all log ends into the file muslopes.txt
  void WriteMuSlopes(const char* fileName);
  void ReadMuSlopes(const char* fileName);
    /// Retrieve a pointer to an object stored in our output ROOT file
    TObject* GetObjectPtr(const char *tag) { return (m_histList->FindObject(tag)); };
    /// process events
    void Go(Int_t numEvents=100000); 
    /// returns number of events in all open files
    UInt_t GetEntries() const;
    /// retrieve a pointer to event number.
    UInt_t GetEvent(UInt_t ievt);

    void ReadCalPed(const char* fileName);
    void PrintCalPed(const char* fileName);
    void PrintCalCorrPed(const char* fileName);
    void ZeroPeds();

private:
    /// starting event number
    Int_t m_StartEvent;
    /// list of user histograms
    THashList *m_histList;

    // array containing gain corrections, determined as the ration of 1000 to 
    // mean muon signal from each crystal face; after application of these 
    // corrections the average muon signal become 1000 at all crystal faces
    float m_calCorr[8][12][2];

    // sigma/mean in the landau fit to muon signal
    float m_muRelSigma[8][12][2];

    float m_calSlopes[8][12];
    float m_calPed[4][8][12][2];
    float m_calPedRms[4][8][12][2];
    float m_calCorrPed[4][8][12][2];
    float m_calCorrPedRms[4][8][12][2];
    float m_calCorrPedCos[2][8][12][2];
    

    /// reset all member variables
    void Clear(); 

    /// Setup the Monte Calro output histograms
    void McHistDefine();
  /// Setup the Digitization output histograms
    void DigiHistDefine();
  /// Setup the Reconstruction output histograms
    void ReconHistDefine();
    
    /// event processing for the monte carlo data
    void McData();

    /// event processing for the digi TKR data
    void DigiTkr();
  /// event processing for digi CAL data
    void DigiCal();
  /// event processing for digi ACD data
    void DigiAcd();
    
    /// event processing for the recon TKR data
    void ReconTkr();
  /// event processing for the recon CAL data
    void ReconCal();
  /// event processing for the recon ACD data
    void ReconAcd();
};


inline muonCalib::muonCalib() 
{
    Clear();
    ZeroPeds();
}

inline muonCalib::muonCalib(const char* digiFileName, 
                                   const char* reconFileName, 
                                   const char* mcFileName, 
                                   const char* histFileName)
{
  // Purpose and Method:  Standard constructor where the user provides the 
  //  names of input ROOT files and optionally the name of the output ROOT
  //  histogram file.
    printf(" opening files:\n\tdigi:\t%s\n\trecon:\t%s\n\tmc:\t%s\n",
    digiFileName, reconFileName, mcFileName);
    
    Clear();
    
    SetHistFileName(histFileName);
    HistDefine();
    MakeHistList();
    
    Init(digiFileName, reconFileName, mcFileName);
    ZeroPeds();
}

inline muonCalib::muonCalib(TChain *digiChain, 
                                   TChain *recChain, 
                                   TChain *mcChain, 
                                   const char *histFileName) {
    Clear();
    
    SetHistFileName(histFileName);
    HistDefine();
    MakeHistList();
    
    if (chainArr) delete chainArr;
    chainArr = new TObjArray();
    
    if (mcChain != 0) {
        m_mcChain = mcChain;
        mc = 0;
        m_mcChain->SetBranchAddress("McEvent",&mc);
        chainArr->Add(m_mcChain);
    }

    if (digiChain != 0) {
        evt = 0;
        m_digiChain = digiChain;
        m_digiChain->SetBranchAddress("DigiEvent",&evt);
        chainArr->Add(m_digiChain);
    }
    
    if (recChain != 0) {
        m_recChain = recChain;
        rec = 0;
        m_recChain->SetBranchAddress("ReconEvent",&rec);
        chainArr->Add(m_recChain);
    }
    ZeroPeds();
    
}


inline muonCalib::~muonCalib() {
    histFile->Close();
    
    //if (m_histList) delete m_histList;
    
    if (histFile) delete histFile;
    
    if (digiFile) delete digiFile;
    if (reconFile) delete reconFile;
    if (mcFile) delete mcFile;
    
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
    
    digiTree = 0;
    reconTree = 0;
    mcTree = 0;
    
    if (fileArr) delete fileArr;
    if (treeArr) delete treeArr;
    if (chainArr) delete chainArr;

  Clear();
}



inline void muonCalib::Init(const char* digiFileName, const char* reconFileName, const char* mcFileName)
{
    // Purpose and Method:  Re-initialize file, tree, event pointers, using the 
  //   input ROOT files.  Histograms are *not* cleared.
    
    if (fileArr) delete fileArr;
    fileArr = new TObjArray();
    
    if (treeArr) delete treeArr;
    treeArr = new TObjArray();
         
    if (mcFile) {
        delete mc; 
        mc = 0;
        mcTree = 0;
        delete mcFile; 
        mcFile = 0;
    }
    
    if (mcFileName != "") {
        mcFile = new TFile(mcFileName);
        if (mcFile->IsOpen() == kTRUE) {
            mcTree = (TTree*)gDirectory->Get("Mc");
            mc = 0;
            mcTree->SetBranchAddress("McEvent",&mc);
            fileArr->Add(mcFile);
            treeArr->Add(mcTree);
        } else {
            mcFile = 0;
            std::cout << "mc data file could not be opened!!" << std::endl;
        }
    }

    if (digiFile) {
        delete evt; 
        evt = 0;
        digiTree = 0;
        delete digiFile; 
        digiFile = 0;
    }
    
    if (digiFileName != "") {
        digiFile = new TFile(digiFileName);
        if (digiFile->IsOpen() == kTRUE) {
            digiTree = (TTree*)gDirectory->Get("Digi");
            evt = 0;
            digiTree->SetBranchAddress("DigiEvent",&evt);
            fileArr->Add(digiFile);
            treeArr->Add(digiTree);
        } else {
            digiFile = 0;
            std::cout << "digi data file could not be opened!!" << std::endl;
        }
    }
    
    if (reconFile) {
        delete rec; 
        rec = 0;
        reconTree = 0;
        delete reconFile;
        reconFile = 0;
    }
    
    if (reconFileName != "") {
        reconFile = new TFile(reconFileName);
        if (reconFile->IsOpen() == kTRUE) {
            reconTree = (TTree*)gDirectory->Get("Recon");
            rec = 0;
            reconTree->SetBranchAddress("ReconEvent",&rec);
            fileArr->Add(reconFile);
            treeArr->Add(reconTree);
        } else {
            reconFile = 0;
            std::cout << "recon data file could not be opened!!" << std::endl;
        }
    }
    
    m_StartEvent = 0;
    
}


inline UInt_t muonCalib::GetEvent(UInt_t ievt) {
    // Purpose and Method:  Get the event, ievt, for all trees
    //    We could be processing single files or chains, 
  //    This routine handles both casees.

    // if using regular trees - we check the array of open trees and
    // move the event pointer to the requested event
    UInt_t nb = 0;
    if (treeArr) {
        for (Int_t i = 0; i < treeArr->GetEntries(); i++) {
            nb += ((TTree*)treeArr->At(i))->GetEvent(ievt);
        }
        return nb;
    }
    
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


inline UInt_t muonCalib::GetEntries() const {    
    // Purpose and Method:  Determine the number of events to iterate over
    //   checking to be sure that the requested number of events is less than
    //   the min number of events in all files

    UInt_t nentries = 0;
    if (treeArr) {
        nentries = ((TTree*)treeArr->At(0))->GetEntries();
        for (Int_t i = 1; i < treeArr->GetEntries(); i++) {
            nentries = TMath::Min(nentries, (UInt_t)((TTree*)treeArr->At(i))->GetEntries());
        }
        return nentries;
    }
    
    if (chainArr) {
        nentries = ((TChain*)chainArr->At(0))->GetEntries();
        for (Int_t i = 1; i < chainArr->GetEntries(); i++) {
            nentries = TMath::Min(nentries, (UInt_t)((TChain*)chainArr->At(i))->GetEntries());
        }
        return nentries;
    }
    
    return nentries;
}


inline void muonCalib::MakeHistList() {
    // Purpose and Method:  Make a THashList of histograms
    //   This avoids the need to refresh the histogram pointers
    
    if (m_histList) delete m_histList;
    
    m_histList = new THashList(30, 5);
    
    TList* list = histFile->GetList();
    TIter iter(list);
    
    TObject* obj = 0;
    
    while (obj=iter.Next()) {
        m_histList->Add(obj);
    }
}

inline void muonCalib::HistClear() {
    // Purpose and Method:  Clear histograms by iterating over the THashList
    
    if (!m_histList) return;
    
    TIter iter(m_histList);
    
    TObject* obj = 0;
    
    while ( obj=(TObject*)iter.Next() ) {
        ((TH1*)obj)->Reset();        
    }
}

inline void muonCalib::ZeroPeds() {
    for (int lyr = 0; lyr < 8; lyr++){
        for (int col = 0; col < 12; col++) {
          m_calSlopes[lyr][col] = 0.0;
          for (int side = 0; side < 2; side++) {
            m_calCorr[lyr][col][side]=1.0;
            m_muRelSigma[lyr][col][side]=0.0;
            for (int i = 0; i < 4; i++) {
              m_calPed[i][lyr][col][side]=0.0;
              m_calPedRms[i][lyr][col][side]=0.0;
              m_calCorrPed[i][lyr][col][side]=0.0;
              m_calCorrPedRms[i][lyr][col][side]=0.0;
            }
            m_calCorrPedCos[0][lyr][col][side]=0.0;
            m_calCorrPedCos[1][lyr][col][side]=0.0;
          }
       }
    }

}

inline void muonCalib::Clear() {
    histFile = 0; 
    m_histList = 0;
    
    digiFile = 0; 
    reconFile = 0;
    mcFile = 0;
    
    digiTree = 0; 
    reconTree = 0;
    mcTree = 0;
    
    m_digiChain = 0;
    m_recChain = 0;
    m_mcChain = 0;
    
    evt = 0;
    rec = 0;
    mc = 0;
    
    fileArr = 0;
    treeArr = 0;
    chainArr = 0;
    ZeroPeds();
    go_type = FILLPEDHIST;
}

#endif
