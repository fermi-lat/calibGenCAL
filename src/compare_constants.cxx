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

int main()
{
  TFile f("diff_ntuple.root", "RECREATE");
  TNtuple n1("ped", "ped", "Layer:Col:Side:Range:Ped:Rms");
  TNtuple n2("slope", "slope", "Layer:Col:Slope");
  TNtuple n3("peak", "peak", "Layer:Col:Side:Peak:Rms");

  int layer, col, side, range;
  double ped1[8][12][2][4], pedRms1[8][12][2][4];
  double ped2[8][12][2][4], pedRms2[8][12][2][4];
  double slope1[8][12], slope2[8][12];
  double peak1[8][12][2], peakRms1[8][12][2];
  double peak2[8][12][2], peakRms2[8][12][2];

  ifstream f1_1("/nfs/farm/g/glast/u03/EM2003/rootFiles/v3r3p2/muon_ver/calib/muped__030928090737_030928030904.txt");
  if(! f1_1.is_open()) {
    std::cout << "f1_1 can not be open! " << std::endl;
    exit(1);
  }

  while(f1_1 >> layer >> col >> side >> range) {
    f1_1 >> ped1[layer][col][side][range] >> pedRms1[layer][col][side][range];
  }

  ifstream f1_2("/nfs/farm/g/glast/u03/EM2003/rootFiles/v3r3p2/muon_ver/calib/muped__031001010239_031001031445.txt");
  if(! f1_2.is_open()) {
    std::cout << "f1_2 can not be open! " << std::endl;
    exit(1);
  }

  while(f1_2 >> layer >> col >> side >> range) {
    f1_2 >> ped2[layer][col][side][range] >> pedRms2[layer][col][side][range];
  }

  for(int iLayer = 0; iLayer != 8; ++iLayer) {
    for(int iCol = 0; iCol != 12; ++iCol) {
      for(int iSide = 0; iSide != 2; ++iSide) {
	for(int iRange = 0; iRange != 4; ++iRange) {

	  float diff1 = 2.*(ped1[iLayer][iCol][iSide][iRange]-ped2[iLayer][iCol][iSide][iRange])/(ped1[iLayer][iCol][iSide][iRange]+ped2[iLayer][iCol][iSide][iRange]);

	  float diff2 = 2.*(pedRms1[iLayer][iCol][iSide][iRange]-pedRms2[iLayer][iCol][iSide][iRange])/(pedRms1[iLayer][iCol][iSide][iRange]+pedRms2[iLayer][iCol][iSide][iRange]);

	  n1.Fill(iLayer, iCol, iSide, iRange, diff1, diff2);
	}
      }
    }
  }

  ifstream f2_1("/nfs/farm/g/glast/u03/EM2003/rootFiles/v3r3p2/muon_ver/calib/muslope__030928090737_030928030904.txt");
  if(! f2_1.is_open()) {
    std::cout << "f2_1 can not be open! " << std::endl;
    exit(1);
  }

  while(f2_1 >> col >> layer) {
    f2_1 >> slope1[layer][col];
  }

  ifstream f2_2("/nfs/farm/g/glast/u03/EM2003/rootFiles/v3r3p2/muon_ver/calib/muslope__031001010239_031001031445.txt");
  if(! f2_2.is_open()) {
    std::cout << "f2_2 can not be open! " << std::endl;
    exit(1);
  }

  while(f2_2 >> col >> layer) {
    f2_2 >> slope2[layer][col];
  }

  for(int iLayer = 0; iLayer != 8; ++iLayer) {
    for(int iCol = 0; iCol != 12; ++iCol) {

      float diff = 2.*(slope1[iLayer][iCol]-slope2[iLayer][iCol])/(slope1[iLayer][iCol]+slope2[iLayer][iCol]);

      n2.Fill(iLayer, iCol, diff);

    }
  }

  ifstream f3_1("/nfs/farm/g/glast/u03/EM2003/rootFiles/v3r3p2/muon_ver/calib/mupeak__030928090737_030928030904.txt");
  if(! f3_1.is_open()) {
    std::cout << "f3_1 can not be open! " << std::endl;
    exit(1);
  }

  while(f3_1 >> side >> col >> layer) {
    f3_1 >> peak1[layer][col][side] >> peakRms1[layer][col][side];
  }

  ifstream f3_2("/nfs/farm/g/glast/u03/EM2003/rootFiles/v3r3p2/muon_ver/calib/mupeak__030930052646_030930072754.txt");
  if(! f3_2.is_open()) {
    std::cout << "f3_2 can not be open! " << std::endl;
    exit(1);
  }

  while(f3_2 >> side >> col >> layer) {
    f3_2 >> peak2[layer][col][side] >> peakRms2[layer][col][side];
  }

  for(int iLayer = 0; iLayer != 8; ++iLayer) {
    for(int iCol = 0; iCol != 12; ++iCol) {
      for(int iSide = 0; iSide != 2; ++iSide) {

	float diff1 = 2*(peak1[iLayer][iCol][iSide]-peak2[iLayer][iCol][iSide])/(peak1[iLayer][iCol][iSide]+peak2[iLayer][iCol][iSide]);

	float diff2 = 2*(peakRms1[iLayer][iCol][iSide]-peakRms2[iLayer][iCol][iSide])/(peakRms1[iLayer][iCol][iSide]+peakRms2[iLayer][iCol][iSide]);

	  n3.Fill(iLayer, iCol, iSide, diff1, diff2);
      }
    }
  }

  f.cd();
  n1.Write();
  n2.Write();
  n3.Write();

}


