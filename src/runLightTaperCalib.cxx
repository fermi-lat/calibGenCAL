#include <fstream>
#include <string>
#include "lightTaperCalib.h"

int main(int argn, char** argc) {

  std::ifstream inputFile;

  if(argn > 1) {
    inputFile.open(argc[1]);
  }
  else {
    inputFile.open("../src/lightTaperCalib_option.dat");
  }

  std::string digiFileName;
  inputFile >> digiFileName;

  std::string reconFileName;
  inputFile >> reconFileName;

  std::string txtFileName;
  inputFile >> txtFileName;

  std::string rootFileName;
  inputFile >> rootFileName;

  lightTaperCalib calib;

  calib.readPedestal("/nfs/farm/g/glast/u03/EM2003/rootFiles/v3r3p2/muon_ver/calib/muped__031005015905_031005040315.txt");

  calib.genLightTaper(digiFileName.c_str(), reconFileName.c_str(), 
		      txtFileName.c_str(), rootFileName.c_str());

}
