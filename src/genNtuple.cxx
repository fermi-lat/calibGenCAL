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
#include <fstream>
#include <iostream>

TROOT gRoot("root", "root");

int main(int argc, char** argv)
{
  if(argc != 5) {
    std::cout << "Need 4 arguments! " << std::endl << "1. output const root file 2. pedestal text file 3. muslope text file 4. mupeak text file" << std::endl;
    exit(1);
  }

  std::cout << "Processing: " << std::endl << argv[2] << std::endl << argv[3] << std::endl 
       << argv[4] << std::endl;

  TFile f(argv[1], "RECREATE");
  TNtuple n1("ped", "ped", "Layer:Col:Side:Range:Ped:Rms");
  TNtuple n2("slope", "slope", "Layer:Col:Slope");
  TNtuple n3("peak", "peak", "Layer:Col:Side:Peak:Rms");

  int layer, col, side, range;
  double ped, pedRms;

  ifstream f1(argv[2]);
  if(! f1.is_open()) {
    std::cout << "f1 can not be open!" << std::endl;
    exit(1);
  }
 
  while(f1 >> layer >> col >> side >> range >> ped >> pedRms) {

    n1.Fill(layer, col, side, range, ped, pedRms);

  }

  ifstream f2(argv[3]);
  double slope;
  while(f2 >> col >> layer >> slope) {

    n2.Fill(layer, col, slope);

  }

  ifstream f3(argv[4]);
  double peak, peakRms;
  while(f3 >> side >> col >> layer >> peak >> peakRms) {

    n3.Fill(layer, col, side, peak, peakRms);

  }

  f.cd();
  n1.Write();
  n2.Write();
  n3.Write();

}
