// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/genMuonPed.cxx,v 1.25 2008/01/22 19:40:58 fewtrell Exp $

/** @file
    @author Zachary Fewtrell

    Fit Trig threshold histograms for each crystal face from.
*/

// LOCAL INCLUDES
#include "src/lib/Util/CfgMgr.h"
#include "src/lib/Util/string_util.h"
#include "src/lib/Util/CGCUtil.h"
#include "src/lib/Hists/TrigHists.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"

// EXTLIB INCLUDES
#include "TFile.h"
#include "TNtuple.h"
#include "TH1S.h"
#include "TF1.h"
#include "TGraphErrors.h"
#include "TROOT.h"
#include "TCanvas.h"


// STD INCLUDES
#include <string>
#include <fstream>
#include <algorithm>
#include <memory>
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
                 "ROOT trigger threshold histograms (output from genTrigHists)",
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

/// represent results of fitting a single threshold channel
class FitResults {
public:
  FitResults() :
    threshMeV(0),
    threshErrMeV(0),
    fitStat(0),
    chisq(0),
    nEntries(0),
    width(0)
  {}

  float threshMeV;
  float threshErrMeV;
  /// ROOT TF1::Fit() return code
  float fitStat;
  float chisq;
  float nEntries;
  float width;
};

/// fit trigger threshold given vectors of energy (X-axis) & trigger efficiency (Y-axis)
/// \param mev vector of x-axis energy values
/// \param eff trigger efficiency each energy value
/// \param effErr errors on each efficiency value
/// \param effHist histogram of efficiency vs energy
FitResults fitChannel(const FaceIdx faceIdx,
                      vector<float> &mev,
                      vector<float> &eff,
                      vector<float> &effErr,
                      TH1S &effHist) {
  // find threshold center point, where efficiency > 0.5
  float mevThresh=0;
  for (unsigned i = 0; i < eff.size(); i++)
    if (eff[i] > 0.5) {
      mevThresh = mev[i];
      break;
    }  

  // copy data into arrays for passing to ROOT
  const unsigned nBins = effHist.GetNbinsX();
  float * const mev_arr = new float[nBins];
  float * const mevErr_arr = new float[nBins];
  float * const eff_arr = new float[nBins];
  float * const effErr_arr = new float[nBins];

  const unsigned short nPts = mev.size();
  copy(mev.begin(), mev.end(), mev_arr);
  /// fill mevErr_arr with equal value
  fill(mevErr_arr, 
       mevErr_arr+sizeof(mevErr_arr)/sizeof(*mevErr_arr),
       0);
  copy(eff.begin(), eff.end(), eff_arr);
  copy(effErr.begin(), effErr.end(), effErr_arr);
  
  const string name = string("fit_trig_eff") + faceIdx.toStr();
  TGraphErrors geffs(nPts,
                     mev_arr, 
                     eff_arr, 
                     mevErr_arr, 
                     effErr_arr);
  geffs.SetNameTitle(name.c_str(), name.c_str());

  TCanvas c(name.c_str(), name.c_str(), -1);

  const float maxEne = effHist.GetXaxis()->GetXmax();
  TF1 step("step", "1.0/(1.0+exp(-[1]*(x-[0])))",0,maxEne);
  step.SetNpx(500);
  step.SetParName(0, "threshold MeV");
  step.SetParLimits(0, 0, maxEne);
  step.SetParName(1, "threshold sharpness");
  step.FixParameter(1, nBins/maxEne); /// set steepness to about 1 bin width
  step.SetParameters(mevThresh, nBins/maxEne);


  FitResults fr;
  fr.fitStat = geffs.Fit(&step,"QLB","");
  fr.threshMeV = step.GetParameter(0);
  fr.threshErrMeV = step.GetParError(0);
  fr.chisq = step.GetChisquare();
  fr.nEntries = effHist.GetEntries();
  fr.width = step.GetParameter(1);

  effHist.SetMaximum(20);
  effHist.Draw();
  geffs.Draw("SAME");

  c.Write();

  delete [] mev_arr;
  delete [] mevErr_arr;
  delete [] eff_arr;
  delete [] effErr_arr;

  return fr;
}

/// fit trigger threshold given histograms of total hits and of triggered hits
FitResults fitHists(const FaceIdx faceIdx,
                    TH1S &trigHist,
                    TH1S &specHist) {
  /// retreive bin Data from histograms
  short const * const specHistData = specHist.GetArray();
  short const * const trigHistData = trigHist.GetArray();

  /// actual data points (skip overflow bins in histograms
  /// mev @ bin center
  vector<float> mevCtr;
  /// percent total hits triggered
  vector<float> eff;
  /// error of efficiency
  vector<float> effErr;

  for (unsigned short nBin = 0;
       nBin < trigHist.GetNbinsX();
       nBin++) {
    const float totalHits = specHistData[nBin+1];
    // ignore empty bins
    if (totalHits <=0)
      continue;

    const float trig = trigHistData[nBin+1];
    const float noTrig = totalHits - trig;

    eff.push_back(trig / totalHits);
    const float mev = specHist.GetBinCenter(nBin+1); 
    mevCtr.push_back(mev);

    // used in efferr calc, must be >= 1.0
    const float strig = max<float>(1.0,trig);
    // used in efferr  calc, must be >= 1.0
    const float snotrig = max<float>(1.0,noTrig);

    const float err = sqrt(strig*noTrig*noTrig + snotrig*trig*trig)/(totalHits*totalHits); 
    effErr.push_back(err);
  }

  // create effHist (not used for fitting, but good for plotting)
  trigHist.Divide(&specHist);
  
  return fitChannel(faceIdx, mevCtr, eff, effErr, trigHist);
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
    const string logfile(cfg.outputBasename.getVal() + ".trig_thresh.log.txt");
    ofstream tmpStrm(logfile.c_str());
    LogStrm::addStream(tmpStrm);

    /// output filenames
    string outTxtPath(cfg.outputBasename.getVal() + ".trig_thresh.txt");
    string outRootPath(cfg.outputBasename.getVal() + ".trig_thresh.root");

    // open input file for read
    TFile inputROOT(cfg.histFilePath.getVal().c_str(),"READ");

    // read in input histograms
    TrigHists trigHists;
    trigHists.loadHists(inputROOT);

    /// open output TXT file
    ofstream outfileTXT(outTxtPath.c_str());

    /// open output ROOT file
    TFile outRootFile(outRootPath.c_str(),"RECREATE");

    TNtuple* ntp = 
      new TNtuple("trig_ntp","trig_ntp",
                  "twr:lyr:col:face:threshMeV:errThreshMeV:chi2:fitstat:nent:width");


    /// print column headers
    LogStrm::get() << ";twr lyr col face threshMeV errThreshMeV width chi2 nEntries fitstat" << endl;
    outfileTXT << ";twr lyr col face threshMeV errthresMeV" << endl;

    for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++) {
      TH1S * trigHist = trigHists.getTrigHist(faceIdx);
      /// we don't require every channel to be present
      if (!trigHist)
        continue;

      TH1S * specHist = trigHists.getSpecHist(faceIdx);
      /// we _do_ require every trigger hist to have a matching spectrum hist
      if (!specHist) {
        LogStrm::get() << "ERROR: missing spectrum hist: " << faceIdx.toStr()
                       << "from file " << cfg.histFilePath.getVal() << endl;
        return -1;
      }
      
      FitResults fr = fitHists(faceIdx, *trigHist, *specHist);

      const float twr = faceIdx.getTwr().val();
      const float lyr = faceIdx.getLyr().val();
      const float col = faceIdx.getCol().val();
      const float face = faceIdx.getFace().val();

      LogStrm::get() << twr << " " << lyr << " " << col << " " << face << " "
                     << fr.threshMeV << " " 
                     << fr.threshErrMeV << " "
                     << fr.width << " "
                     << fr.chisq     << " "
                     << fr.nEntries  << " "
                     << fr.fitStat   
                     << endl;

      outfileTXT << twr << " " << lyr << " " << col << " " << face 
                 << " " << fr.threshMeV 
                 << " " << fr.threshErrMeV << endl;

      ntp->Fill(twr,
                lyr,
                col,
                face,
                fr.threshMeV,
                fr.threshErrMeV,
                fr.chisq,
                fr.fitStat,
                fr.nEntries,
                fr.width);
    }

  
    outRootFile.Write();
    outRootFile.Close();

  } catch (exception &e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
    return -1;
  }

  return 0;
}
