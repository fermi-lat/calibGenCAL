// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/Thresh/fitLACHists.cxx,v 1.4 2008/05/14 18:39:46 fewtrell Exp $

/** @file
    @author Zachary Fewtrell

    Fit LAC threshold histograms for each crystal face from.
*/

// LOCAL INCLUDES
#include "src/lib/Util/CfgMgr.h"
#include "src/lib/Util/string_util.h"
#include "src/lib/Util/CGCUtil.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/SimpleCalCalib/ADC2NRG.h"

// EXTLIB INCLUDES
#include "TFile.h"
#include "TNtuple.h"
#include "TH1I.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TF1.h"


// STD INCLUDES
#include <string>
#include <fstream>
#include <sstream>

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
                 "ROOT histograms for each LAC threshold.",
                 ""
                 ),
    adc2nrgFilename("muSlopeFilename",
                    "text file with muSlope (adc2mev) calibration data",
                    ""),
    outputBasename("outputBasename",
                   "all output files will use this basename + some_ext",
                   ""),
    help("help",
         'h',
         "print usage info")
  {
    cmdParser.registerArg(histFilePath);
    cmdParser.registerArg(adc2nrgFilename);
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

  CmdArg<string> adc2nrgFilename;

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
    const string logfile(cfg.outputBasename.getVal() + ".lac_fit.log.txt");
    ofstream tmpStrm(logfile.c_str());
    LogStrm::addStream(tmpStrm);

    string outputTXTPath(cfg.outputBasename.getVal() + ".lac_fit.txt");
  
    LogStrm::get() << __FILE__ << ": opening output TXT file: " << outputTXTPath << endl;
    ofstream outfile(outputTXTPath.c_str());
    /// print column headers
    outfile << ";twr lyr col face lac errlac pedDrift lacMeV errlacMeV" << endl;
    LogStrm::get() << ";twr lyr col face lac errlac pedDrift lacMeV fitstat chi2 mev_slope mev_offset" << endl;

    // open input files
    ADC2NRG adc2nrg;
    LogStrm::get() << __FILE__ << ": calib file: " << cfg.adc2nrgFilename.getVal() << endl;
    adc2nrg.readTXT(cfg.adc2nrgFilename.getVal());

    // open output files
    LogStrm::get() << __FILE__ << ": opening output histogram file: " << cfg.histFilePath.getVal() << endl;
    TFile fhist(cfg.histFilePath.getVal().c_str(),"UPDATE");
    TNtuple* ntp = 
      new TNtuple("lacadcntp","lacadcntp",
                  "twr:lyr:col:face:lac:errlac:pedDrift:lacMeV:errlacMeV:bkg0:bkg_steepness:chi2:nent:fitstat");

    TCanvas*  canv = new TCanvas("canv","canv",100,100,600,600);
    int ipad=0;
    int npad=4;
    canv->Divide(2,2);


    TH1I* hadc;
    TH1I* hped;

    float pedDrift=0;
  
    for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++)
      for (FaceNum face; face.isValid(); face++) {
        const FaceIdx faceIdx(xtalIdx, face);

        ostringstream hadcname;
        hadcname << "hadc_" << faceIdx.toStr();
        hadc = (TH1I*)fhist.Get(hadcname.str().c_str());
        /// skip missing channels
        if (!hadc)
          continue;

        ostringstream hpedname;
        hpedname << "hped_" << faceIdx.toStr();
        hped = (TH1I*)fhist.Get(hpedname.str().c_str());

        ostringstream lacfitname;
        lacfitname << "lacfit_" << faceIdx.toStr();

        ostringstream hrbname;
        hrbname << "hrbadc_" << faceIdx.toStr();
                        
        ostringstream lacrbfitname;
        lacrbfitname << "lacrbfit_" << faceIdx.toStr();

        canv->cd(1+(ipad%npad));
        ipad++;
                                
        canv->cd(1+(ipad%npad));
        ipad++;

        hped->Fit("gaus","Q");
        pedDrift = hped->GetFunction("gaus")->GetParameter(1);

        //-- PHASE 1: FIND THRESHOLD IN REBINNED HISTOGRAM --//
        TF1* funrb = new TF1(lacrbfitname.str().c_str(),
                             "([2]/x+[3])/(1+exp(([0]-x)/[1]))", -50, 300);
        funrb->SetNpx(500);
        funrb->SetParName(0, "lac threhsold");
        funrb->SetParName(1, "threshold width");
        funrb->SetParName(2, "bkg steepness");
        funrb->SetParName(3, "bkg constant");

        TH1I* h = hadc;
        canv->cd(1+(ipad%npad));
        ipad++;

        // original binning was 5 adc units, rebinned should be 20 adc units per bin.
        TH1I* hrebin = (TH1I*)h->Rebin(4,hrbname.str().c_str());
        hrebin->SetTitle(hrbname.str().c_str());
        
        /// LAC threshold is usually near the highest bin
        const int mbin = hrebin->GetMaximumBin();
        float lac_thresh = hrebin->GetBinCenter(mbin);
                        
        /// bkg constant will usually be close to level of last bin
        const unsigned short lastBinRB = hrebin->GetNbinsX()-1;
        float bkg_constant = hrebin->GetBinContent(lastBinRB);
        // bin 10 should be 200 ADC, or about 6.5 MeV, 
        float bkg_steepness = ((hrebin->GetBinContent(10))-bkg_constant)/(1.0/10 - 1.0/lastBinRB);

        funrb->SetParameters(lac_thresh,
                             1.0,
                             bkg_steepness,
                             bkg_constant);
        // lac threshold (should be near rebinned threshold (30 adc = 1mev))
        funrb->SetParLimits(0, lac_thresh-30, lac_thresh+30);

        // thresh width
        funrb->FixParameter(1, 1.0);

        // background steepness
        funrb->SetParLimits(2,   0, hrebin->GetEntries()*300);
        
        // bkg constant
        funrb->SetParLimits(3, 0, hrebin->GetEntries());

        hrebin->Fit(lacrbfitname.str().c_str(),"RLQ");
        // since we rebineed by factor 4, constants will be four times smaller under default binning.
        bkg_constant = funrb->GetParameter(3)*0.25;
        bkg_steepness = funrb->GetParameter(2)*0.25;

        canv->cd(1+(ipad%npad));
        ipad++;

        //-- PHASE 2: FIND THRESHOLD WITH NORMAL BINNING --//
        TF1* fun = new TF1(lacfitname.str().c_str(),
                           "([2]/x+[3])/(1+exp(([0]-x)/[1]))", -50, 300);
        fun->SetNpx(500);
        fun->SetParName(0,"lac threhsold");
        fun->SetParName(1,"threshold width");
        fun->SetParName(2,"bkg steepness");
        fun->SetParName(3,"bkg constant");

        fun->SetParameters(lac_thresh,
                           1.0,
                           bkg_steepness,
                           bkg_constant);

        // lac threshold
        fun->SetParLimits(0, -50, 300);

        // thresh width
        fun->FixParameter(1, 1.0);

        // background steepness
        fun->SetParLimits(2,   0, h->GetEntries()*300);
        
        // bkg constant
        fun->SetParLimits(3, 0, h->GetEntries());

        const float fitstat = h->Fit(lacfitname.str().c_str(),"RLQ");

        const float lac = fun->GetParameter(0);
        const float errlac = fun->GetParError(0);
        const float chi2 = fun->GetChisquare();
        const float nent = h->GetEntries();
        bkg_steepness = fun->GetParameter(2);
        bkg_constant = fun->GetParameter(3);

        const float lacMeV = (lac-pedDrift)*adc2nrg.getADC2NRG(RngIdx(faceIdx,LEX8));
        const float errlacMeV = errlac*lacMeV/(lac-pedDrift);
        const float mev_slope = adc2nrg.getADC2NRG(RngIdx(faceIdx, LEX8));
        const float mev_offset = 0;

        const unsigned short twr = faceIdx.getTwr().val();
        const unsigned short lyr = faceIdx.getLyr().val();
        const unsigned short col = faceIdx.getCol().val();

        LogStrm::get() << twr << " " << lyr << " " << col << " " << face << " "
                       << lac << " " << errlac << " " << pedDrift << " " 
                       << lacMeV << " " << fitstat << " " << chi2 << " " 
                       << mev_slope << " " << mev_offset << " " 
                       << endl;

        ntp->Fill(twr,lyr,col,face,lac,errlac,pedDrift,lacMeV,errlacMeV,bkg_constant,bkg_steepness,chi2,nent,fitstat);

        outfile << twr << " " << lyr << " " << col << " " << face << " " << lacMeV << " " << errlacMeV << endl;

        canv->Update();
      }

  
    LogStrm::get() << __FILE__ << "Writing output ROOT file." << endl;
    fhist.Write();
    fhist.Close();

    LogStrm::get() << __FILE__ << "Successfully completed." << endl;
  } catch (exception &e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
    return -1;
  }

  return 0;
}
