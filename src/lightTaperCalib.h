#ifndef lightTaperCalib_Class
#define lightTaperCalib_Class 1

#include <string>
#include "TROOT.h"
#include "TGraphErrors.h"
#include "TChain.h"
#include "TH1F.h"
#include "TProfile.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TNtuple.h"
#include "digiRootData/DigiEvent.h"
#include "reconRootData/ReconEvent.h"

class lightTaperCalib {
 public:

  lightTaperCalib();
  ~lightTaperCalib();

  void genLightTaper(const char* digi, const char* recon, 
		     const char* outputTxt, const char* outputRoot);
  void genLightTaper(TChain* digi, TChain* recon, 
		     const char* outputTxt, const char* outputRoot);

  void readPedestal(const char* fileName);

 private:

  void fitTaper();

  void analyzeEvent(int nEvent);

  void getCalEnergy();

  bool passCut();

  void extrapolate(float& x, float& y, float z) const;

  void fillCALAdc(int layer, int col, float pos); 

  static const int g_nLayer = 8;
  static const int g_nCol = 12;
  static const int g_nFace = 2;
  static const float g_calLen = 333.;
  static const int g_nDiv = 40;

  // z coordinate of center of the top crystal layer, rough estimate 
  static const float g_calTop = -57.;
  // gap in z between center of the crystal in adjacent layer, rough estimate
  static const float g_calDz = -21.;


  TGraphErrors* m_taperPos[g_nLayer][g_nCol][g_nFace];

  // an array of pointers to histograms storing adc values at longitudinal 
  // positions along the crystal
  TH1F* m_taper[g_nLayer][g_nCol][g_nFace][g_nDiv];

  TH1F* m_eneHist;

  TNtuple* m_tuple;

  TFile* m_reconFile;
  TTree* m_reconTree;
  ReconEvent* m_reconEvent;

  TFile* m_digiFile;
  TTree* m_digiTree;
  DigiEvent* m_digiEvent;

  TFile* m_taperFile;

  // reconstructed event vertex and direction
  TVector3 m_pos, m_dir;

  // calibrated energy in each crystal
  float m_xtalEne[g_nLayer][g_nCol];

  float m_pedestal[g_nLayer][g_nCol][g_nFace];

  //name of ascii output taper file
  std::string m_txtOutput;
};

#endif
