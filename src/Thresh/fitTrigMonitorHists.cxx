// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/Thresh/fitTrigMonitorHists.cxx,v 1.7 2008/07/23 20:31:03 fewtrell Exp $

/** @file
    @author Zachary Fewtrell

    Fit Trigger threshold histograms for each crystal face using
    histograms from genTrigMonitorHists.exe
*/

// LOCAL INCLUDES
#include "src/lib/Util/CfgMgr.h"
#include "src/lib/Hists/TrigHists.h"
#include "src/lib/Util/CGCUtil.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"

// EXTLIB INCLUDES
#include "TFile.h"
#include "TH1S.h"
#include "TF1.h"
#include "TNtuple.h"

// STD INCLUDES
#include <string>
#include <fstream>
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


/// fit background spectrum for given channel, only fit portion above maxbin
/// \param h histogram to fit
/// \param height return height of spectrum.
/// \param power return spectral power index
/// \param constant return background constant level
void fitSpectrum(TH1S &h, float &height, float &power) {
  typedef enum {
    NPARM_SPEC_HEIGHT,
    NPARM_SPEC_POWER
  } SPECHIST_PARMS;

  TF1 spec("spec_fit", "([0]*x**[1])");
  spec.SetParName(NPARM_SPEC_HEIGHT,   "spectrum height");
  spec.SetParName(NPARM_SPEC_POWER, "spectral index");
  
  const unsigned maxBin = h.GetMaximumBin();
  const float maxBinCenter = h.GetBinCenter(maxBin);
  const float maxEne = h.GetXaxis()->GetXmax();
  const float maxHeight = h.GetMaximum();

  /// spectrum power idx usually -2
  static const float maxPowerIdx = 0;
  static const float minPowerIdx = -4;
  static const float initPowerIdx = -2;

  /// spectrum height limited by height of histogram
  spec.SetParLimits(NPARM_SPEC_HEIGHT, 0, 2.0*maxHeight*pow(maxBinCenter, -1*minPowerIdx));
  spec.SetParameter(NPARM_SPEC_HEIGHT, maxHeight/pow(maxBinCenter,initPowerIdx));

  spec.SetParLimits(NPARM_SPEC_POWER, minPowerIdx, maxPowerIdx);
  spec.SetParameter(NPARM_SPEC_POWER, initPowerIdx);

  h.Fit(&spec,"QLB","",maxBinCenter, maxEne);

  height = spec.GetParameter(NPARM_SPEC_HEIGHT);
  power = spec.GetParameter(NPARM_SPEC_POWER);
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

    // open input file for read
    LogStrm::get() << __FILE__ << ": Opening input ROOT file " << cfg.histFilePath.getVal() << endl;
    TFile inROOTFile(cfg.histFilePath.getVal().c_str(),"READ");
    /// open output ROOT file
    /// output filenames
    const string outTxtPath(cfg.outputBasename.getVal() + ".trig_thresh.txt");
    const string outRootPath(cfg.outputBasename.getVal() + ".trig_thresh.root");
    LogStrm::get() << __FILE__ << ": Opening output ROOT file: " << outRootPath << endl;
    TFile outROOTFile(outRootPath.c_str(),"RECREATE");

    /// read in histograms from file
    TrigHists fleHists("fleHist",
                       &outROOTFile, &inROOTFile);
    TrigHists fheHists("fheHist",
                       &outROOTFile, &inROOTFile);

    /// create fitting stepction (straight line 'background' multiplied by a
    /// sigmoidal 'threshold')
    TF1* step = new TF1("trig_fit","([2]*x**[3])*([4]+(1-[4])/(1+exp(([0]-x)/[1])))");
    step->SetNpx(500);
    
    /// enumerate fit paramters.
    typedef enum {
      NPARM_THOLD,
      NPARM_WIDTH,
      NPARM_SPEC_HEIGHT,
      NPARM_SPEC_POWER,
      NPARM_BKG_PCT
    } TRIGHIST_PARMS;

    step->SetParName(NPARM_THOLD, "threshold");
    step->SetParName(NPARM_WIDTH, "threshold width");
    step->SetParName(NPARM_SPEC_HEIGHT,   "spectrum height");
    step->SetParName(NPARM_SPEC_POWER, "spectral index");
    step->SetParName(NPARM_BKG_PCT, "trigger efficiency below threshold");

    TNtuple* ntp = 
      new TNtuple("trig_fit_ntp","trig_fit_ntp",
                  "twr:lyr:col:face:diode:thresh:err:width:spec_height:spec_power:bkg:chisq:nEntries:fitstat");

    /// print column headers
    LogStrm::get() << ";twr lyr col face diode threshMeV errThreshMeV width spec_height spec_power bkg chi2 nEntries fitstat" << endl;
    for (DiodeIdx diodeIdx; diodeIdx.isValid(); diodeIdx++) {
      const DiodeNum diode = diodeIdx.getDiode();

      /// select histogram collection for current diode
      TrigHists &trigHists = (diode == LRG_DIODE) ? fleHists : fheHists;
      
      /// retrieve histogram from collection
      const FaceIdx faceIdx = diodeIdx.getFaceIdx();
      TH1S *const trigHist = trigHists.getHist(faceIdx);
      /// we don't require every channel to be present
      if (!trigHist)
        continue;

      /// find background spectrum
      float spec_height, spec_power;
      fitSpectrum(*trigHist, spec_height, spec_power);
      
      /// setup fitting parameters.
      const float maxEne = trigHist->GetXaxis()->GetXmax();
      const unsigned nBins = trigHist->GetNbinsX();
      const unsigned maxBin = trigHist->GetMaximumBin();
      const float maxBinCenter = trigHist->GetBinCenter(maxBin);

      /// threshold must be on x-axis, start @ middle of hist
      step->SetParLimits(NPARM_THOLD, maxBinCenter*.75, std::min<float>(maxBinCenter*1.25,maxEne));
      step->SetParameter(NPARM_THOLD, maxBinCenter);

      /// threshold width should be roughly one bin.
      step->FixParameter(NPARM_WIDTH, maxEne/nBins);

      /// background spectra now defined.
      step->FixParameter(NPARM_SPEC_HEIGHT, spec_height);
      step->FixParameter(NPARM_SPEC_POWER, spec_power);

      step->SetParLimits(NPARM_BKG_PCT,0,.5);
      step->SetParameter(NPARM_BKG_PCT,0);

      /// fit histogram
      unsigned fitstat = 0;

      fitstat = trigHist->Fit(step,
                              "QLB",
                              "",
                              maxBinCenter/2,
                              maxEne); // start fitting @ 50% of threshold (background is usually flat above this point)

      /// get fit results
      const float threshMeV = step->GetParameter(NPARM_THOLD);
      const float threshErrMeV = step->GetParError(NPARM_THOLD);
      const float bkg = step->GetParameter(NPARM_BKG_PCT);
      const float chisq = step->GetChisquare();
      const unsigned nEntries = (unsigned)trigHist->GetEntries();
      const float width = step->GetParameter(NPARM_WIDTH);

      /// output results
      LogStrm::get() << diodeIdx.getTwr().val()
                     << " " << diodeIdx.getLyr().val()
                     << " " << diodeIdx.getCol().val()
                     << " " << diodeIdx.getFace().val()
                     << " " << diodeIdx.getDiode().val()
                     << " " << threshMeV
                     << " " << threshErrMeV
                     << " " << width
                     << " " << spec_height
                     << " " << spec_power
                     << " " << bkg
                     << " " << chisq
                     << " " << nEntries
                     << " " << fitstat
                     << endl;

      ntp->Fill(diodeIdx.getTwr().val(),
                diodeIdx.getLyr().val(),
                diodeIdx.getCol().val(),
                diodeIdx.getFace().val(),
                diodeIdx.getDiode().val(),
                threshMeV,
                threshErrMeV,
                width,
                spec_height,
                spec_power,
                bkg,
                chisq,
                nEntries,
                fitstat
                );
    }

    LogStrm::get() << __FILE__ << ": Writing output ROOT file." << endl;
    outROOTFile.Write();
    outROOTFile.Close();

    LogStrm::get() << __FILE__ << ": Successfully completed." << endl;
  } catch (exception &e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
    return -1;
  }

  return 0;
}
