// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/Thresh/fitTrigHists.cxx,v 1.2 2008/05/09 21:51:37 fewtrell Exp $

/** @file
    @author Zachary Fewtrell

    Fit Trigger threshold histograms for each crystal face using
    histograms from genTrigMonitorHists.exe
*/

// LOCAL INCLUDES
#include "src/lib/Util/CfgMgr.h"
#include "src/lib/Hists/TrigHists.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"

// EXTLIB INCLUDES
#include "TFile.h"
#include "TH1S.h"
#include "TF1.h"


// STD INCLUDES
#include <string>
#include <fstream>

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
    TFile inputROOT(cfg.histFilePath.getVal().c_str(),"READ");

    /// output filenames
    const string outTxtPath(cfg.outputBasename.getVal() + ".trig_thresh.txt");
    const string outRootPath(cfg.outputBasename.getVal() + ".trig_thresh.root");

    /// open output ROOT file
    LogStrm::get() << __FILE__ << ": Openiing output ROOT file: " << outputRootPath << endl;
    TFile outRootFile(outRootPath.c_str(),"RECREATE");

    /// read in histograms from file
    const TrigHists fleHists("fleHist",
                       0, &histfile);
    const TrigHists fheHists("fheHist",
                       0, &histfile);

    /// create fitting stepction (straight line 'background' multiplied by a
    /// sigmoidal 'threshold')
    TF1* step = new TF1("trig_fit",
                       "([2]*x+[3])/(1+exp(([0]-x)/[1]))");
    step->SetNpx(500);
    
    /// enumerate fit paramters.
    typedef enum {
      NPARM_THOLD,
      NPARM_WIDTH,
      NPARM_BKG,
      NPARM_SLOPE
    } TRIGHIST_PARMS;

    step->SetParName(NPARM_THOLD, "threshold");
    step->SetParName(NPARM_WIDTH, "threshold width");
    step->SetParName(NPARM_BKG,   "bkg slope");
    step->SetParName(NPARM_SLOPE, "bkg constant");


    /// print column headers
    LogStrm::get() << ";twr lyr col face diode threshMeV errThreshMeV width chi2 nEntries fitstat" << endl;
    for (DiodeIdx diodeIdx; diodeIdx.isValid(); diodeIdx++) {
      const DiodeNum diode = diodeIdx.getDiode();

      /// select histogram collection for current diode
      const TrigHists &trigHists = (diode == LRG_DIODE) ? fleHists : fheHists;
      
      /// retrieve histogram from collection
      const FaceIdx = diodeIdx.getFaceIdx();
      TH1S const*const trigHist = trigHists.getHist(faceIdx);
      /// we don't require every channel to be present
      if (!trigHist)
        continue;
      
      /// setup fitting parameters.
      const float maxEne = trigHist.GetXaxis()->GetXmax();
      const unsigned nBins = trigHist.GetNbinsX();
      const double maxHeight = trigHist.GetMaximum();
      /// threshold must be on x-axis, start @ middle of hist
      step->SetParLimits(NPARM_THOLD,0, maxEne);
      step->SetParameter(NPARM_THOLD, maxEne/2);
      /// threshold width should be roughly one bin.
      step->FixParameter(NPARM_WIDTH, nBins/maxEne);
      /// 'slope' is flat or negative (start flat)
      step->SetParLimits(NPARM_SLOPE, -1*maxHeight, 0);
      step->SetParameter(NPARM_SLOPE, 0);
      /// 'bkg' is positive (start @ 0)
      step->SetParLimits(NPARM_BKG, 0, maxHeight);
      step->SetParameter(NPARM_BKG, 0);

      /// fit histogram
      const unsigned fitStat = trigHist.Fit(&step,"QLB","");
      /// get fit results
      const float threshMeV = step->GetParameter(0);
      const float threshErrMeV = step->GetParError(0);
      const float chisq = step->GetChisquare();
      const unsigned nEntries = effHist.GetEntries();
      const float width = step->GetParameter(1);

      /// output results
      LogStrm::get() << diodeIdx.getTwr().val()
                     << " " << diodeIdx.getLyr.val()
                     << " " << diodeIdx.getCol.val()
                     << " " << diodeIdx.getFace.val()
                     << " " << diodeIdx.getDiode.val()
                     << " " << threshMeV
                     << " " << errThreshMeV
                     << " " << width
                     << " " << chi2
                     << " " << nEntries
                     << " " << fitstat
                     << endl;
    }

  } catch (exception &e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
    return -1;
  }

  return 0;
}
