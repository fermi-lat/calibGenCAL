#include "muonCalib.h"
#include "TSystem.h"
#include <iostream>
#include <string>
#include <fstream>

/*! \File runMuonCalib.cxx

\b Phase I


The calibration process is iterative,as shown by the presence of two phases of operation.  After performing data selection on the basis of CAL-determined muon track, Phase I (Figure 3) computes pedestals and performs a linear fit to the asymmetry measured from the central half of each crystal.  The slope of the resulting best fit line (muSlope) is a conversion factor from measured asymmetry to position.  The normalization of the line yields a first estimate of gain (muPeak), under the assumption that the gain is the same for both ends of any given crystal.


\b Phase II


Phase II (Figure 4) uses the results (pedestals, muSlopes, muPeaks) from Phase I, then the Phase I data set to produce an asymmetry table (asymmetry vs position with crystal width granularity) which is written out as text and XML files.  It also collects a gain, position, and path-length corrected pulse height histogram and fits a Landau model to the muon peaks measured at each end.  The results of these fits yield more realistic muPeaks.  Another iteration is performed so that small changes in data selection due to the refined muPeaks can be taken into account.  Finally, muPeaks are written out to text and XML files.


\b Note on readXXX() functions


The application is interspersed with several read() functions which read calibration data from text files that were created earlier in the application.  This allows the developers to comment out earlier portions of the code and save processing time while they are testing later passes.

*/

int main(int argn, char** argc) {

  std::ifstream inputFile;

  if(argn > 1) {
    inputFile.open(argc[1]);
  }
  else {
    inputFile.open("../src/muonCalib_option.dat");
  }

  std::string temp;
  getline(inputFile, temp);

  std::vector<std::string> digiFileNames;
  std::string::size_type pos = 0;
  while (1) {
    std::string::size_type i = temp.find(' ', pos);
    
	 if(i != std::string::npos) {  //'space' delimiter is found, return previous string.
		digiFileNames.push_back(temp.substr(pos, i-pos));
	 } else {                      //delimiter is not found.  if previous string has length, 
		//then treat as filename.
		std::string lastFile = temp.substr(pos);
		if (lastFile.length() > 0) 
        digiFileNames.push_back(lastFile);
		break;
	 }
  
	 pos = i + 1;
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
	 muonCalib r(&digiFileNames, 0, 0, pedHist.c_str());

	 // Default settings are FILLPEDHIST, basically fills up the pedestal histograms w/ all data points
	 r.Go(10000);
	 r.Rewind();
	 r.FitPedHist();   //loops through & finds mean & rms pedestal for every layer/xtal/end, store results in m_calPed*

	 
    r.PrintCalPed(pedFile.c_str());
	 r.Rewind();

	 r.SetFillPedHist4Ranges();  //LOAD up rawAdcHist, pdahist
	 r.Go(10000);
	 r.FitPedHist();             // fit pdaHist for pedestals.  store data in m_calPed*
	 std::cout << std::endl << __FILE__ << "(" << __LINE__ << ")" << " Writing pedestals...\n";
    r.PrintCalPed(pedFile.c_str());
	 r.WritePedXML(pedFileXML.c_str());
    /*
      r.SetFillCorrPedHist2Ranges();
      r.Go(10000);                // fill rawadchist,fill corrpdahist MINUS pedestal
      r.FitCorrPedHist();         // run gaussian fit on corrpdahist, fill m_calCorr*
      r.PrintCalCorrPed(corrpedFile.c_str());
      std::cout << std::endl << __FILE__ << "(" << __LINE__ << ")" << " Writing corrped...\n";
      r.WriteCorrPedXML(corrpedFileXML.c_str());
    */
	 r.Rewind();
	 r.SetFillRatHist();
	 r.Go(300000);               // fill rawadchist, fill ratntup w/ pedestal corrected data fill a,ar,fill gx,gy
	 // fill LTOT array, MAXNL &NTOT,call some graph functions,fill TX,TY, more
	 r.FitRatHist();

	 std::cout << std::endl << __FILE__ << "(" << __LINE__ << ")" << " Writing mu slopes...\n";
    r.WriteMuSlopes(slopeFile.c_str());
	 r.WriteMuSlopesXML(slopeFileXML.c_str());
	 r.WriteMuPeaks(peakFile.c_str());

	 r.WriteHist();

  }

  // second pass to calibrate muon peak

  {
	 muonCalib r(&digiFileNames, 0, 0, peakHist.c_str());

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
	 std::cout << std::endl << __FILE__ << "(" << __LINE__ << ")" << " Writing light_asym table...\n";
    r.WriteAsymTable(asymFile.c_str());
	 r.WriteAsymXML(asymFileXML.c_str());

	 r.FitMuHist();  //uses thrhist, , created m_cal_Corr, m_muRelSigma

    // 	 // after fitting, event selection may change, so need to refit it
    // 	 r.Rewind();
    // 	 r.HistClear();
    // 	//	 r.ReadAsymTable(asymFile.c_str());
    // 	 r.SetFillMuHist();
    // 	 r.SetAsymCorrSlope();
    // 	 r.Go(1000000);

    // 	 r.FitMuHist();

	 std::cout << std::endl << __FILE__ << "(" << __LINE__ << ")" << " Writing mu peaks...\n";
    r.WriteMuPeaks(peakFile.c_str());
	 r.WriteMuPeaksXML(peakFileXML.c_str());

    std::cout << std::endl << __FILE__ << "(" << __LINE__ << ")" << " Writing histograms...\n";
    r.WriteHist();

  }

  return 0;
}
