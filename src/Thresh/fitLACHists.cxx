// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/Thresh/fitLACHists.cxx,v 1.7 2008/05/23 06:38:59 sanchez Exp $

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

/// percent of max histogram hieght required for first significant bin
static const float FIRSTBIN_FRAC_MAX = 0.15;

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

    float pedSigma=0;
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
        pedSigma = hped->GetFunction("gaus")->GetParameter(2);
        pedDrift = hped->GetFunction("gaus")->GetParameter(1);

        //-- PHASE 1: FIND THRESHOLD IN REBINNED HISTOGRAM --//
        //-- get 'fist pass' estimates @ fitting parms
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

        //-- PHASE 2: FIND precise THRESHOLD WITH NORMAL BINNING --//

        //Decalration of variable
        float fitstat, lac,errlac,chi2,nent;

        // Look for the first bin w/ significant height (15% of max) in LEX8 histogram
        float FirstBin=0;
        int ibin=0;

        const float maxHeight = h->GetMaximum();
        const float maxBinCtr = h->GetBinCenter(h->GetMaximumBin());
        while (ibin < h->GetEntries()){
          if (h->GetBinContent(ibin) > FIRSTBIN_FRAC_MAX*maxHeight) {
            FirstBin = h->GetBinWidth(ibin)*(ibin-1.);
            break;
          }
          ibin++;
        }

        //CASE 1
        // firstbin is > pedDrift+2*pedSigma
        if (FirstBin > pedDrift+2*pedSigma) { 
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
          fun->SetParLimits(0, FirstBin-10, 300);

          // thresh width
          fun->FixParameter(1, 1.0);

          // background steepness
          fun->SetParLimits(2,   0, maxHeight*maxBinCtr);
        
          // bkg constant
          fun->SetParLimits(3, 0, maxHeight);

          fitstat = h->Fit(lacfitname.str().c_str(),"RLQ");

          lac = fun->GetParameter(0);
          errlac = fun->GetParError(0);
          chi2 = fun->GetChisquare();
          nent = h->GetEntries();
          bkg_steepness = fun->GetParameter(2);
          bkg_constant = fun->GetParameter(3);
        }


        // CASE 2
        // The LAC value is between pedDrift and pedDrift+2.*pedSigma
        // the new fun is (fsigna+gauss)*feff

        else if (FirstBin < pedDrift+2.*pedSigma && FirstBin > pedDrift ) {
          float Rmax = pedDrift+4.*pedSigma;
          TF1* fun = new TF1(lacfitname.str().c_str(),
                             "(gaus+([5]/x+[6]))/(1+exp(([3]-x)/[4]))", 0, 300);
          fun->SetNpx(500);
          fun->SetParName(3,"lac threhsold");
          fun->SetParName(4,"threshold width");
          fun->SetParName(5,"bkg steepness");
          fun->SetParName(6,"bkg constant");

          fun->SetParameters(1.0,
                             1.0,
                             1.0,
                             Rmax,
                             1.0,
                             bkg_steepness,
                             bkg_constant);

          //Fix the parameter og the gaussian equal to the parameter of the pedestal
          fun->FixParameter(1, pedDrift);
          fun->FixParameter(2, pedSigma);

          // lac threshold
          fun->SetParLimits(3, FirstBin*0.8, FirstBin*2);

          // thresh width
          fun->FixParameter(4, 1.0);

          // background steepness
          fun->SetParLimits(5,   0, maxHeight*maxBinCtr);
        
          // bkg constant
          fun->SetParLimits(6, 0, maxHeight);

          fitstat = h->Fit(lacfitname.str().c_str(),"RLQ");

          lac = fun->GetParameter(3);

          errlac = fun->GetParError(3);
          chi2 = fun->GetChisquare();
          nent = h->GetEntries();
          bkg_steepness = fun->GetParameter(5);
          bkg_constant = fun->GetParameter(6);

        }

        // CASE 3
        // LAC value is below pedDrift
        // new fun is gauss*feff
        
        else if (FirstBin < pedDrift) {
          float Rmax = pedDrift+3.*pedSigma;
          TF1* fun = new TF1(lacfitname.str().c_str(),
                             "gaus/(1+exp(([3]-x)/[4]))", FirstBin, Rmax);

          fun->SetNpx(500);
          fun->SetParName(3,"lac threhsold");
          fun->SetParName(4,"threshold width");

          fun->SetParameters(1.0,
                             1.0,
                             1.0,
                             Rmax,
                             1.0);

          //Fix the parameter og the gaussian equal to the parameter of the pedestal
          fun->FixParameter(1, pedDrift);
          fun->FixParameter(2, pedSigma);

          // lac threshold
          fun->SetParLimits(3, FirstBin, Rmax);

          // thresh width
          fun->FixParameter(4, 1.0);

          fitstat = h->Fit(lacfitname.str().c_str(),"RLQ","",0,Rmax);

          lac = fun->GetParameter(3);
          errlac = fun->GetParError(3);
          chi2 = fun->GetChisquare();
          nent = h->GetEntries();
          bkg_steepness = 0.;
          bkg_constant = 0.;
        } 

        else {
          throw std::runtime_error("Invalid LAC fit condition.");
        }


        // END DAVID SANCHEZ'S PATCH
        ///////////////////////////////////////////////////////////////////////////////////

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

  
    LogStrm::get() << __FILE__ << ": Writing output ROOT file." << endl;
    fhist.Write();
    fhist.Close();

    LogStrm::get() << __FILE__ << ": Successfully completed." << endl;
  } catch (exception &e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
    return -1;
  }

  return 0;
}
