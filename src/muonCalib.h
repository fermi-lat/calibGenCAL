/*
  muonCalib contains the following features.

  1) event loop.
  2) muon calibration constant generation
  3) related constant storage
  4) related ASCII/XML/ROOT FILE IO
  5) related ROOT histograms

 */

#ifndef muonCalib_h
#define muonCalib_h 1

#include "RootFileAnalysis.h"

//ROOT INCLUDES
#include "TFile.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TF1.h"
#include "TChain.h"

#include <string>
#include <vector>

#if !defined(__CINT__)
// Need these includes if we wish to compile this code
#else  // for interactive use
class DigiEvent;
class ReconEvent;
class McEvent;
#endif

class muonCalib : public RootFileAnalysis {
public :

  enum GO_TYPE { FILLPEDHIST, FILLMUHIST, FILLRATHIST, FILLPEDHIST4RANGES,
					  FILLCORRPEDHIST, FILLCORRPEDHIST2RANGES };
  enum ASYM_CORR_TYPE { NONE, SLOPE, SPLINE };

  void SetAsymCorrNone() {asym_corr_type = NONE;}
  void SetAsymCorrSlope() {asym_corr_type = SLOPE;}
  void SetAsymCorrSpline() {asym_corr_type = SPLINE;}
  void SetFillPedHist() { go_type = FILLPEDHIST;}
  void SetFillPedHist4Ranges() { go_type = FILLPEDHIST4RANGES;}
  void SetFillCorrPedHist() { go_type = FILLCORRPEDHIST;}
  void SetFillCorrPedHist2Ranges() { go_type = FILLCORRPEDHIST2RANGES;}
  void SetFillMuHist() { go_type = FILLMUHIST;}
  void SetFillRatHist() { go_type = FILLRATHIST;}

  /// Default ctor, requires that that user calls muonCalib::Init
  /// to setup access to specific ROOT files.
  muonCalib();

  /// Special ctor which accepts TChains for input files
  muonCalib(std::vector<std::string> *digiFileNames,
				std::vector<std::string> *recFileNames,
				std::vector<std::string> *mcFileNames,
				const char *histFileName="muonCalib_histograms.root");

  ~muonCalib();

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
  
  ///Write mu peak positions for all log ends into an ascii file
  void WriteMuPeaks(const char* fileName);
  int WriteMuPeaksXML(const char *fileName);
  void ReadMuPeaks(const char* fileName);
 
  ///Write mu slopes for all log ends into the file muslopes.txt
  void WriteMuSlopes(const char* fileName);
  int WriteMuSlopesXML(const char *fileName);
  void ReadMuSlopes(const char* fileName);
  /// Retrieve a pointer to an object stored in our output ROOT file

  
  ///Write light asymmetry table from bin content of ratfull histograms to an ascii file
  void WriteAsymTable(const char* fileName);
  ///Read light asymmetry table from an ascii file and create a cubic spline for each crystal
  void ReadAsymTable(const char* fileName);
  int WriteAsymXML(const char *fileName);

  void ReadCalPed(const char* fileName);
  int WritePedXML(const char* fileName);
  void PrintCalPed(const char* fileName);
  
  int ReadCorrPed(const char *fileName);
  void PrintCalCorrPed(const char* fileName);
  int WriteCorrPedXML(const char *fileName);

  /// process events
  void Go(Int_t numEvents=100000);

private:
  void ZeroPeds();
  TObject* GetObjectPtr(const char *tag) { return (m_histList->FindObject(tag)); };
  /// define user histograms, ntuples and other output objects that will be saved to output
  void HistDefine();
  /// make list of user histograms and all objects created for output
  void MakeHistList();
  /// write the existing histograms and ntuples out to file
  /// allow the user to specify their own file name for the output ROOT file
  void SetHistFileName(const char *histFileName)
  { m_histFileName = histFileName; }


  /// populate m_asymTable from histograms
  int PopulateAsymArray();
  ASYM_CORR_TYPE asym_corr_type;

  GO_TYPE go_type;

  /// Histogram file
  TFile       *histFile;

  /// name of the output histogram ROOT file
  const char        *m_histFileName;

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
  TObjArray* asymCorr;

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
  float m_asymTable[8][12][10];

  /// Setup the Digitization output histograms
  void DigiHistDefine();

  /// event processing for digi CAL data
  void DigiCal();

  /// Zeros out all member vars, does NOT free memory,for use in constructor
  void ZeroMembers();

  static const std::string FACE_MNEM[];
  static const std::string RNG_MNEM[];

};

#endif
