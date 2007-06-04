// EXTLIB INCLUDES
#include "TSystem.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TProfile.h"
#include "TTree.h"
#include "TNtuple.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TPostScript.h"
#include "TPaveText.h"
#include "TStyle.h"
#include "TPaveLabel.h"

// STD INCLUDES
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

int main(int argc, char** argv) {
// int emiAnal(char* run) {

    	float time_slice=3.0;

  string run;
  if(argc > 1) run = argv[1];
  string str_time_slice;
  if (argc > 2) str_time_slice = argv[2];
  string version="v3r9p3";
  if (argc > 3) version=argv[3];
  cout << " " << str_time_slice.c_str() << endl;
  sscanf(str_time_slice.c_str(),"%f",&time_slice);
  cout << " time_slice=" << time_slice << endl;


	ostringstream fcalstr;
	ostringstream frepstr;
	ostringstream fhiststr;
	frepstr << "EMI_report_" << run << ".ps";
	fhiststr << "EMI_analysis_hists_" << run << ".root";


	fcalstr  << "muonPeds.digitization-licos-" << version << "_"  << run << "_digi_DIGI.root";
	TFile* fcal = new TFile(fcalstr.str().c_str());

	
	float max_mean_dev=30;
    float max_rms = 30;
    float warn_rms = 8.0;
    int n_mean_fail = 0;
    int n_rms_fail = 0;

    //    	double time_slice=3.0;
	int n_time_slices = 200;


	TProfile* pr_ped_chan[16][8][12][2];

	TProfile* pr_ped_lat = (TProfile*)fcal->Get("prpedlat");

	int nbins = pr_ped_lat->GetNbinsX();

	for(int i = 0; i<nbins; i++){
		int nts = nbins-i-1;
		int n = pr_ped_lat->GetBinEntries(nts);
		if(n > 0){n_time_slices=nts-1; break;}
	}
	cout << " n_time_slice=" << n_time_slices << endl;
	int iPerTrig = pr_ped_lat->GetEntries();


	TProfile* pr_ped_twr[16];
	TH1F* hcallo = (TH1F*) fcal->Get("hcallo");
	TH1F* hcalhi = (TH1F*) fcal->Get("hcalhi");
	hcallo->GetXaxis()->SetRangeUser(0,n_time_slices*time_slice);
	hcalhi->GetXaxis()->SetRangeUser(0,n_time_slices*time_slice);

	for(int twr=0;twr<16;twr++){
	    ostringstream prpedtwrname;
	    prpedtwrname << "prpedtwr_T" << twr;
	    pr_ped_twr[twr]=(TProfile*)fcal->Get(prpedtwrname.str().c_str());
	}

	for(int twr=0;twr<16;twr++)
	  for(int lyr=0;lyr<8;lyr++)
	    for(int face=0;face<2;face++){
	      for(int col=0;col<12;col++){
		       ostringstream prpedname;
		       prpedname << "prped_T" << twr  << "_L" << lyr << "_C" << col << "_F" << face;
		       pr_ped_chan[twr][lyr][col][face]=(TProfile*) fcal->Get(prpedname.str().c_str());
	      }
	    }


	int nwarnrms[16][8][12][2];	
	for(int twr=0;twr<16;twr++)
		for(int lyr=0;lyr<8;lyr++)
			for(int col=0;col<12;col++)
				for(int face=0;face<2;face++){
					nwarnrms[twr][lyr][col][face]=0;
				}


TFile* newfile = new TFile(fhiststr.str().c_str(),"RECREATE");

TH1F* hmean = new TH1F("hmean","mean pedestals for all channels and time slices",500,-50,50);
TH1F* hrms = new TH1F("hrms","pedestal RMS for all channels and time slices",500,0,50);
TH1F* hmeanlat = new TH1F("hmeanlat","LAT mean pedestal for all time slices",100,-2,2);
TH1F* hrmslat = new TH1F("hrmslat","LAT pedestal RMS for all time slices",100,0,2);
TH1F* hmeantwr = new TH1F("hmeantwr","Tower mean pedestals for all towers and time slices",100,-2,2);
TH1F* hrmstwr = new TH1F("hrmstwr","Tower pedestal RMS for all towers and time slices",100,0,2);
TH1F* hmeanafee = new TH1F("hmeanafee","AFEE mean pedestals for all AFEE boards and time slices",100,-2,2);
TH1F* hrmsafee = new TH1F("hrmsafee","AFEE pedestal RMS for all AFEE boards and time slices",100,0,4);


for(int i=0;i<n_time_slices;i++){
     for(int twr=0;twr<16;twr++){
		for(int xy=0;xy<2;xy++)
		  for(int face=0;face<2;face++){
		    for(int row=0;row<4;row++)
			for(int col=0;col<12;col++)
			  {
			    int lyr=xy+row*2;
			    int nbinent = (pr_ped_chan[twr][lyr][col][face])->GetBinEntries(i+1);
			    if (nbinent < 1) continue;
			    float mean = (pr_ped_chan[twr][lyr][col][face])->GetBinContent(i+1);
			    float rms = ((pr_ped_chan[twr][lyr][col][face])->GetBinError(i+1))
						*sqrt(float(nbinent));
			    hmean->Fill(mean);
			    hrms->Fill(rms);
			    if(mean > max_mean_dev)n_mean_fail++;
			    if(rms > max_rms)n_rms_fail++;
			    if(rms > warn_rms)(nwarnrms[twr][lyr][col][face])++;
			  }
		    
		  }
	 float mean = (pr_ped_twr[twr])->GetBinContent(i+1);
	 float rms = ((pr_ped_twr[twr])->GetBinError(i+1))
	   *sqrt((pr_ped_twr[twr])->GetBinEntries(i+1));
	 hmeantwr->Fill(mean);
	 hrmstwr->Fill(rms);
		    
     }
	 float mean = pr_ped_lat->GetBinContent(i+1);
	 float rms = (pr_ped_lat->GetBinError(i+1))
	   *sqrt(pr_ped_lat->GetBinEntries(i+1));
	 hmeanlat->Fill(mean);
	 hrmslat->Fill(rms);
		
 }



	TCanvas canv_EMI("canv_EMI","EMI test report",100,10,600,700);
	const char* repchar = frepstr.str().c_str();
	TPostScript ps(repchar);
	ps.NewPage();
	TPaveLabel* title = new TPaveLabel(.2,.9,.8,.95,"EMI test report");
	title->Draw();

   TPaveText* pt1 = new TPaveText(.1,.2,.9,.8);
//   ostringstream timestr;
   ostringstream runstr;
   ostringstream meanfailstr;
   ostringstream meanlimitstr;
   ostringstream rmsfailstr;
   ostringstream rmslimitstr;
   ostringstream statusstr;
   ostringstream ntimeslicesstr;
   ostringstream timestr;
   ostringstream npertrigstr;
   ostringstream neventsstr;


   meanlimitstr << " mean pedestal deviation limit: " << max_mean_dev <<" adc units";
   rmslimitstr << " pedestal RMS limit: " << max_mean_dev <<" adc units";
   timestr << " Run duration: " << n_time_slices*time_slice << " seconds";
   ntimeslicesstr << " Number of time slices : " << n_time_slices;
   npertrigstr << " Number of analysed periodic triggers: " << iPerTrig;
//   neventsstr << " Total number of events : " << nEvt;
//   timestr << "Time stamp: " << timestamp;
   runstr << " Run: " << run;
   meanfailstr << " Number of time slices with big mean pedestal deviation : " << n_mean_fail;
   rmsfailstr << " Number of time slices with big pedestal rms : " << n_rms_fail;
   if (n_mean_fail+n_rms_fail == 0) statusstr << " Status: PASSED"; else statusstr << " Status: WARNING";		

   
 //  pt1->AddText(timestr.str().c_str());
    pt1->AddText(runstr.str().c_str());
    pt1->AddText(timestr.str().c_str());
    pt1->AddText(ntimeslicesstr.str().c_str());
    pt1->AddText(neventsstr.str().c_str());
   pt1->AddText(npertrigstr.str().c_str());
   pt1->AddText(meanlimitstr.str().c_str());
   pt1->AddText(rmslimitstr.str().c_str());
   pt1->AddText(meanfailstr.str().c_str());
   pt1->AddText(rmsfailstr.str().c_str());
   pt1->AddText(statusstr.str().c_str());
   pt1->Draw();
	canv_EMI.Update();
	canv_EMI.Clear();
//   ps->NewPage();
   canv_EMI.Divide(1,2);
   (canv_EMI.cd(1))->SetLogy(1);
   gStyle->SetOptStat(0);
   hmean->SetXTitle("mean pedestal deviation, adc units");
   hmean->Draw();
	TH2F* hhmean = new TH2F("hhmean","hhmean",n_time_slices,0,n_time_slices*time_slice,40,-10,10);
        hhmean->SetTitle("mean pedestal vs time for all channels");
        hhmean->SetXTitle("time,seconds");
	hhmean->SetYTitle("mean pedestal deviation, adc units");
	hhmean->GetYaxis()->SetTitleOffset(1.2);
	TH2F* hhrms = new TH2F("hhrms","hhrms",n_time_slices,0,n_time_slices*time_slice,40,0,20);
        hhrms->SetTitle(" pedestal rms vs time for all channels");
        hhrms->SetXTitle("time,seconds");
	hhrms->SetYTitle("  pedestal RMS, adc units");
	hhrms->GetYaxis()->SetTitleOffset(1.2);


	TH2F* hhmeanlat = new TH2F("hhmeanlat","hhmeanlat",n_time_slices,0,n_time_slices*time_slice,40,-1,1);
        hhmeanlat->SetTitle("LAT mean pedestal vs time");
        hhmeanlat->SetXTitle("time,seconds");
	hhmeanlat->SetYTitle("LAT mean pedestal deviation, adc units");
	hhmeanlat->GetYaxis()->SetTitleOffset(1.2);
	TH2F* hhrmslat = new TH2F("hhrmslat","hhrmslat",n_time_slices,0,n_time_slices*time_slice,40,0,2);
        hhrmslat->SetTitle(" LAT pedestal rms vs time");
        hhrmslat->SetXTitle("time,seconds");
	hhrmslat->SetYTitle("LAT pedestal RMS, adc units");
	hhrmslat->GetYaxis()->SetTitleOffset(1.2);



	TH2F* hhmeantwr = new TH2F("hhmeantwr","hhmeantwr",n_time_slices,0,n_time_slices*time_slice,40,-2,2);
        hhmeantwr->SetTitle("Tower mean pedestal vs time");
        hhmeantwr->SetXTitle("time,seconds");
	hhmeantwr->SetYTitle("tower mean pedestal deviation, adc units");
	hhmeantwr->GetYaxis()->SetTitleOffset(1.2);
	TH2F* hhrmstwr = new TH2F("hhrmstwr","hhrmstwr",n_time_slices,0,n_time_slices*time_slice,40,0,4);
        hhrmstwr->SetTitle(" tower pedestal rms vs time");
        hhrmstwr->SetXTitle("time,seconds");
	hhrmstwr->SetYTitle("tower pedestal RMS, adc units");
	hhrmstwr->GetYaxis()->SetTitleOffset(1.2);



	TH2F* hhmeanafee = new TH2F("hhmeanafee","hhmeanafee",n_time_slices,0,n_time_slices*time_slice,40,-2,2);
        hhmeanafee->SetTitle("AFEE mean pedestal vs time");
        hhmeanafee->SetXTitle("time,seconds");
	hhmeanafee->SetYTitle("AFEE mean pedestal deviation, adc units");
	hhmeanafee->GetYaxis()->SetTitleOffset(1.2);
	TH2F* hhrmsafee = new TH2F("hhrmsafee","hhrmsafee",n_time_slices,0,n_time_slices*time_slice,40,0,4);
        hhrmsafee->SetTitle(" AFEE pedestal rms vs time");
        hhrmsafee->SetXTitle("time,seconds");
	hhrmsafee->SetYTitle("AFEE pedestal RMS, adc units");
	hhrmsafee->GetYaxis()->SetTitleOffset(1.2);

	canv_EMI.cd(2);
	TNtuple* ntmean = new TNtuple("ntmean","ntmean","mean:rms:nent:twr:lyr:col:face:slice");
	TNtuple* ntmeantwr = new TNtuple("ntmeantwr","ntmeantwr","mean:rms:nent:twr:slice");
	TNtuple* ntmeanlat = new TNtuple("ntmeanlat","ntmeanlat","mean:rms:nent:slice");
	for(int i=0;i<n_time_slices;i++){
	    float mean = pr_ped_lat->GetBinContent(i+1);
	    float rms = pr_ped_lat->GetBinError(i+1);
	    float nent = pr_ped_lat->GetBinEntries(i+1);
	    ntmeanlat->Fill(mean,rms,nent,i);    					  
		for(int twr=0;twr<16;twr++){
			for(int xy=0;xy<2;xy++)
				for(int face=0;face<2;face++){
					for(int row=0;row<4;row++)
						for(int col=0;col<12;col++){
							int lyr=2*row+xy;
							float mean = (pr_ped_chan[twr][lyr][col][face])->GetBinContent(i+1);
							float rms = (pr_ped_chan[twr][lyr][col][face])->GetBinError(i+1);
							float nent = (pr_ped_chan[twr][lyr][col][face])->GetBinEntries(i+1);
							if(nent>0)ntmean->Fill(mean,rms,nent,twr,lyr,col,face,i);    				
						}
				}
				float mean = (pr_ped_twr[twr])->GetBinContent(i+1);
				float rms = (pr_ped_twr[twr])->GetBinError(i+1);
				float nent = (pr_ped_twr[twr])->GetBinEntries(i+1);
				ntmeantwr->Fill(mean,rms,nent,twr,i);    				
		}
	}
	string meanexpr = "mean:"+str_time_slice+"*(slice+0.5)>>hhmean";
	                        ntmean->Draw(meanexpr.c_str(),"","goff");
				gPad->SetLogz(1);
				gStyle->SetPalette(1,0);
				hhmean->SetMinimum(0.5);
				hhmean->Draw("colz");

				canv_EMI.Update();

   ps.NewPage();
   canv_EMI.cd(1);
   gPad->SetLogy(1);
   hrms->SetXTitle("pedestal RMS, adc units");
   gStyle->SetOptStat(0);
   gStyle->SetPalette(1,0);
   hrms->Draw();
   
   canv_EMI.cd(2);
   string rmsexpr = "min(19.5,rms*sqrt(nent)):"+str_time_slice+"*(slice+0.5)>>hhrms";
    ntmean->Draw(rmsexpr.c_str(),"","goff");
    gPad->SetLogz(1);
    hhrms->SetMinimum(0.5);
    hhrms->Draw("colz");
    

	canv_EMI.Update();
	canv_EMI.Clear();
	//	ps.NewPage();

	TPaveLabel* pagetitle = new TPaveLabel(.2,.9,.8,.95,"List of channels with RMS > 8 adc units");
	pagetitle->Draw();

	TPaveText* pt2 = new TPaveText(.1,.2,.9,.8);

	int nlines=0;
	for(int twr=0;twr<16;twr++)
		for(int lyr=0;lyr<8;lyr++)
			for(int col=0;col<12;col++)
				for(int face=0;face<2;face++){
				  int nwarn=nwarnrms[twr][lyr][col][face];
				  ostringstream line;
				  if(nwarn == 0) continue;
				  if((nlines++)>20)continue;
				  line << " tower=" << twr <<" layer=" << lyr
						<< " column=" << col <<" face=" << face
				                << " has RMS>" << warn_rms <<" in " << nwarn << " time slices";
				  pt2->AddText(line.str().c_str());
				}
	pt2->Draw();
	canv_EMI.Update();




	canv_EMI.Clear();
   canv_EMI.Divide(1,2);
   (canv_EMI.cd(1))->SetLogy(1);
   gStyle->SetOptStat(0);
   hmeantwr->SetXTitle("mean pedestal deviation, adc units");
   hmeantwr->Draw();

   canv_EMI.cd(2);
 
   hhmeantwr->Draw();
    ntmeantwr->SetMarkerStyle(22);
    ntmeantwr->SetMarkerSize(0.4);
    string meantwrexpr = "mean:"+str_time_slice+"*(slice+twr/16.0)";
    ntmeantwr->Draw(meantwrexpr.c_str(),"","same");

   canv_EMI.Update();



   ps.NewPage();
   canv_EMI.cd(1);
   gPad->SetLogy(1);
   hrmstwr->SetXTitle("pedestal RMS, adc units");
   gStyle->SetOptStat(0);
   gStyle->SetPalette(1,0);
   hrmstwr->Draw();
   
   canv_EMI.cd(2);
    hhrmstwr->Draw();
    ntmeantwr->SetMarkerStyle(22);
    ntmeantwr->SetMarkerSize(0.4);
    string rmstwrexpr = "rms*sqrt(nent):"+str_time_slice+"*(slice+twr/16.0)";
    ntmeantwr->Draw(rmstwrexpr.c_str(),"","same");

   canv_EMI.Update();

	canv_EMI.Clear();
   canv_EMI.Divide(1,2);
   (canv_EMI.cd(1))->SetLogy(1);
   gStyle->SetOptStat(0);
   hmeanlat->SetXTitle("mean pedestal deviation, adc units");
   hmeanlat->Draw();

   canv_EMI.cd(2);
 
   hhmeanlat->Draw();
   pr_ped_lat->Draw("same");

   canv_EMI.Update();



   ps.NewPage();
   canv_EMI.cd(1);
   gPad->SetLogy(1);
   hrmslat->SetXTitle("pedestal RMS, adc units");
   hrmslat->SetMinimum(0.5);
   gStyle->SetOptStat(0);
   gStyle->SetPalette(1,0);
   hrmslat->Draw();
   
   canv_EMI.cd(2);

    hhrmslat->Draw();
    ntmeanlat->SetMarkerStyle(22);
    ntmeanlat->SetMarkerSize(0.4);
    string rmslatexpr = "rms*sqrt(nent):"+str_time_slice+"*(slice+0.5)";
    ntmeanlat->Draw(rmslatexpr.c_str(),"","same");
    
   canv_EMI.Update();


   ps.NewPage();
   (canv_EMI.cd(1))->SetLogy(0);
   hcallo->SetMaximum(100);
   hcallo->SetXTitle("time,seconds");
   hcallo->SetMarkerStyle(21);
   hcallo->SetMarkerSize(0.4);
   hcallo->Draw("E0");
   
   (canv_EMI.cd(2))->SetLogy(0);
   hcalhi->SetMaximum(40);
   hcalhi->SetXTitle("time,seconds");
   hcalhi->SetMarkerStyle(21);
   hcalhi->SetMarkerSize(0.4);
   hcalhi->Draw("E0");

   canv_EMI.Update();
	
   ps.Close();	
   ntmean->Write();
   ntmeanlat->Write();
   ntmeantwr->Write();
   hmean->Write();
   hrms->Write();
   hmeanlat->Write();
   hrmslat->Write();
   hmeantwr->Write();
   hrmstwr->Write();
   hmeanafee->Write();
   hrmsafee->Write();
   hhmean->Write();
   hhrms->Write();
   newfile->Close();
   delete newfile;
   return 0;
}
