#include "muonCalib.h"
#include "TSystem.h"
#include <iostream>
#include <string>
#include <fstream>

int main(int argn, char** argc) {

  std::ifstream inputFile;

  if(argn > 1) {
    inputFile.open(argc[1]);
  }
  else {
    inputFile.open("../src/muonCalib_option.dat");
  }

  std::string digiFileName;
  inputFile >> digiFileName;
  std::cout << "digi root file: " << digiFileName << std::endl;

  std::string pedFile;
  inputFile >> pedFile;
  std::cout << "ped text file: " << pedFile << std::endl;

  std::string corrpedFile;
  inputFile >> corrpedFile;
  std::cout << "corrped text file: " << corrpedFile << std::endl;

  std::string slopeFile;
  inputFile >> slopeFile;
  std::cout << "slope text file: " << slopeFile << std::endl;

  std::string peakFile;
  inputFile >> peakFile;
  std::cout << "peak text file: " << peakFile << std::endl;

  std::string pedHist;
  inputFile >> pedHist;
  std::cout << "ped hist file: " << pedHist << std::endl;

  std::string peakHist;
  inputFile >> peakHist;
  std::cout << "peak hist file: " << peakHist << std::endl;

  std::string pedFileXML;
  inputFile >> pedFileXML;
  std::cout << "ped xml file: " << pedFileXML << std::endl;

  std::string corrpedFileXML;
  inputFile >> corrpedFileXML;
  std::cout << "corrped xml file: " << corrpedFileXML << std::endl;

  std::string slopeFileXML;
  inputFile >> slopeFileXML;
  std::cout << "slope xml file: " << slopeFileXML << std::endl;

  std::string asymFileXML;
  inputFile >> asymFileXML;
  std::cout << "asym xml file: " << asymFileXML << std::endl;

  std::string peakFileXML;
  inputFile >> peakFileXML;
  std::cout << "peak xml file: " << peakFileXML << std::endl;

  inputFile.close();

  // first pass
  {
	 std::vector<std::string> digiFileNames;
	 digiFileNames.push_back(digiFileName);
	 muonCalib r(&digiFileNames, 0, 0, pedHist.c_str());

    r.Go(10000);
    r.Rewind();
    r.FitPedHist();

    r.PrintCalPed(pedFile.c_str());
    r.Rewind();

    r.SetFillPedHist4Ranges();
    r.Go(10000);
    r.FitPedHist();
	 std::cout << std::endl << __FILE__ << "(" << __LINE__ << ")" << " Writing pedestals...\n";
    r.PrintCalPed(pedFile.c_str());
	 r.WritePedXML(pedFileXML.c_str());

    r.SetFillCorrPedHist2Ranges();
    r.Go(10000);
    r.FitCorrPedHist();
	 std::cout << std::endl << __FILE__ << "(" << __LINE__ << ")" << " Writing corrped...\n";
    r.PrintCalCorrPed(corrpedFile.c_str());
    r.WriteCorrPedXML(corrpedFileXML.c_str());

    r.Rewind();
    r.SetFillRatHist();
    r.Go(1000000);

    r.FitRatHist();

	 std::cout << std::endl << __FILE__ << "(" << __LINE__ << ")" << " Writing mu slopes...\n";
    r.WriteMuSlopes(slopeFile.c_str());
	 r.WriteMuSlopesXML(slopeFileXML.c_str());
    r.WriteMuPeaks(peakFile.c_str());
    r.WriteHist();

  }

  // second pass to calibrate muon peak

  {
	 std::vector<std::string> digiFileNames;
	 digiFileNames.push_back(digiFileName);
    muonCalib r(&digiFileNames, 0, 0, peakHist.c_str());

    r.ReadCalPed(pedFile.c_str());
    r.ReadMuSlopes(slopeFile.c_str());

    // read in m_calCorr, correct difference in gain between two ends of a crystal
    r.ReadMuPeaks(peakFile.c_str());

    // has to fit muon peak twice, first time to correct gains for event
    // selection
    r.Rewind();
    r.HistClear();

    r.SetFillMuHist();
    r.Go(1000000);

    r.FitMuHist();

    // after fitting, event selection may change, so need to refit it
    r.Rewind();
    r.HistClear();

    r.SetFillMuHist();
    r.Go(1000000);

    r.FitMuHist();

	 std::cout << std::endl << __FILE__ << "(" << __LINE__ << ")" << " Writing mu peaks...\n";
    r.WriteMuPeaks(peakFile.c_str());
	 r.WriteMuPeaksXML(peakFileXML.c_str());

    std::cout << std::endl << __FILE__ << "(" << __LINE__ << ")" << " Writing histograms...\n";
    r.WriteHist();

  }

  return 0;
}
