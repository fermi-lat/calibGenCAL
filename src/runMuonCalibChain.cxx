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
    inputFile.open("../src/muonCalibChain_option.dat");
  }

  std::string temp;
  getline(inputFile, temp);

  std::vector<std::string> digiFileNames;
  std::string::size_type pos = 0;
  for( ; ; ) {

    string::size_type i = temp.find(' ', pos);
    if(i != string::npos) {
      digiFileNames.push_back(temp.substr(pos, i-pos));
    }
    else {

      std::string lastFile = temp.substr(pos);
      if(lastFile.find("digi") != string::npos) {
		  digiFileNames.push_back(lastFile);
		  break;
      }
      else {
		  break;
      }

    }

    pos = i + 1;
  }

  // Build chain of input filenames
  TChain* digiChain = new TChain("Digi");
  for(std::vector<std::string>::const_iterator itr = digiFileNames.begin();
      itr != digiFileNames.end(); ++itr) {
    std::cout << "digi file: " << *itr << endl;
    digiChain->Add(itr->c_str());
  }

  std::string pedFile;
  inputFile >> pedFile;
  std::cout << "ped text file: " << pedFile << std::endl;

  std::string corrpedFile;
  inputFile >> corrpedFile;
  std::cout << "correlated ped text file: " << corrpedFile << std::endl;

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

  std::string asymFile;
  inputFile >> asymFile;
  std::cout << "asymmetry file: " << asymFile << std::endl;

  // first pass
  {
    muonCalib r(digiChain, 0, 0, pedHist.c_str());

    // Default settings are FILLPEDHIST, basically fills up the pedestal histograms w/ all data points
    r.Go(10000);
    r.Rewind();
    r.FitPedHist();   //loops through & finds mean & rms pedestal for every layer/xtal/end, store results in m_calPed*

    r.PrintCalPed(pedFile.c_str());
    r.Rewind();

    r.SetFillPedHist4Ranges();  //LOAD up rawAdcHist, pdahist
    r.Go(10000);
    r.FitPedHist();             // fit pdaHist for pedestals.  store data in m_calPed*
    r.PrintCalPed(pedFile.c_str());

    r.SetFillCorrPedHist2Ranges();
    r.Go(10000);                // fill rawadchist,fill corrpdahist MINUS pedestal
    r.FitCorrPedHist();         // run gaussian fit on corrpdahist, fill m_calCorr*
    r.PrintCalCorrPed(corrpedFile.c_str());

    r.Rewind();
    r.SetFillRatHist();
    r.Go(1000000);               // fill rawadchist, fill ratntup w/ pedestal corrected data fill a,ar,fill gx,gy
	 // fill LTOT array, MAXNL &NTOT,call some graph functions,fill TX,TY, more
    r.FitRatHist();

    r.WriteMuSlopes(slopeFile.c_str());
    r.WriteMuPeaks(peakFile.c_str());

    r.WriteHist();

  }

  // second pass to calibrate muon peak

  {
    muonCalib r(digiChain, 0, 0, peakHist.c_str());

    r.ReadCalPed(pedFile.c_str());     //load up m_calPed*
    r.ReadMuSlopes(slopeFile.c_str()); //load up m_calSlopes

    // read in m_calCorr, correct difference in gain between two ends of a crystal
    r.ReadMuPeaks(peakFile.c_str());   //load up m_calCorr
//	r.ReadAsymTable(asymFile.c_str());  
    // has to fit muon peak twice, first time to correct gains for event
    // selection
    r.Rewind();
    r.HistClear();

    r.SetFillMuHist(); //fills thrhist,rawhist
	r.SetAsymCorrNone();
    r.Go(1000000);
	r.WriteAsymTable(asymFile.c_str());

    r.FitMuHist();  //uses thrhist, , created m_cal_Corr, m_muRelSigma

    // after fitting, event selection may change, so need to refit it
    r.Rewind();
    r.HistClear();
	r.ReadAsymTable(asymFile.c_str());
    r.SetFillMuHist();
	r.SetAsymCorrSpline();
    r.Go(1000000);

    r.FitMuHist();

    r.WriteMuPeaks(peakFile.c_str());

    r.WriteHist();

  }

  return 0;
}
