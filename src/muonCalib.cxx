#include "muonCalib.h"

// ROOT INCLUDES
#include "TStyle.h"
#include "TF2.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TNtuple.h"
#include "TProfile.h"
#include "TSpline.h"

#include <cmath>
#include <iostream>
#include <cstring>
#include <fstream>

const std::string muonCalib::FACE_MNEM[] = {"NEG",
														  "POS"};
const std::string muonCalib::RNG_MNEM[] = {"LEX8",
														 "LEX1",
														 "HEX8",
														 "HEX1"};

muonCalib::muonCalib() : RootFileAnalysis(0,0,0) {
  ZeroMembers();
};

muonCalib::muonCalib(std::vector<std::string> *digiFileNames,
							std::vector<std::string> *recFileNames,
							std::vector<std::string> *mcFileNames,
							const char *histFileName) : RootFileAnalysis(digiFileNames,recFileNames,mcFileNames) {
  ZeroMembers();

  SetHistFileName(histFileName);
  HistDefine();
  MakeHistList();

}

muonCalib::~muonCalib() {
  if (m_histList) delete m_histList;  // have to delete m_histList first, otherwise it's members are pulled out from under by histFile close.
  if (histFile) histFile->Close();
  if (histFile) delete histFile;
}

void muonCalib::DigiHistDefine() {
  // Purpose and Method:  Digitization histogram definitions

  new TH1F("LTOT","LTOT",10,0,10);
  new TH1F("NTOT","NTOT",100,0,100);
  new TH1F("LHOLES","LHOLES",10,0,10);
  new TH1F("MAXNL","MAXNL",10,0,10);
  new TH1F("TX","TX",50,-1.0,1.0);
  new TH1F("TY","TY",50,-1.0,1.0);

  // used to set scale before draw TGraph gx, gy
  TH2F *xview = new TH2F("xview","xview",8,-0.5, 7.5,12,-0.5,11.5);
  TH2F *yview = new TH2F("yview","yview",8,-0.5, 7.5,12,-0.5,11.5);

  c1 = new TCanvas("c1","event display",800,600);
  c1->Divide(2,1);
  c1->cd(1); xview->Draw();
  c1->cd(2); yview->Draw();
  gx = new TGraph(40); gx->Draw("*");
  gy = new TGraph(40); gy->Draw("*");
  glx = new TGraphErrors(40); glx->SetMarkerStyle(23); glx->Draw("P");
  gly = new TGraphErrors(40); gly->SetMarkerStyle(23); gly->Draw("P");
  c1->Update();
  xline  = new TF1("xline","pol1",0,8);
  yline  = new TF1("yline","pol1",0,8);
  xlongl  = new TF1("xlongl","pol1",0,8);
  ylongl  = new TF1("ylongl","pol1",0,8);

  pedhist = new TObjArray(192);
  corrpedhist = new TObjArray(192);

  rawAdcHist = new TObjArray(768);
  pdahist = new TObjArray(768);
  corrpdahist = new TObjArray(768);
  rawhist = new TObjArray(192);
  thrhist = new TObjArray(192);
  adjhist = new TObjArray(192);
  midhist = new TObjArray(192);
  asycalib = new TObjArray(192);
  poshist = new TObjArray(96);
  rathist = new TObjArray(96);
  ratfull = new TObjArray(96);
  asymCorr = new TObjArray(96);
  asyhist = new TObjArray(96);
  reshist = new TObjArray(96);
  ratntup = new TObjArray(192);
  prevTimeStamp = 0;

  for (int layer=0;layer <8;layer++){
    for(int col=0;col<12;col++){
      char poshisname[]="pos000";
      char rathisname[]="rat000";
      char ratfulname[]="raf000";
      char rawhisname[]="raw000";
      char asyhisname[]="asy000";
      char reshisname[]="res000";
      sprintf(poshisname,"pos%1d%02d",layer,col);
      sprintf(rathisname,"rat%1d%02d",layer,col);
      sprintf(ratfulname,"raf%1d%02d",layer,col);
      sprintf(rawhisname,"raw%1d%02d",layer,col);
      sprintf(asyhisname,"asy%1d%02d",layer,col);
      sprintf(reshisname,"res%1d%02d",layer,col);
      poshist->AddAt(new TH1F(poshisname,poshisname,60,-1,11),layer*12+col);
      rathist->AddAt(new TProfile(rathisname,rathisname,6,-3,3," "),layer*12+col);

      ratfull->AddAt(new TProfile(ratfulname,ratfulname,12,-16.7064,16.7064," "),layer*12+col);

      asyhist->AddAt(new TProfile(asyhisname,asyhisname,6,-3,3," "),layer*12+col);

      reshist->AddAt(new TH1F(reshisname,reshisname,100,-5,5),layer*12+col);

      rawhist->AddAt(new TH1F(rawhisname,rawhisname,200,-300,6700),layer*12+col);

      for(int side = 0;side<2;side++){
		  char pedhisname[]="ped0000";
		  char corrpedhisname[]="corrped0000";
		  char thrhisname[]="thr0000";
		  //char adjhisname[]="adj0000";
		  // char midhisname[]="mid0000";
		  char asycalibname[]="acl0000";
		  char ratntpname[]="rtp0000";
		  int  histid= (layer*12+col)*2+side;
		  sprintf(pedhisname,"ped%1d%02d%1d",layer,col,side);
		  sprintf(corrpedhisname,"corrped%1d%02d%1d",layer,col,side);
		  sprintf(thrhisname,"thr%1d%02d%1d",layer,col,side);
		  sprintf(asycalibname,"acl%1d%02d%1d",layer,col,side);
		  sprintf(ratntpname,"rtp%1d%02d%1d",layer,col,side);
		  //     sprintf(adjhisname,"adj%1d%02d%1d",layer,col,side);
		  //     sprintf(midhisname,"mid%1d%02d%1d",layer,col,side);
		  pedhist->AddAt(new TH1F(pedhisname,pedhisname,500,0,1000),histid);
		  corrpedhist->AddAt(new TH2F(corrpedhisname,corrpedhisname,
												61, -20.5, 40.5, 21, -10.5, 10.5 ),histid);

		  thrhist->AddAt(new TH1F(thrhisname,thrhisname,300,0,3000),histid);
		  asycalib->AddAt(new TH1F(asycalibname,asycalibname,200,-0.5,0.5),histid);

		  ratntup->AddAt(new TNtuple(ratntpname,ratntpname,"lex8:lex1:hex8:hex1"),
							  (layer*12+col)*2+side);
		  //     adjhist->AddAt(new TH1F(adjhisname,adjhisname,120,0,600),histid);
		  //     midhist->AddAt(new TH1F(midhisname,midhisname,120,0,1200),histid);

		  for(int rng=0;rng<4;rng++){
			 char rawAdcName[] = "adc00000";
			 sprintf(rawAdcName, "adc%1d%02d%1d%1d", layer, col, side, rng);
			 rawAdcHist->AddAt(new TH1F(rawAdcName, rawAdcName,300, 0, 4200),
									 histid*4+rng);

			 char pdahisname[]="pda00000";
			 sprintf(pdahisname,"pda%1d%02d%1d%1d",layer,col,side,rng);
			 pdahist->AddAt(new TH1F(pdahisname,pdahisname,1000,0,1000),histid*4+rng);
		  }

		  for(int rng=0;rng<4;rng+=2){
			 char corrpdahisname[]="corrpda00000";
			 sprintf(corrpdahisname,"corrpda%1d%02d%1d%1d",layer,col,side,rng);
			 corrpdahist->AddAt(new TH2F(corrpdahisname,corrpdahisname,
												  61, -20.5, 40.5, 21, -10.5, 10.5 ),
									  rng/2+side*2+col*4+layer*48);
		  }

		  /*
			 for(int trcol = 0; trcol<10; trcol++){
			 char verthisname[]="vert0000";
			 sprintf(verthisname,"vert%1d%1d%1d%1d",layer,col,side,trcol);
			 verthist->AddAt(new TH1F(verthisname,verthisname,100,0,500),histid*10+trcol);
			 }
		  */
      }
    }
  }

}

void muonCalib::DigiCal()
{
  // Purpose and Method:  Process on one CAL digi event

  float thresh = 300;
  float adctot; double timestamp,dtime;
  timestamp = evt->getTimeStamp();
  dtime=timestamp-prevTimeStamp;
  //float event = evt->getEventId();

  prevTimeStamp = timestamp;
  adctot=0;
  const TObjArray* calDigiCol = evt->getCalDigiCol();
  if (!calDigiCol) return;

  //int nCalDigi = calDigiCol->GetEntries();

  CalDigi *cdig = 0;

  for( int l=0; l<8; l++)
    for (int c=0;c<12;c++)
      for(int s=0;s<2;s++){
		  a[l][c][s]=0;
		  ar[l][c][s]=0;
      }

  for( int cde_nb=0; (cdig=(CalDigi*) calDigiCol->At(cde_nb)); cde_nb++ ){  //loop through each 'hit' in one event
    const CalXtalReadout* cRo=cdig->getXtalReadout(0);

    CalXtalId id = cdig->getPackedId();  // get interaction information
    int layer = id.getLayer();
    //int tower = id.getTower();
    int col = id.getColumn();
    float adcP = cRo->getAdc(CalXtalId::POS);
    float adcM = cRo->getAdc(CalXtalId::NEG);
    int range = 0;

    if ( go_type == FILLPEDHIST || go_type == FILLPEDHIST4RANGES
			|| go_type == FILLRATHIST || go_type == FILLCORRPEDHIST
			|| go_type == FILLCORRPEDHIST2RANGES  ) {

      int histid = (layer*12+col)*2;

      if (go_type == FILLPEDHIST) {
		  ((TH1F*)pedhist->At(histid))->Fill(adcM);
		  ((TH1F*)pedhist->At(histid+1))->Fill(adcP);
      }

      if (go_type == FILLCORRPEDHIST) {
		  ((TH2F*)corrpedhist->At(histid))  ->Fill(adcM-m_calPed[0][layer][col][0]);
		  ((TH2F*)corrpedhist->At(histid+1))->Fill(adcP-m_calPed[0][layer][col][1]);
      }

      int numRo = cdig->getNumReadouts();
      float aadcP[4],aadcM[4];

      if( go_type == FILLPEDHIST4RANGES || go_type == FILLRATHIST
			 || go_type == FILLCORRPEDHIST2RANGES ){
		  for (int iRo=0;iRo<numRo;iRo++){
			 const CalXtalReadout* acRo=cdig->getXtalReadout(iRo);
			 aadcP[iRo] = acRo->getAdc(CalXtalId::POS);
			 aadcM[iRo] = acRo->getAdc(CalXtalId::NEG);

			 ((TH1F*) rawAdcHist->At(histid*4+iRo))->Fill(aadcM[iRo]);
			 ((TH1F*) rawAdcHist->At((histid+1)*4+iRo))->Fill(aadcP[iRo]);
		  }

      }

      if(go_type == FILLRATHIST ){
		  float aarP[4],aarM[4];
		  for(int iRo=0;iRo<numRo;iRo++){
			 aarM[iRo]=aadcM[iRo]-m_calPed[iRo][layer][col][0];
			 aarP[iRo]=aadcP[iRo]-m_calPed[iRo][layer][col][1];
		  }

//		  ((TNtuple*)ratntup->At((12*layer+col)*2))->Fill(aarM);
//		  ((TNtuple*)ratntup->At((12*layer+col)*2+1))->Fill(aarP);
      }
      if ( (go_type == FILLPEDHIST4RANGES || go_type == FILLCORRPEDHIST2RANGES) &&
			  fabs(adcM - m_calPed[0][layer][col][0])<5*m_calPedRms[0][layer][col][0] &&
			  fabs(adcP - m_calPed[0][layer][col][1])<5*m_calPedRms[0][layer][col][0]
			  ) {

		  if (go_type == FILLPEDHIST4RANGES)
			 for (int iRo=0;iRo<numRo;iRo++){
				((TH1F*)pdahist->At((histid)*4+iRo))->Fill(aadcM[iRo]);
				((TH1F*)pdahist->At((histid+1)*4+iRo))->Fill(aadcP[iRo]);
			 }
		  if (go_type == FILLCORRPEDHIST2RANGES)
			 for (int iRo=0;iRo<numRo;iRo+=2){
				((TH2F*)corrpdahist->At(iRo/2+col*4+layer*48))->Fill(
																					  aadcM[iRo]-m_calPed[iRo][layer][col][0],
																					  aadcM[iRo+1]-m_calPed[iRo+1][layer][col][0] );
				((TH2F*)corrpdahist->At(iRo/2+2+col*4+layer*48 ))->Fill(
																						  aadcP[iRo]-m_calPed[iRo][layer][col][1],
																						  aadcP[iRo+1]-m_calPed[iRo+1][layer][col][1] );
			 }
      }
    }

    if ( go_type == FILLMUHIST || go_type == FILLRATHIST ) {

      // m_calCorr are initially set as 3
      // during FILLRATHIST (light asymmetry calibration), m_calCorr

      a[layer][col][0] = (adcM - m_calPed[range][layer][col][0])*m_calCorr[layer][col][0];
      a[layer][col][1] = (adcP - m_calPed[range][layer][col][1])*m_calCorr[layer][col][1];
      ar[layer][col][0] = (adcM - m_calPed[range][layer][col][0]);
      ar[layer][col][1] = (adcP - m_calPed[range][layer][col][1]);

    }

  }   // for calDigiCol

  adctot=0;   // sum up adc total, if we have any
  for(int ll=0; ll<8; ll++){
    for( int cc=0; cc<12; cc++){
      for( int ss=0; ss<2; ss++){
		  adctot += ar[ll][cc][ss];
      }
    }
  }

  if(go_type == FILLMUHIST || go_type == FILLRATHIST){

    if(go_type == FILLRATHIST)thresh=200; else thresh=200;

    float xc[100],xl[100],yc[100],yl[100], tlx,tly,yl0,xl0;

    int lfirst = 0, llast = 0; // first and last layer with at least 1 log fired

    int nmax = 100;  // maximal number of points in Graph
    int nl[8];  // no of logs fired at each layer

    int ntot = 0;   // total number of logs fired

    // total number of logs fired in all measure X(Y) layers, can not exceed nmax
    int nx = 0, ny = 0;

    int ltot = 0;   // total number of layers with at least 1 log fired
    int ltotx = 0, ltoty = 0;

    int maxnl=0;  // maximal number of logs fired in any single layer
    int maxnlx=0; int maxnly=0;

    // fit results of fitting gx, gy with a polynomial: a+bx
    // tx, ty are fitted slopes; x0, y0 are intercepts
    float tx=0, ty=0, y0=0, x0=0;

    tlx=0; tly=0; yl0=0; xl0=0;

    for(int l=0; l<8; l++){
      nl[l]=0;

      for(int c=0; c<12; c++){

		  int thr2s = (a[l][c][0] + a[l][c][1]) > thresh ;

		  if(thr2s){
			 ntot++;
			 //float ratio = a[l][c][1]/(a[l][c][0]);

			 (nl[l])++; if(nl[l]>maxnl) maxnl=nl[l];
			 switch (l%2) {

			 case 0 :

				if(nx<nmax){
				  xc[nx]=c; xl[nx]=l;nx++;
				  gx->Set(nx); gx->SetPoint(nx-1,l,c);
				}

				if(nl[l]>maxnlx) maxnlx=nl[l];

				break;

			 case 1 :
				if(ny<nmax){
				  yc[ny]=c; yl[ny]=l;ny++;
				  gy->Set(ny); gy->SetPoint(ny-1,l,c);
				}

				if(nl[l]>maxnly) maxnly=nl[l];

				break;

			 }
		  }  // if(thr2s)
      }  // for(int c=0; c<12; c++){

      if(nl[l]>0){
		  ltot++;
		  if(l%2)ltoty++; else ltotx++;
      }

    }  // for(int l=0; l<8; l++)

    for(lfirst=0; lfirst<8 && nl[lfirst]==0;lfirst++);
    for(llast=7; llast>=0 && nl[llast]==0;llast--);
	 TH1F* ptLTOT = ((TH1F*)GetObjectPtr("LTOT"));
    ptLTOT->Fill(ltot);
    if(ltot>0)((TH1F*)GetObjectPtr("LHOLES"))->Fill(llast-lfirst+1-ltot);

    ((TH1F*)GetObjectPtr("MAXNL"))->Fill(maxnl);
    ((TH1F*)GetObjectPtr("NTOT"))->Fill(ntot);

    if(nx > 0){

      c1->cd(1);
      ((TH2F*)GetObjectPtr("xview"))->Draw();
      gx->Draw("*");

      if(nx>1){

		  gx->Fit(xline,"WQN");
		  gly->Fit(xlongl,"WQN");
		  double* xpar= xline->GetParameters(); tx=*(xpar+1); x0=*xpar;
		  double* xlpar= xlongl->GetParameters(); tlx=*(xlpar+1); xl0=*xlpar;
		  ((TH1F*)GetObjectPtr("TX"))->Fill(tlx);

      }
    }

    if(ny > 0) {

      c1->cd(2);
      ((TH2F*)GetObjectPtr("yview"))->Draw();gy->Draw("*");
      if(ny>1){
		  gy->Fit(yline,"WQN"); glx->Fit(ylongl,"WQN");
		  double* ypar= yline->GetParameters(); ty=*(ypar+1); y0=*ypar;
		  double* ylpar= ylongl->GetParameters(); tly=*(ylpar+1); yl0=*ylpar;
		  ((TH1F*)GetObjectPtr("TY"))->Fill(tly);

      }
    }

    //float thrsel=200;
    for(int l=0; l<8; l++){
      for(int c=0; c<12; c++){

		  digi_select[l][c]=0;

		  float posx = l*tx+x0; float posy=l*ty+y0;
		  //float poslx = l*tlx+xl0; float posly=l*tly+yl0;
		  float pos=(l%2) ? posx:posy;
		  //float posl=(l%2) ? poslx:posly;

		  int histid = l*12+c;
		  float ratio = 1.0;

		  int thr2s = (a[l][c][0] + a[l][c][1]) > thresh;

		  int adj=1;

		  // determine whether neighbour log's signal are less than the threshold
		  if (c <7) adj = (adj && a[l][c+1][0] + a[l][c+1][1]<thresh);
		  if (c >0) adj = (adj && a[l][c-1][0] + a[l][c-1][1]<thresh);

		  ratio = a[l][c][1]/(a[l][c][0]);

		  if (a[l][c][0]+a[l][c][1] != 0)
			 ((TH1F*)rawhist->At(histid))->Fill(a[l][c][0]+a[l][c][1]);

		  // Require following conditions:
		  // 1. signal in the log is above the threshold
		  // 2. adjacent logs have signals below the threshold
		  // 3. total number of logs fired in any given layer less than 3
		  // 4. fitted track must be close to vertical within measured x or
		  //    measured y layer
		  if(thr2s && adj && maxnl < 3 &&
			  ( ( (l%2)==0 && ltotx > 3  && ltoty>1 && maxnlx==1 &&
					fabs(tx)<0.01 && fabs(ty)<0.5 ) ||
				 ( (l%2)==1 && ltoty > 3 && ltotx>1 && maxnly==1  &&
					fabs(ty)<0.01 && fabs(tx)<0.5 )
				 ) ) {

			 if(ratio > 0)

				// 2.7844 is width of a log, used to translate from log number to
				// actual position

				if( asym_corr_type == SLOPE){
				  ((TProfile*)ratfull->At(12*l+c))->Fill((pos-5.5)*2.7844,log(ratio)+m_calSlopes[l][c]*(pos-5.5));
				}else if(asym_corr_type == SPLINE){
				  TSpline3* spl = (TSpline3*) (asymCorr->At(12*l+c));
				  double pos_eval = spl->Eval(log(ratio));
				  /*
					 std::cout << " l="<< l<< " c=" << c
					 << " pos=" << pos-5.5 << " pos_eval=" << pos_eval << std::endl;
				  */
				  ((TProfile*)ratfull->At(12*l+c))->Fill((pos-5.5)*2.7844,pos_eval-(pos-5.5));
				}else{
				  ((TProfile*)ratfull->At(12*l+c))->Fill((pos-5.5)*2.7844,log(ratio));
				}

			 if(pos > 1.5 && pos < 9.5){
				digi_select[l][c] = 1;

				// 1.304 is the ratio between double log height and log width
				// tx is calculated using log number rather than actual length
				// here is the conversion
				float tanx=1.304*tx; float tany=1.304*ty;
				float secth = sqrt(1+tanx*tanx+tany*tany);
				float slcoef = m_calSlopes[l][c]*(pos-5.5)/2;

				float asym = (a[l][c][1]-a[l][c][0])/(a[l][c][1]+a[l][c][1]);
				((TProfile*)asyhist->At(12*l+c))->Fill(pos-5.5,asym);
				if(ratio > 0){
				  ((TProfile*)rathist->At(12*l+c))->Fill(pos-5.5,log(ratio));
				}

				for(int s=0; s<2; s++){
				  int histid = (l*12+c)*2+s;
				  float adc_ped = a[l][c][s] *(1+(2*s-1)*slcoef);
				  ((TH1F*)thrhist->At(histid))->Fill(adc_ped/secth);
				}
			 }  // if(pos > 1.5 && pos < 9.5)
		  }
      } // for(int c=0; c<12; c++){
    } // for( l=0; l<8; l++){
  }  // if(go_type == FILLMUHIST || go_type == FILLRATHIST)
}

void muonCalib::FitRatHist(){
  for (int layer=0;layer < 8;layer++){
	 for(int col=0;col<12;col++){
		int histid = layer*12+col;
		TProfile* h = (TProfile*)rathist->At(histid);
		h->Fit("pol1","Q");
		double* par = (h->GetFunction("pol1"))->GetParameters();
		float logratio = *par; float slope = *(par+1);
		std::cout << " layer=" << layer << " col=" << col <<
		  " logratio =" <<logratio << " slope=" << slope << std::endl;
		m_calCorr[layer][col][0] *= exp(logratio/2);
		m_calCorr[layer][col][1] *= exp(-logratio/2);
		m_calSlopes[layer][col] = -slope;
	 }
  }

}

void muonCalib::FitMuHist(){
  for (int layer=0;layer < 8;layer++){
	 for(int col=0;col<12;col++){
	   for(int side=0;side<2;side++){
		  int histid = (layer*12+col)*2+side;
		  TH1F* h = (TH1F*)thrhist->At(histid);
		  float ave = h->GetMean();
		  float rms = h->GetRMS();
		  h->Fit("landau", "Q", "", ave-2*rms, ave+3*rms);
		  double* par = (h->GetFunction("landau"))->GetParameters();
		  float mean = *(par+1); float sigma=*(par+2);
		  std::cout << " mean=" << mean << "  sigma=" << sigma << std::endl;

		  // note *= here, gain correction applied
		  m_calCorr[layer][col][side] *=1000/mean;
		  m_muRelSigma[layer][col][side] = sigma/mean;

	   }
	 }
  }
}

void muonCalib::WriteMuPeaks(const char* fileName){
  std::ofstream mpout(fileName);
  for (int layer=0;layer < 8;layer++){
	 for(int col=0;col<12;col++){
		for(int side=0;side<2;side++){
		  mpout << " " << side << " " << col << " " << layer <<" "

			 // note compared to fitMuHist, gain corection is removed here
				  << 1000/m_calCorr[layer][col][side] << " "
				  << m_muRelSigma[layer][col][side]
				  << std::endl;
		}
	 }
  }
}

int muonCalib::WriteMuPeaksXML(const char *fileName) {
  ofstream outFile(fileName);
  if (!outFile.is_open()) {
	 std::cout << "ERROR! unable to open txtFile='" << fileName << "'" << std::endl;
	 return -1;
  }

  outFile << "<?xml version=\"1.0\" ?>" << std::endl;
  outFile << "<!DOCTYPE calCalib SYSTEM \"$(CALIBUTILROOT)/xml/calCalib_v2r1.dtd\" [] >" << std::endl;
  outFile << "<calCalib>" << std::endl;
  outFile << "<generic instrument=\"EM\" timestamp=\"2003-10-1-12:56\" calibType=\"CAL_ElecGain\" fmtVersion=\"v3r3p2\">" << std::endl;
  outFile << "</generic>" << std::endl;
  outFile << "<dimension nRow=\"1\" nCol=\"1\" nLayer=\"8\" nXtal=\"12\" nFace=\"2\" />" << std::endl;
  outFile << "<tower iRow=\"0\" iCol=\"0\">" << std::endl;

  float range_ratios[] = {9.0, 0.6, 9.0};

  for (int layer=0;layer <8;layer++){
	 outFile << "    <layer iLayer=\"" << layer << "\">" << std::endl;
	 for(int col=0;col<12;col++){
		outFile << "      <xtal iXtal=\"" << col << "\">" << std::endl;
		for(int side = 0;side<2;side++){
		  const std::string face(FACE_MNEM[side]);
		  outFile << "        <face end=\"" << face << "\">" << std::endl;
		  // change muon peak from 12.3 MeV to 11.2 MeV, 11.2 MeV is obtained by fitting MC energy spectrum deposited in a single crystal with a landau function, 11.2 MeV is the fit peak position while 12.3 MeV is the average value obtained from PDG.
		  float av = 11.2*m_calCorr[layer][col][side]/1000;
		  float rms = m_muRelSigma[layer][col][side];
		  for(int range=0; range <4; range++){
			 outFile <<"            <calGain avg=\"" << av
						<< "\" sig=\"" << rms << "\" range=\"" << RNG_MNEM[range] << "\" />"
						<< std::endl;
			 if(range < 3) av *= range_ratios[range];
		  }
		  outFile << "        </face>" << std::endl;
		}
		outFile << "       </xtal>" << std::endl;
	 }
	 outFile << "     </layer>" << std::endl;
  }
  outFile << "</tower>" << std::endl << "</calCalib>" << std::endl;

  outFile.close();
  return 0;
}

void muonCalib::WriteMuSlopes(const char* fileName){
  std::ofstream mslout(fileName);
  for (int layer=0;layer < 8;layer++){
	 for(int col=0;col<12;col++){
		mslout << " " << col << " " << layer <<" "
				 << 2*2.7844/m_calSlopes[layer][col] << std::endl;
	 }
  }
}

int muonCalib::WriteMuSlopesXML(const char *fileName) {
  ofstream outFile(fileName);
  if (!outFile.is_open()) {
	 std::cout << "ERROR! unable to open txtFile='" << fileName << "'" << std::endl;
	 return -1;
  }

  outFile << "<?xml version=\"1.0\" ?>" << std::endl;
  outFile << "<!DOCTYPE calCalib SYSTEM \"$(CALIBUTILROOT)/xml/calCalib_v2r1.dtd\" [] >" << std::endl;
  outFile << "<calCalib>" << std::endl;
  outFile << "<generic instrument=\"EM\" timestamp=\"2003-10-1-12:56\" calibType=\"CAL_MuSlope\" fmtVersion=\"v3r3p2\">" << std::endl;
  outFile << "</generic>" << std::endl;
  outFile << "<dimension nRow=\"1\" nCol=\"1\" nLayer=\"8\" nXtal=\"12\" nFace=\"1\" />" << std::endl;
  outFile << "<tower iRow=\"0\" iCol=\"0\">" << std::endl;

  for (int layer=0;layer <8;layer++){
	 outFile << "    <layer iLayer=\"" << layer << "\">" << std::endl;
	 for(int col=0;col<12;col++){
		outFile << "      <xtal iXtal=\"" << col << "\">" << std::endl;
		const std::string face("NA");
		outFile << "        <face end=\"" << face << "\">" << std::endl;
		float av = -10*2*2.7844/m_calSlopes[layer][col];
		for(int range=0; range <4; range++){
		  outFile <<"            <muSlope slope=\"" << av
					 << "\" range=\"" << RNG_MNEM[range] << "\" />"
					 << std::endl;
		}
		outFile << "        </face>" << std::endl;
		outFile << "       </xtal>" << std::endl;
	 }
	 outFile << "     </layer>" << std::endl;
  }
  outFile << "</tower>" << std::endl << "</calCalib>" << std::endl;

  outFile.close();
  return 0;
}

void muonCalib::FitPedHist(){
  for (int layer=0;layer < 8;layer++){
	 for(int col=0;col<12;col++){
		for(int side = 0;side <2;side++){
		  int nrng = (go_type == FILLPEDHIST4RANGES) ? 4 : 1;
		  for(int rng = 0;rng<nrng; rng++){
			 TH1F* h;
			 if(go_type == FILLPEDHIST4RANGES) {
				int histid = ((layer*12+col)*2+side)*4+rng;
				h = (TH1F*)pdahist->At(histid);
			 }else{
				int histid = (layer*12+col)*2+side;
				h = (TH1F*)pedhist->At(histid);
			 }
			 float av = h->GetMean(); float rms = h->GetRMS();
			 for( int iter=0; iter<3; iter++ ){          // get rid of far off values.
				h->SetAxisRange(av-3*rms,av+3*rms);       // mean & rms will be calculated at new range
				av = h->GetMean(); rms = h->GetRMS();
			 }
			 int fitresult= h->Fit("gaus", "Q","", av-3*rms, av+3*rms );  // run gaussian fit
			 std::cout<<h->GetName()<<"  "<<fitresult<<std::endl;
			 h->SetAxisRange(av-150,av+150);

			 m_calPed[rng][layer][col][side] =
				((TF1*)h->GetFunction("gaus"))->GetParameter(1);
			 m_calPedRms[rng][layer][col][side] =
				((TF1*)h->GetFunction("gaus"))->GetParameter(2);
		  }
		}
	 }
  }
}

void muonCalib::FitCorrPedHist(){
  Double_t fit_ranges[4];
  //char name[]="corrpda00000";
  TH2F* pdl;
  TF2 *gauss= new TF2("gauss",
							 "[0]/(6.28*sqrt([1]*[2]))*exp( -0.5/([1]*[1])*((x-[4])*cos([3])+(y-[5])*sin([3]))**2 ) * exp( -0.5/([2]*[2])*((y-[5])*cos([3])-(x-[4])*sin([3]))**2)" );
  gauss->SetParName( 0, "area" );
  gauss->SetParName( 1, "sigma_X" );
  gauss->SetParName( 2, "sigma_Y" );
  gauss->SetParName( 3, "angle" );
  gauss->SetParName( 4, "X_MPV" );
  gauss->SetParName( 5, "Y_MPV" );

  for( short int lr=0; lr<8; lr++ )
	 for( short int col=0; col<12; col++ )
		for( short int fc=0; fc<2; fc++ )
		  for( short int rg=0; rg<4; rg+=2 ){
			 if( go_type==FILLCORRPEDHIST && rg==2 ) continue;
			 if ( go_type==FILLCORRPEDHIST )
				pdl= (TH2F*) corrpedhist->At( fc+col*2+lr*24);
			 else
				pdl= (TH2F*) corrpdahist->At( rg/2+fc*2+col*4+lr*48);

			 for( int jj=0; jj<3; jj++ ){
				fit_ranges[0]=   pdl->GetMean(2)-pdl->GetRMS(2)*3;
				fit_ranges[1]=   pdl->GetMean(2)+pdl->GetRMS(2)*3;
				fit_ranges[2]=   pdl->GetMean(1)-pdl->GetRMS(1)*3;
				fit_ranges[3]=   pdl->GetMean(1)+pdl->GetRMS(1)*3;
				pdl->GetXaxis()->SetRangeUser( fit_ranges[2], fit_ranges[3] );
				pdl->GetYaxis()->SetRangeUser( fit_ranges[0], fit_ranges[1] );
			 }

			 double cosangle= atan(  pdl->GetRMS(2)/pdl->GetRMS(1) );
			 gauss->SetParameter( "area", pdl->GetEntries()) ;
			 gauss->SetParameter( "sigma_X", pdl->GetRMS(1) );
			 gauss->SetParameter( "sigma_Y", pdl->GetRMS(2) );
			 gauss->SetParameter( "angle", cosangle );
			 gauss->SetParameter( "X_MPV", pdl->GetMean(1) );
			 gauss->SetParameter( "Y_MPV", pdl->GetMean(2) );
			 gauss->SetRange( fit_ranges[2], fit_ranges[3],
									fit_ranges[0], fit_ranges[1] );

			 gauss->SetParLimits( 0, pdl->GetEntries()/2, pdl->GetEntries()*2 );
			 gauss->SetParLimits( 1, pdl->GetRMS(1)/2, pdl->GetRMS(1)*2 );
			 gauss->SetParLimits( 2, pdl->GetRMS(2)/2, pdl->GetRMS(2)*2 );
			 gauss->SetParLimits( 3, cosangle/2, cosangle*2 );
			 gauss->SetParLimits( 4, pdl->GetMean(1)-pdl->GetRMS(1)/2,
										 pdl->GetMean(1)+pdl->GetRMS(1)/2);
			 gauss->SetParLimits( 5, pdl->GetMean(2)-pdl->GetRMS(2)/2,
										 pdl->GetMean(2)+pdl->GetRMS(2)/2 );
			 pdl->Fit( gauss,"Q" );

			 //save results
			 m_calCorrPed[rg][lr][col][fc]= m_calPed[rg][lr][col][fc];
			 m_calCorrPed[rg][lr][col][fc]+= gauss->GetParameter("X_MPV");
			 m_calCorrPed[rg+1][lr][col][fc]= m_calPed[rg+1][lr][col][fc];
			 m_calCorrPed[rg+1][lr][col][fc]+= gauss->GetParameter("Y_MPV");
			 m_calCorrPedRms[rg][lr][col][fc]= gauss->GetParameter("sigma_X");
			 m_calCorrPedRms[rg+1][lr][col][fc]= gauss->GetParameter("sigma_Y");
			 m_calCorrPedCos[rg/2][lr][col][fc]=
				cos(gauss->GetParameter("angle"));
		  }
  return;
}

void muonCalib::PrintCalPed(const char* fileName){
  std::ofstream muped(fileName);
  for (int layer=0;layer < 8;layer++){
	 for(int col=0;col<12;col++){
		for(int side = 0;side <2;side++){

		  int nrng = (go_type == FILLPEDHIST4RANGES) ? 4 : 1;
		  for(int rng = 0;rng<nrng; rng++){

			 muped << " " << layer
					 << " " << col
					 << " " << side
					 << " " << rng
					 << " " << m_calPed[rng][layer][col][side]
					 << " " << m_calPedRms[rng][layer][col][side]
					 << std::endl;
		  }
		}
    }
  }
}

int muonCalib::PopulateAsymArray() {
  if (!ratfull) return -1;  // no histograms avaiable for load.

  for (int layer=0;layer < 8;layer++)
	 for(int col=0;col<12;col++){
		char ratfulname[]="raf000";
		int  histid= layer*12+col;
		sprintf(ratfulname,"raf%1d%02d",layer,col);

		TProfile* raf = (TProfile*)ratfull->At(histid);
		
		for (int ibin=2;ibin<12;ibin++){
		  float bin = (raf->GetBinContent(ibin));
		  m_asymTable[layer][col][ibin-2] = bin;
		}
	 }
  
  return 0;
}

void muonCalib::WriteAsymTable(const char* fileName){
  std::ofstream asym_table_out(fileName);
  for (int layer=0;layer < 8;layer++)
	 for(int col=0;col<12;col++){

		char ratfulname[]="raf000";
		int  histid= layer*12+col;
		sprintf(ratfulname,"raf%1d%02d",layer,col);
		TProfile* raf = (TProfile*)ratfull->At(histid);
		char scol[]="00";
		sprintf(scol," %2d",col);
		asym_table_out << " " << layer << " " << scol;

		for (int ibin=2;ibin<12;ibin++){
		  float bin = (raf->GetBinContent(ibin));
		  char sbin[]="  0.00000  ";
		  sprintf(sbin," %8.5f",bin);
		  asym_table_out << " " << sbin;
		  
		}
		asym_table_out << std::endl;

	 }
}

int muonCalib::WriteAsymXML(const char *fileName) {
  ofstream outFile(fileName);
  if (!outFile.is_open()) {
	 std::cout << "ERROR! unable to open txtFile='" << fileName << "'" << std::endl;
	 return -1;
  }

  outFile << "<?xml version=\"1.0\" ?>" << std::endl;
  outFile << "<!DOCTYPE calCalib SYSTEM \"$(CALIBUTILROOT)/xml/calCalib_v2r1.dtd\" [] >" << std::endl;
  outFile << "<calCalib>" << std::endl;
  outFile << "<generic instrument=\"EM\" timestamp=\"2003-10-1-12:56\" calibType=\"CAL_LightAsym\" fmtVersion=\"v3r3p2\">" << std::endl;
  outFile << "</generic>" << std::endl;
  outFile << "<dimension nRow=\"1\" nCol=\"1\" nLayer=\"8\" nXtal=\"12\" nFace=\"1\" nRange=\"2\" />" << std::endl;
  outFile << "<tower iRow=\"0\" iCol=\"0\">" << std::endl;

  for (int layer=0;layer <8;layer++){
	 outFile << "    <layer iLayer=\"" <<layer << "\">" << std::endl;
	 for(int col=0;col<12;col++){
		outFile << "      <xtal iXtal=\"" << col <<"\">" << std::endl;
		const std::string face("NA");
		outFile << "        <face end=\"" << face <<"\">" << std::endl;
		char* le = "LE";
		char* he = "HE";
		char* r[] = {le,he};
		for(int range=0; range <2; range++){
		  outFile <<"            <lightAsym diode=\"" <<r[range]
					 << "\" error=\"0.03\"" <<std::endl;
		  outFile << "              values=\"";
		  for (int i=0;i<10;i++) {
			 float bin = m_asymTable[layer][col][i];
			 outFile << " " << bin;
		  }
		  outFile << "\" />" << std::endl;
		}
		outFile << "        </face>" << std::endl;
		outFile << "       </xtal>" << std::endl;
	 }
	 outFile << "     </layer>" << std::endl;
  }
  outFile << "</tower>" << std::endl << "</calCalib>" << std::endl;

  outFile.close();
  return 0;
}

void muonCalib::ReadAsymTable(const char* fileName){
  std::ifstream asymin(fileName);
  while(1){
    double x[14];
    for(int i=0;i<14;i++)x[i]=i-6.5;
    double asym[10],y[14];
    int layer,col;
	 asymin >> layer
			  >> col;
	 for (int i=0;i<10;i++) asymin >> asym[i];
    if(!asymin.good()) break;

	 for(int i=0;i<10;i++) {
		y[i+2]=asym[i];   // -x[i+2]*m_calSlopes[layer][col];

		//LE & HE the same
		m_asymTable[layer][col][i] = asym[i];
	 }
	 std::cout <<" " << layer
				  <<" " << col
				  << std::endl;
	 //                for ( i=0;i<10;i++) std::cout << " " << asym[i]; std::cout << std::endl;
	 //                for ( i=0;i<10;i++) std::cout << " " << y[i+2]; std::cout << std::endl;

    y[1]=2*y[2]-y[3];
    y[0]=2*y[1]-y[2];
    y[12]=2*y[11]-y[10];
    y[13]=2*y[12]-y[11];

    if(y[11]<y[2]) for (int i=0;i<7;i++)
		{double xx=x[i];x[i]=x[13-i];x[13-i]=xx;
		  double yy=y[i];y[i]=y[13-i];y[13-i]=yy;}
    char splname[]="spl000";
    sprintf(splname,"spl%1d%02d",layer,col);

	 // we aren't always running in histogram mode :)
    if (asymCorr) asymCorr->AddAt(new TSpline3(splname,y,x,14),12*layer+col);
  }

}

void muonCalib::PrintCalCorrPed(const char* fileName){
  std::ofstream muped(fileName);
  for (int layer=0;layer < 8;layer++)
	 for(int col=0;col<12;col++)
		for(int side = 0;side <2;side++){
		  int nrng = (go_type == FILLCORRPEDHIST2RANGES) ? 4 : 1;
		  for(int rng = 0;rng<nrng; rng++){
			 muped << " " << layer
					 << " " << col
					 << " " << side
					 << " " << rng
					 << " " << m_calCorrPed[rng][layer][col][side]
					 << " " << m_calCorrPedRms[rng][layer][col][side];
			 if ( rng%2==0 )
				muped << " " << m_calCorrPedCos[rng%2][layer][col][side];
			 muped << std::endl;
		  }
		}
}

int muonCalib::ReadCorrPed(const char *fileName) {
  std::ifstream inFile(fileName);
  if (!inFile.is_open()) {
	 std::cout << "ERROR! unable to open txtFile='" << fileName << "'" << std::endl;
	 return -1;
  }

  while(1){
    float av,rms,cos; int layer,col,side, rng;
	 inFile >> layer
			  >> col
			  >> side
			  >> rng
			  >> av
			  >> rms;

	 cos = 0;
	 if (rng%2 == 0) 
		inFile >> cos;
	 
    if(!inFile.good()) break;
	 std::cout <<" " << layer
				  <<" " << col
				  <<" " << side
				  <<" " << rng
				  <<" " << av
				  <<" " << rms;
	 if (rng%2 == 0) 
		std::cout <<" " << cos;
	 std::cout << std::endl;

    m_calCorrPed[rng][layer][col][side] = av;
	 m_calCorrPedRms[rng][layer][col][side] = rms;
	 if (rng%2 == 0) m_calCorrPedCos[rng/2][layer][col][side] = cos;
  }

  return 0;
}

int muonCalib::WriteCorrPedXML(const char *fileName) {
  ofstream outFile(fileName);
  if (!outFile.is_open()) {
	 std::cout << "ERROR! unable to open txtFile='" << fileName << "'" << std::endl;
	 return -1;
  }

  outFile << "<?xml version=\"1.0\" ?>" << std::endl;
  outFile << "<!DOCTYPE calCalib SYSTEM \"$(CALIBUTILROOT)/xml/calCalib_v2r1.dtd\" [] >" << std::endl;
  outFile << "<calCalib>" << std::endl;
  outFile << "<generic instrument=\"EM\" timestamp=\"2003-10-1-12:56\" calibType=\"CAL_Ped\" fmtVersion=\"v3r3p2\">" << std::endl;
  outFile << "</generic>" << std::endl;
  outFile << "<dimension nRow=\"1\" nCol=\"1\" nLayer=\"8\" nXtal=\"12\" nFace=\"2\" />" << std::endl;
  outFile << "<tower iRow=\"0\" iCol=\"0\">" << std::endl;

  for (int layer=0;layer <8;layer++){
    outFile << "    <layer iLayer=\"" << layer << "\">" << std::endl;
    for(int col=0;col<12;col++){
      outFile << "      <xtal iXtal=\"" << col << "\">" << std::endl;
      for(int side = 0;side<2;side++){
		  const std::string face(FACE_MNEM[side]);
		  outFile << "        <face end=\"" << face << "\">" << std::endl;

		  // only use first range for now
		  int rng = 0;
		  float av       = m_calCorrPed[rng][layer][col][side];
		  float rms      = m_calCorrPedRms[rng][layer][col][side];
		  float cosAngle = m_calCorrPedCos[rng/2][layer][col][side];
		  for(int range=0; range <4; range++)
			 outFile << "            <calPed avg=\"" << av
						<< "\" sig=\""                  << rms
						<< "\" cos=\""                  << ((range%2) ? -1000.0 : cosAngle)
						<< "\" range=\""                << RNG_MNEM[range]
						<< "\" />" << std::endl;
		  outFile << "        </face>" << std::endl;
      }
      outFile << "       </xtal>" << std::endl;
    }
    outFile << "     </layer>" << std::endl;
  }
  outFile << "</tower>" << std::endl << "</calCalib>" << std::endl;

  outFile.close();
  return 0;
}

void muonCalib::ReadCalPed(const char* fileName){
  std::ifstream pedin(fileName);

  while(1){

    float av,rms; int layer,col,side, rng;
	 pedin >> layer
			 >> col
			 >> side
			 >> rng
			 >> av
			 >> rms;

    if(!pedin.good()) break;
	 std::cout <<" " << layer
				  <<" " << col
				  <<" " << side
				  << " " << rng
				  <<" " << av
				  <<" " << rms
				  << std::endl;
	 m_calPed[rng][layer][col][side] = av;
	 m_calPedRms[rng][layer][col][side] = rms;
  }
}

void muonCalib::ReadMuSlopes(const char* fileName){
  std::ifstream slopein(fileName);
  while(1){

    float slope; int layer,col;
	 slopein >> col
				>> layer
				>> slope;

    if(!slopein.good()) break;
	 std::cout <<" " << layer
				  <<" " << col
				  <<" " << slope
				  << std::endl;

	 m_calSlopes[layer][col] = 2*2.7844/slope;
  }
}

void muonCalib::ReadMuPeaks(const char* fileName){
  std::ifstream mupeaksin(fileName);

  while(1){

    float mupeak,sigma; int layer,col,side;
	 mupeaksin
		>> side
		>> col
		>> layer
		>> mupeak
		>> sigma;

    if(!mupeaksin.good()) break;
	 std::cout <<" " << layer
				  <<" " << col
				  <<" " << side
				  <<" " << mupeak
				  << std::endl;

	 m_calCorr[layer][col][side]    = 1000.0/mupeak;
	 m_muRelSigma[layer][col][side] = sigma;
  }
}

void muonCalib::HistDefine() {
  // Purpose and Method:  Setup Histograms

  gStyle->SetOptStat(111111);

  if (std::string(m_histFileName) == "") return;

  histFile = new TFile(m_histFileName,"RECREATE");
  DigiHistDefine();
}

void muonCalib::Go(Int_t numEvents)
{
  // Purpose and Method:  Event Loop
  //   All analysis goes here

  //  To read only selected branches - saves processing time
  //  Comment out any branches you are not interested in.

  // mc branches:
  if (m_mcChain) m_mcChain->SetBranchStatus("*", 0);    // disable all branches

  if (m_digiChain) {
	 m_digiChain->SetBranchStatus("*",0);  // disable all branches
	 // activate desired brances
	 m_digiChain->SetBranchStatus("m_cal*",1);
	 //        digiChain->SetBranchStatus("m_tkr*",1);
	 //        digiChain->SetBranchStatus("m_acd*",1);
	 m_digiChain->SetBranchStatus("m_eventId", 1);
	 m_digiChain->SetBranchStatus("m_runId", 1);
	 m_digiChain->SetBranchStatus("m_timeStamp", 1);
  }

  if (m_recChain) m_recChain->SetBranchStatus("*",0);  // disable all branches

  // determine how many events to process
  Int_t nentries = GetEntries();
  std::cout << "\nNum Events in File is: " << nentries << std::endl;
  Int_t curI;
  Int_t nMax = TMath::Min(numEvents+m_StartEvent,nentries);
  if (m_StartEvent == nentries) {
	 std::cout << " all events in file read" << std::endl;
	 return;
  }
  if (nentries <= 0) return;

  // Keep track of how many bytes we have read in from the data files
  Int_t nbytes = 0, nb = 0;

  // BEGINNING OF EVENT LOOP
  for (Int_t ievent=m_StartEvent; ievent<nMax; ievent++, curI=ievent) {
	 if (mc)  mc->Clear();
	 if (evt) evt->Clear();
	 if (rec) rec->Clear();

	 digiEventId = 0; reconEventId = 0; mcEventId = 0;
	 digiRunNum = 0; reconRunNum = 0; mcRunNum = 0;

	 nb = GetEvent(ievent);
	 nbytes += nb;

	 // Digi ONLY analysis
	 if (evt) {
		digiEventId = evt->getEventId();
		digiRunNum = evt->getRunId();
      if(digiEventId%1000 == 0)
		  std::cout <<" run " << digiRunNum << " event " << digiEventId << std::endl;

		//            DigiTkr();
		DigiCal();
		//            DigiAcd();
	 }

  }  // end analysis code in event loop

  // EVENT LOOP POST PROCESSING. i.e. not once per event but after all events.
  if (go_type == FILLMUHIST || go_type == FILLRATHIST) {
	 PopulateAsymArray();
  }

  m_StartEvent = curI;
}

void muonCalib::MakeHistList() {
  // Purpose and Method:  Make a THashList of histograms
  //   This avoids the need to refresh the histogram pointers

  if (!histFile) return;

  if (m_histList) delete m_histList;

  m_histList = new THashList(30, 5);

  TList* list = histFile->GetList();
  TIter iter(list);

  TObject* obj = 0;

  while ((obj=iter.Next())) {
	 m_histList->Add(obj);
  }
}

void muonCalib::HistClear() {
  // Purpose and Method:  Clear histograms by iterating over the THashList

  if (!m_histList) return;

  TIter iter(m_histList);

  TObject* obj = 0;

  while ((obj=(TObject*)iter.Next())) {
	 if (obj->InheritsFrom(TH1::Class())) ((TH1*)obj)->Reset();
  }
}

void muonCalib::ZeroPeds() {
  for (int lyr = 0; lyr < 8; lyr++){
	 for (int col = 0; col < 12; col++) {
		m_calSlopes[lyr][col] = 0.0;
		for (int side = 0; side < 2; side++) {
		  m_calCorr[lyr][col][side]=3.0;
		  m_muRelSigma[lyr][col][side]=0.0;
		  for (int i = 0; i < 4; i++) {
			 m_calPed[i][lyr][col][side]=0.0;
			 m_calPedRms[i][lyr][col][side]=0.0;
			 m_calCorrPed[i][lyr][col][side]=0.0;
			 m_calCorrPedRms[i][lyr][col][side]=0.0;
		  }
		  m_calCorrPedCos[0][lyr][col][side]=0.0;
		  m_calCorrPedCos[1][lyr][col][side]=0.0;
		}
	 }
  }
}

int muonCalib::WritePedXML(const char* fileName) {
  ofstream outFile(fileName);
  if (!outFile.is_open()) {
	 std::cout << "ERROR! unable to open txtFile='" << fileName << "'" << std::endl;
	 return -1;
  }

  outFile << "<?xml version=\"1.0\" ?>" << std::endl;
  outFile << "<!DOCTYPE calCalib SYSTEM \"$(CALIBUTILROOT)/xml/calCalib_v2r1.dtd\" [] >" << std::endl;
  outFile << "<calCalib>" << std::endl;
  outFile << "<generic instrument=\"EM\" timestamp=\"2003-10-1-12:56\" calibType=\"CAL_Ped\" fmtVersion=\"v3r3p2\">" << std::endl;
  outFile << "</generic>" << std::endl;
  outFile << "<dimension nRow=\"1\" nCol=\"1\" nLayer=\"8\" nXtal=\"12\" nFace=\"2\" />" << std::endl;
  outFile << "<tower iRow=\"0\" iCol=\"0\">" << std::endl;

  for (int layer=0;layer <8;layer++){
    outFile << "    <layer iLayer=\"" <<layer << "\">" << std::endl;
    for(int col=0;col<12;col++){
      outFile << "      <xtal iXtal=\"" <<col <<"\">" << std::endl;
      for(int side = 0;side<2;side++){
		  const std::string face(FACE_MNEM[side]);
		  outFile << "        <face end=\"" << face <<"\">" << std::endl;

		  for(int range=0; range <4; range++) {

			  // if only first range pedestals are available,
			  // they are used to fill pedestals for higher ranges

		  int rng = (go_type == FILLPEDHIST4RANGES) ? range : 0;			  
		  float av = m_calPed[rng][layer][col][side];
		  float rms = m_calPedRms[rng][layer][col][side];

			 outFile <<"            <calPed avg=\"" <<av
						<< "\" sig=\"" <<rms
						<< "\" range=\"" << RNG_MNEM[range] << "\" />"
						<< std::endl;
		  }
		  outFile << "        </face>" << std::endl;
      }
      outFile << "       </xtal>" << std::endl;
    }
    outFile << "     </layer>" << std::endl;
  }
  outFile << "</tower>" << std::endl << "</calCalib>" << std::endl;

  outFile.close();
  return 0;
}

/// Zeros out all member vars, does NOT free memory,for use in constructor
void muonCalib::ZeroMembers() {
  ZeroPeds();

  histFile        = 0;
  m_histFileName  = 0;
  pedhist         = 0;
  corrpedhist     = 0;
  pdahist         = 0;
  corrpdahist     = 0;
  rawhist         = 0;
  rawAdcHist      = 0;
  thrhist         = 0;
  adjhist         = 0;
  midhist         = 0;
  poshist         = 0;
  rathist         = 0;
  ratfull         = 0;
  asyhist         = 0;
  reshist         = 0;
  ratntup         = 0;
  asycalib        = 0;
  asymCorr        = 0;
  c1              = 0;
  gx              = 0;
  gy              = 0;
  xline           = 0;
  yline           = 0;
  glx             = 0;
  gly             = 0;
  xlongl          = 0;
  ylongl          = 0;
  land            = 0;
  m_histList      = 0;

  memset(digi_select, 0, sizeof(digi_select));
  memset(a,           0, sizeof(a));
  memset(ar,          0, sizeof(ar));
  memset(m_asymTable, 0, sizeof(m_asymTable));

  go_type           = FILLPEDHIST;
  asym_corr_type    = NONE;
}
