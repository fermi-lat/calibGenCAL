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
  string run;
  if(argc > 1) run = argv[1];



  ostringstream fcalstr;
  ostringstream fsvacstr;
  ostringstream frepstr;
  ostringstream fhiststr;
  frepstr << "EMI_report_" << run << ".ps";
  fhiststr << "EMI_analysis_hists_" << run << ".root";


  fcalstr  << "/nfs/farm/g/glast/u34/Integration/rootData/"  << run 
           << "/v6r070329p17/recon/recon-v3r6p3_" << run << "_cal_ntuple.root";
  fsvacstr << "/nfs/farm/g/glast/u34/Integration/rootData/"  << run 
           << "/v6r070329p17/svacTuple/v2r2/svacTuple-v3r6p3_" << run << "_svac_svac.root";
  TFile* fcal = new TFile(fcalstr.str().c_str());
  TTree* calTuple = (TTree*)fcal->Get("CalTuple");
  TFile* fsvac = new TFile(fsvacstr.str().c_str()); 
  TTree* svac = (TTree*)fsvac->Get("Output"); 
  TFile* newfile = new TFile(fhiststr.str().c_str(),"RECREATE");

        
  float max_mean_dev=30;
  float max_rms = 30;
  float warn_rms = 8.0;
  int n_mean_fail = 0;
  int n_rms_fail = 0;

  svac->Draw("EventID:EvtTicks*5e-8>>hTimeCalib","","prof");
  double* ticks = svac->GetV2();
  int n = svac->GetSelectedRows();
  double ticks0 = ticks[0];
  double ticksmax = ticks[n-1];
  double time_slice=3.0;
  int n_time_slices = (int)(ticksmax-ticks0)/time_slice;
  cout <<" ticks0=" << ticks0 << " ticksmax=" << ticksmax << " n_time_slices=" << n_time_slices << endl;


  TProfile* pr_ped_chan[16][8][12][2];
  TH1F* h_ped_chan[16][8][12][2];

  TProfile* pr_ped_lat = new TProfile("prpedlat","prpedlat",n_time_slices,0,ticksmax-ticks0,-2,2);
  TProfile* pr_ped_twr[16];
  TProfile* pr_ped_afee[16][2][2];
  TH1F* hcallo = new TH1F("hcallo","CAL_LO rate per time slice",n_time_slices,0,n_time_slices*time_slice);
  TH1F* hcalhi = new TH1F("hcalhi","CAL_HI rate per time slice",n_time_slices,0,n_time_slices*time_slice);
  ostringstream callo_expr;
  ostringstream calhi_expr;
  callo_expr << "EvtTicks*5e-8 - " << ticks0 << ">>hcallo";
  calhi_expr << "EvtTicks*5e-8 - " << ticks0 << ">>hcalhi";

  svac->Draw(callo_expr.str().c_str(),"(GemConditionsWord & 4) !=0","goff");
  svac->Draw(calhi_expr.str().c_str(),"(GemConditionsWord & 8) !=0","goff");

  for(int twr=0;twr<16;twr++){
    ostringstream prpedtwrname;
    prpedtwrname << "prpedtwr_T" << twr;
    pr_ped_twr[twr]=new TProfile(prpedtwrname.str().c_str(),prpedtwrname.str().c_str(),n_time_slices,0,ticksmax-ticks0,-10,10);
    for (int xy=0;xy<2;xy++)
      for (int face=0;face<2;face++){
        ostringstream prpedafeename;
        prpedafeename <<  "prpedafee_T" << twr << "_" << ( (xy==0) ? "X" : "Y") << ( (face==0) ? "P" : "N");
        pr_ped_afee[twr][xy][face]=new TProfile(prpedafeename.str().c_str(),prpedafeename.str().c_str(),n_time_slices,0,ticksmax-ticks0,-10,10);
      }
  }

  for(int twr=0;twr<16;twr++)
    for(int lyr=0;lyr<8;lyr++)
      for(int face=0;face<2;face++){
        for(int col=0;col<12;col++){
          ostringstream hpedname;
          hpedname << "hped_T" << twr  << "_L" << lyr << "_C" << col << "_F" << face;
          ostringstream prpedname;
          prpedname << "prped_T" << twr  << "_L" << lyr << "_C" << col << "_F" << face;
          h_ped_chan[twr][lyr][col][face]=new TH1F(hpedname.str().c_str(),hpedname.str().c_str(),200,-100,100);
          pr_ped_chan[twr][lyr][col][face]=new TProfile(prpedname.str().c_str(),prpedname.str().c_str(),n_time_slices,0,ticksmax-ticks0,-50,50);
        }
      }

        
  UInt_t          EventID;
  UInt_t          GemConditionsWord;
  UInt_t          GemDeltaEventTime;
  double EvtTicks;
  float CalXtalAdcPed[16][8][12][2];

  svac->SetBranchStatus("*",0);
  calTuple->SetBranchStatus("*",0);

  svac->SetBranchStatus("GemConditionsWord",1);
  svac->SetBranchStatus("GemDeltaEventTime",1);
  svac->SetBranchStatus("EventID",1);
  svac->SetBranchStatus("EvtTicks",1);
  calTuple->SetBranchStatus("CalXtalAdcPed*",1);
  
  svac->SetBranchAddress("EventID",&EventID);
  svac->SetBranchAddress("GemConditionsWord",&GemConditionsWord);
  svac->SetBranchAddress("GemDeltaEventTime",&GemDeltaEventTime);
  svac->SetBranchAddress("EvtTicks",&EvtTicks);
  calTuple->SetBranchAddress("CalXtalAdcPed[16][8][12][2]",&CalXtalAdcPed);

  int nEvt = calTuple->GetEntries();
  cout << "in calTuple nEvents=" << nEvt << endl;
  int nEvt_svac = svac->GetEntries();
  cout << "in svac nEvents=" << nEvt_svac << endl;

  int iPerTrig=0;
  for(int iEvt = 0; iEvt != nEvt; ++iEvt) {
    svac->GetEntry(iEvt);
    if(GemConditionsWord != 32 || GemDeltaEventTime*0.05 < 1000)continue;
    calTuple->GetEntry(iEvt);
    iPerTrig++;
    if((iPerTrig%1000) == 0) cout << "  iPerTrig=" << iPerTrig << " iEvt=" << iEvt << endl;  

    for(int twr=0;twr<16;twr++)
      for(int lyr=0;lyr<8;lyr++)
        for(int col=0;col<12;col++)
          for(int face=0;face<2;face++){
            (h_ped_chan[twr][lyr][col][face])->Fill(CalXtalAdcPed[twr][lyr][col][face]);
          }

  }

  float avr[16][8][12][2];
  float sig[16][8][12][2]; 
  int nwarnrms[16][8][12][2];   
  TF1* fun = new TF1("fun","gaus",-100,100);
  for(int twr=0;twr<16;twr++)
    for(int lyr=0;lyr<8;lyr++)
      for(int col=0;col<12;col++)
        for(int face=0;face<2;face++){
          double par[3];
          (h_ped_chan[twr][lyr][col][face])->Fit("fun","QN");
          fun->GetParameters(par);
          avr[twr][lyr][col][face]=par[1];
          sig[twr][lyr][col][face]=par[2];
          (h_ped_chan[twr][lyr][col][face])->Write();
          nwarnrms[twr][lyr][col][face]=0;
        }

  iPerTrig=0;
  for(int iEvt = 0; iEvt != nEvt; ++iEvt) {
    svac->GetEntry(iEvt);
    if(GemConditionsWord != 32 || GemDeltaEventTime*0.05 < 1000)continue;
    calTuple->GetEntry(iEvt);
    iPerTrig++;
    if((iPerTrig%1000) == 0) cout << "  iPerTrig=" << iPerTrig << " iEvt=" << iEvt << endl;  

    float time = EvtTicks*5e-8-ticks0;
    float latsum=0.0;
        
    for(int twr=0;twr<16;twr++){
      float twrsum=0.0;
      for(int xy=0;xy<2;xy++)
        for(int face=0;face<2;face++){
          float afeesum=0.0;
          for(int row=0;row<4;row++)
            for(int col=0;col<12;col++)
              {
                int lyr=row*2+xy;
                float dped = CalXtalAdcPed[twr][lyr][col][face]-avr[twr][lyr][col][face];
                (pr_ped_chan[twr][lyr][col][face])->Fill(time,dped);
                latsum += dped;
                twrsum +=dped;
                afeesum += dped;
              }
          pr_ped_afee[twr][xy][face]->Fill(time,afeesum/48.0); 
        }
      pr_ped_twr[twr]->Fill(time,twrsum/192.0); 
    }
    pr_ped_lat->Fill(time,latsum/3072.0); 
  }
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
                                        
                float mean = (pr_ped_chan[twr][lyr][col][face])->GetBinContent(i+1);
                float rms = ((pr_ped_chan[twr][lyr][col][face])->GetBinError(i+1))
                  *sqrt((pr_ped_chan[twr][lyr][col][face])->GetBinEntries(i+1));
                hmean->Fill(mean);
                hrms->Fill(rms);
                if(mean > max_mean_dev)n_mean_fail++;
                if(rms > max_rms)n_rms_fail++;
                if(rms > warn_rms)(nwarnrms[twr][lyr][col][face])++;
              }
          float mean = (pr_ped_afee[twr][xy][face])->GetBinContent(i+1);
          float rms = ((pr_ped_afee[twr][xy][face])->GetBinError(i+1))
            *sqrt((pr_ped_afee[twr][xy][face])->GetBinEntries(i+1));
          hmeanafee->Fill(mean);
          hrmsafee->Fill(rms);
                    
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


  for(int twr=0;twr<16;twr++){
    for(int xy=0;xy<2;xy++)
      for(int face=0;face<2;face++){
        for(int row=0;row<4;row++)
          for(int col=0;col<12;col++){
            int lyr=xy+row*2;
            (pr_ped_chan[twr][lyr][col][face])->Write();
          }
        pr_ped_afee[twr][xy][face]->Write(); 
      }
    pr_ped_twr[twr]->Write();   
  }

  pr_ped_lat->Write();

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
  timestr << " Run duration: " << ticksmax-ticks0 << " seconds";
  ntimeslicesstr << " Number of time slices : " << n_time_slices;
  npertrigstr << " Number of analysed periodic triggers: " << iPerTrig;
  neventsstr << " Total number of events : " << nEvt;
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
  TNtuple* ntmeanafee = new TNtuple("ntmeanafee","ntmeanafee","mean:rms:nent:twr:xy:face:slice");
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
              ntmean->Fill(mean,rms,nent,twr,lyr,col,face,i);                                   
            }
          float mean = (pr_ped_afee[twr][xy][face])->GetBinContent(i+1);
          float rms = (pr_ped_afee[twr][xy][face])->GetBinError(i+1);
          float nent = (pr_ped_afee[twr][xy][face])->GetBinEntries(i+1);
          ntmeanafee->Fill(mean,rms,nent,twr,xy,face,i);                                
        }
      float mean = (pr_ped_twr[twr])->GetBinContent(i+1);
      float rms = (pr_ped_twr[twr])->GetBinError(i+1);
      float nent = (pr_ped_twr[twr])->GetBinEntries(i+1);
      ntmeantwr->Fill(mean,rms,nent,twr,i);                             
    }
  }
  ntmean->Draw("mean:3*slice+1.5>>hhmean","","goff");
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
  ntmean->Draw("min(19.5,rms*sqrt(nent)):3*slice+1.5>>hhrms","","goff");
  gPad->SetLogz(1);
  hhrms->SetMinimum(0.5);
  hhrms->Draw("colz");
    

  canv_EMI.Update();
  canv_EMI.Clear();
  //    ps.NewPage();

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
  hmeanafee->SetXTitle("mean pedestal deviation, adc units");
  hmeanafee->Draw();

  canv_EMI.cd(2);
 
  hhmeanafee->Draw();
  ntmeanafee->Draw("mean:3*(slice+(4*twr+2*xy+face)/64.0)","","same");

  canv_EMI.Update();



  ps.NewPage();
  canv_EMI.cd(1);
  gPad->SetLogy(1);
  hrmsafee->SetXTitle("pedestal RMS, adc units");
  gStyle->SetOptStat(0);
  hrmsafee->Draw();
   
  canv_EMI.cd(2);
  hhrmsafee->Draw();
  ntmeanafee->Draw("rms*sqrt(nent):3*(slice+(4*twr+2*xy+face)/64.0)","","same");

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
  ntmeantwr->Draw("mean:3*(slice+twr/16.0)","","same");

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
  ntmeantwr->Draw("rms*sqrt(nent):3*(slice+twr/16.0)","","same");

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
  ntmeanlat->Draw("rms*sqrt(nent):3*slice+1.5","","same");
    
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
  ntmeanafee->Write();
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
