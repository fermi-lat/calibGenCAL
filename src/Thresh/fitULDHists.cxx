// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/Thresh/fitULDHists.cxx,v 1.2 2008/04/22 19:02:47 fewtrell Exp $

/** @file
    @author Zachary Fewtrell

    Fit ULD threshold histograms for each ADC range.
*/

// LOCAL INCLUDES
#include "src/lib/Util/CfgMgr.h"
#include "src/lib/Util/string_util.h"
#include "src/lib/Util/CGCUtil.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"

// EXTLIB INCLUDES
#include "TFile.h"
#include "TNtuple.h"
#include "TH1S.h"
#include "TF1.h"


// STD INCLUDES
#include <string>
#include <fstream>
#include <sstream>
#include <cfloat>
#include <cmath>

using namespace std;
using namespace CfgMgr;
using namespace calibGenCAL;
using namespace CalUtil;

/// Manage application configuration parameters
class AppCfg {
public:
  AppCfg(const int argc,
         const char **argv) :
    cmdParser(path_remove_ext(__FILE__)),
    histFilePath("histFilePath",
                 "ROOT histograms for each ULD threshold.",
                 ""
                 ),
    outputBasename("outputBasename",
                   "all output files will use this basename + some_ext",
                   ""),
    help("help",
         'h',
         "print usage info")
  {
    cmdParser.registerArg(histFilePath);
    cmdParser.registerArg(outputBasename);
    cmdParser.registerSwitch(help);

    try {
      cmdParser.parseCmdLine(argc, argv);
    } catch (exception &e) {
      // ignore invalid commandline if user asked for help.
      if (!help.getVal())
        cout << e.what() << endl;
      cmdParser.printUsage();
      exit(-1);
    }


  }

  /// construct new parser
  CmdLineParser cmdParser;

  CmdArg<string> histFilePath;

  CmdArg<string> outputBasename;

  /// print usage string
  CmdSwitch help;

};

/// return bin center for last non zero bin.
float findLastNonZeroBin(TH1S &h) {
  const unsigned nBins = h.GetNbinsX();

  int currentBin = nBins-1;
  unsigned lastNZBin = nBins - 1;

  while (currentBin >= 0) {
    lastNZBin = currentBin;
    if (h.GetBinContent(currentBin) > 0)
      break;
    currentBin--;
  }

  return h.GetBinCenter(currentBin);
}

int main(const int argc, const char **argv) {
  // libCalibGenCAL will throw runtime_error
  try {
    AppCfg cfg(argc,argv);

    //-- SETUP LOG FILE --//
    /// multiplexing output streams
    /// simultaneously to cout and to logfile
    LogStrm::addStream(cout);

    // generate logfile name
    const string logfile(cfg.outputBasename.getVal() + ".uld_fit.log.txt");
    ofstream tmpStrm(logfile.c_str());
    LogStrm::addStream(tmpStrm);

    string outputTXTPath(cfg.outputBasename.getVal() + ".uld_fit.txt");
  
    LogStrm::get() << __FILE__ << ": Opening output TXT file: " << outputTXTPath << endl;
    ofstream outfile(outputTXTPath.c_str());
    /// output column headers
    outfile << ";twr lyr col face rng uld erruld" << endl;

    // open output files
    LogStrm::get() << __FILE__ << ": Opening ROOT file for update: " << cfg.histFilePath.getVal() << endl;
    TFile fhist(cfg.histFilePath.getVal().c_str(),"UPDATE");
    TNtuple* ntp = 
      new TNtuple("uld_ntp","uld_ntp",
                  "twr:lyr:col:face:rng:uld:erruld:bkg0:bkg1:chi2:nent:fitstat");

    TH1S* hadc;
  
    // loop through each channel
    /// output column headers
    LogStrm::get() << "twr lyr col face rng uld erruld fitstat chi2 initial_uld_thresh" << endl;
    for (RngIdx rngIdx; rngIdx.isValid(); rngIdx++) {
      // no ULD for HEX1
      if (rngIdx.getRng() == HEX1)
        continue;
      
      ostringstream hadcname;
      hadcname << "uld_" << rngIdx.toStr();
      hadc = (TH1S*)fhist.Get(hadcname.str().c_str());

      /// skip missing channels
      if (!hadc)
        continue;

      ostringstream uldfitname;
      uldfitname << "uldfit_" << rngIdx.toStr();

      const float initial_uld_thresh = findLastNonZeroBin(*hadc);

      // fit params
      // [0] = uld threshold level
      // [1] = uld sharpness (small val is sharper)
      // [2] = bkg spectrum slope
      // [3] = bkg spectrum constant
      TF1* fun = new TF1(uldfitname.str().c_str(),"([2]*(x-3595)+[3])/(1+exp((x-[0])/[1]))",3095,4095);
      fun->SetNpx(500);
      // INIT/LIMIT FIT PARMS
      fun->SetParameters(initial_uld_thresh, 3, 0, 1);

      /// limit ULD thresh to real ADC values
      fun->SetParName(0, "uld threshold (adc)");
      fun->SetParLimits(0, 3095, 4096);
      /// sharpness is positive value (should be very small)
      fun->SetParName(1, "threshold sharpness");
      fun->SetParLimits(1, 3, 3);
      /// bkg slope
      fun->SetParName(2, "bkg slope");
      /// limit background constant to positive values (scale to size of histogram)
      fun->SetParName(3, "bkg constant");
      fun->SetParLimits(3, 0, hadc->GetEntries());
      
      const float fitstat = hadc->Fit(uldfitname.str().c_str(),"RLQ");
      
      const float uld = fun->GetParameter(0);
      const float erruld = fun->GetParError(0);
      const float chi2 = fun->GetChisquare();
      const float nent = hadc->GetEntries();
      const float bkg1 = fun->GetParameter(2);
      const float bkg0 = fun->GetParameter(3);

      const unsigned short twr = rngIdx.getTwr().val();
      const unsigned short lyr = rngIdx.getLyr().val();
      const unsigned short col = rngIdx.getCol().val();
      const unsigned short face = rngIdx.getFace().val();
      const unsigned short rng = rngIdx.getRng().val();
      
      LogStrm::get() << twr << " " << lyr << " " << col << " " << face << " " << rng << " "
                     << uld << " " << erruld << " " 
                     << fitstat << " " << chi2 << " " << initial_uld_thresh << " "
                     << endl;
      outfile << twr << " " << lyr << " " << col << " " << face << " " << rng << " " << uld << " " << erruld << endl;

      ntp->Fill(twr,lyr,col,face,rng,
                uld, erruld, bkg0, bkg1,
                chi2, nent, fitstat);

    }

  
    fhist.Write();
    fhist.Close();

  } catch (exception &e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
    return -1;
  }

  return 0;
}
