#include "muonCalib.h"

TObjArray tkrLayerHistArr;
UInt_t digiEventId, reconEventId, mcEventId;
UInt_t digiRunNum, reconRunNum, mcRunNum;

void muonCalib::McHistDefine() {

}

void muonCalib::DigiHistDefine() {
    // Purpose and Method:  Digitization histogram definitions

  TH1F* LTOT = new TH1F("LTOT","LTOT",10,0,10);
  TH1F* NTOT = new TH1F("NTOT","NTOT",100,0,100);
  TH1F* LHOLES = new TH1F("LHOLES","LHOLES",10,0,10);
  TH1F* MAXNL = new TH1F("MAXNL","MAXNL",10,0,10);
  TH1F* TX = new TH1F("TX","TX",50,-1.0,1.0);
  TH1F* TY = new TH1F("TY","TY",50,-1.0,1.0);

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
        std::cout << " " << layer <<" " << col 
        <<" "<<side  << " " << std::endl;
  char pedhisname[]="ped0000";
  char corrpedhisname[]="corrped0000";
  char thrhisname[]="thr0000";
  char adjhisname[]="adj0000";
  char midhisname[]="mid0000";
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
  pedhist->AddAt(new TH1F(pedhisname,pedhisname,300,0,600),histid);
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
    pdahist->AddAt(new TH1F(pdahisname,pdahisname,300,0,600),histid*4+rng);    
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

void muonCalib::ReconHistDefine() 
{
}


void muonCalib::McData() 
{
}

void muonCalib::DigiTkr() 
{
}

void muonCalib::DigiCal() 
{
  // Purpose and Method:  Process on CAL digi event

  float thresh = 300; 
  float adctot; double timestamp,dtime;
  timestamp = evt->getTimeStamp();
  dtime=timestamp-prevTimeStamp;
  float event = evt->getEventId();  

  prevTimeStamp = timestamp;
  adctot=0;
  const TObjArray* calDigiCol = evt->getCalDigiCol();
  if (!calDigiCol) return;

  int nCalDigi = calDigiCol->GetEntries();

  CalDigi *cdig = 0;

  for( int l=0; l<8; l++)
    for (int c=0;c<12;c++)
      for(int s=0;s<2;s++){ 
  a[l][c][s]=0;
  ar[l][c][s]=0;
      }
    
  for( int cde_nb=0; cdig=(CalDigi*) calDigiCol->At(cde_nb); cde_nb++ ){
    const CalXtalReadout* cRo=cdig->getXtalReadout(0);
        
    CalXtalId id = cdig->getPackedId();
    int layer = id.getLayer();
    int tower = id.getTower();
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
        ((TH2F*)corrpedhist->At(histid))->Fill(adcM-
                                               m_calPed[0][layer][col][0]);
        ((TH2F*)corrpedhist->At(histid+1))->Fill(adcP-
                                               m_calPed[0][layer][col][1]);
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
                       
        ((TNtuple*)ratntup->At((12*layer+col)*2))->Fill(aarM);    
        ((TNtuple*)ratntup->At((12*layer+col)*2+1))->Fill(aarP);              
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

      // m_calCorr are initially set as 1
      // during FILLRATHIST (light asymmetry calibration), m_calCorr 
                
      a[layer][col][0] = (adcM - m_calPed[range][layer][col][0])*m_calCorr[layer][col][0];
      a[layer][col][1] = (adcP - m_calPed[range][layer][col][1])*m_calCorr[layer][col][1];
      ar[layer][col][0] = (adcM - m_calPed[range][layer][col][0]);      
      ar[layer][col][1] = (adcP - m_calPed[range][layer][col][1]);    
        
    }            

  }   // while
              
  adctot=0;
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
    float ratio = a[l][c][1]/(a[l][c][0]);

    (nl[l])++; if(nl[l]>maxnl) maxnl=nl[l];
    switch (l%2) {

    case 0 : 

      if(nx<nmax){
        xc[nx]=c; xl[nx]=l;nx++; 
        gx->Set(nx); gx->SetPoint(nx-1,l,c);
      }
 
      if(go_type == FILLMUHIST && ratio > 0){
        float longpos = 5.5 - (log(ratio)/m_calSlopes[l][c]);
      }

      if(nl[l]>maxnlx) maxnlx=nl[l];  

      break;

    case 1 : 
      if(ny<nmax){
        yc[ny]=c; yl[ny]=l;ny++; 
        gy->Set(ny); gy->SetPoint(ny-1,l,c);
      }

      if(go_type == FILLMUHIST && ratio > 0){
        float longpos = 5.5-(log(ratio)/m_calSlopes[l][c]);
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

    ((TH1F*)GetObjectPtr("LTOT"))->Fill(ltot);
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

    float thrsel=200;    
    for(int l=0; l<8; l++){
      for(int c=0; c<12; c++){

  /*
  int ltest = l<7 ? l+1 : l-1;

  for (int side=0;side<2;side++){

    // codes for other tests, not used in muon calibration
    int csel = 2+side*7;
    int sel = (ar[ltest][csel][0]+ar[ltest][csel][1])>thrsel;
    int noleft = (ar[ltest][csel-1][0]+ar[ltest][csel-1][1])<thrsel;
    int noright = (ar[ltest][csel+1][0]+ar[ltest][csel+1][1])<thrsel;
    if( sel && noleft && noright && ar[l][c][1]+ar[l][c][0]>thrsel) 
      ((TH1F*)asycalib->At((12*l+c)*2+side))->Fill((ar[l][c][1]-ar[l][c][0])/(ar[l][c][1]+ar[l][c][0]));

  }
  */

  digi_select[l][c]=0;

  float posx = l*tx+x0; float posy=l*ty+y0;
  float poslx = l*tlx+xl0; float posly=l*tly+yl0;
  float pos=(l%2) ? posx:posy;
  float posl=(l%2) ? poslx:posly;

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
      ((TProfile*)ratfull->At(12*l+c))->Fill((pos-5.5)*2.7844,log(ratio)+m_calSlopes[l][c]*(pos-5.5));            
              
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

        // measure resolution
        if(go_type == FILLMUHIST) {
    float longpos = 5.5 - (log(ratio)/m_calSlopes[l][c]);
    ((TH1F*)reshist->At(12*l+c))->Fill(2.7844*(longpos-posl));
        }

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
          " logratio =" <<logratio << " slope=" << slope << endl;
    m_calCorr[layer][col][0] *= exp(logratio/2);
    m_calCorr[layer][col][1] *= exp(-logratio/2);
    m_calSlopes[layer][col] = -slope;
   }
  }

}

void muonCalib::FitMuHist(){
  TF1 *lognormal= new TF1("lognormal", logNormal_fcn, 0, 1, 4 );
  for ( short int layer=0;layer < 8;layer++){
   for( short int col=0;col<12;col++){
     for( short int side=0;side<2;side++){
       int histid = (layer*12+col)*2+side;
       TH1F* h = (TH1F*)thrhist->At(histid);
       for( short int ll=0; ll<5; ll++ )
         h->SetAxisRange( h->GetMean() - 2* h->GetRMS(),
                        h->GetMean() + 3* h->GetRMS() );
       float ave = h->GetMean();
       float rms = h->GetRMS();
     
       lognormal->SetParameter( 0, h->GetEntries() );
       lognormal->SetParameter( 1, ave );
       lognormal->SetParameter( 2, 2*rms );
       lognormal->SetParameter( 3,  .5 );
       lognormal->SetParLimits( 2, rms/9, 4*rms );
       lognormal->SetParLimits( 1, ave-rms, ave+rms*2 );
       h->Fit( "lognormal", "R" );
       ave= lognormal->GetParameters(1);
       rms= lognormal->GetParameter(2);
  
       std::cout << " mean=" << ave << "  sigma=" << rms << endl;
       // note *= here, gain correction applied
       m_calCorr[layer][col][side] *=1000/ave;
       m_muRelSigma[layer][col][side] = rms/lognormal->Eval(ave);
     }
   }
  }
}

Double_t logNormal_fcn( Double_t *x, Double_t *par ){//{{{2
  if( *(par+3)==0 ){
    Double_t val= ((*x) - (*(par+1)))/ (*(par+2));
    return (*par)*exp( -0.5*val*val );
  }
  Double_t val= 1.17741002251547466 * (*(par+3)); // number is sqrt(log(4))
  Double_t val2= sinh(val)/(2.36*val*(*(par+2)));
  val2*= ((*x) - (*(par+1)))* (*(par+3));
  if (val2<-0.9999) return 0.;
  val= log( 1 + val2 )/(*(par+3)); 
  val2 = -0.5*val * val + (*(par+3))*(*(par+3));
  return (*par)*TMath::Exp(val2);
}//}}}2  

//using namespace std;
//#include "fstream.h"
#include <fstream>

void muonCalib::WriteMuPeaks(const char* fileName){
    std::ofstream mpout(fileName); 
  for (int layer=0;layer < 8;layer++){
     for(int col=0;col<12;col++){
     for(int side=0;side<2;side++){
    mpout << " " << side << " " << col << " " << layer <<" " 

      // note compared to fitMuHist, gain corection is removed here
        << 1000/m_calCorr[layer][col][side] << " "
                          << m_muRelSigma[layer][col][side]
                          << endl;

     }
   }
  }
}
void muonCalib::WriteMuSlopes(const char* fileName){
    std::ofstream mslout(fileName); 
  for (int layer=0;layer < 8;layer++){
     for(int col=0;col<12;col++){
    mslout << " " << col << " " << layer <<" " 
        << 2*2.7844/m_calSlopes[layer][col] << endl;     
   }
  }
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
          for( int iter=0; iter<3; iter++ ){
            h->SetAxisRange(av-3*rms,av+3*rms);
            av = h->GetMean(); rms = h->GetRMS();
          }
          int fitresult= h->Fit("gaus", "","", av-3*rms, av+3*rms );
          cout<<h->GetName()<<"  "<<fitresult<<endl;
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
    char name[]="corrpda00000";
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

						cout<<pdl->GetEntries();
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
            pdl->Fit( gauss, "" );
          
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
                       << endl;
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
                       << endl;

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
                       << endl;

 m_calCorr[layer][col][side] = 1000.0/mupeak;  
    }            
}

void muonCalib::DigiAcd() {
    // Purpose and Method:  Process on ACD digi event
    //   Determines the total energy deposited and
    //   store the PHA for tile 0000 in a histogram

    const TObjArray* acdDigiCol = evt->getAcdDigiCol();
    if (!acdDigiCol) return;

    Double_t totE = 0.0;
    UShort_t pha0 = 0;
    // Create an id for tile 0000
    AcdId id41(0, 0, 4, 1);

    TIter acdDigiIter(acdDigiCol);
    AcdDigi *acdDigiItem = 0;
    
    while (acdDigiItem = (AcdDigi*)acdDigiIter.Next()) {
        totE = acdDigiItem->getEnergy();
        AcdId id = acdDigiItem->getId();
        if (id.getId() == id41.getId()) {
            pha0 = acdDigiItem->getPulseHeight(AcdDigi::A) + 
                acdDigiItem->getPulseHeight(AcdDigi::B);
        }
    }

    ((TH1F*)GetObjectPtr("ACDPHATILE41"))->Fill(pha0);
    ((TH1F*)GetObjectPtr("ACDTOTE"))->Fill(totE);

    return;
}


void muonCalib::ReconTkr() {
    // Purpose and Method:  Process one TkrRecon event
    
    TkrRecon *tkrRec = rec->getTkrRecon();
    
    // If no TRKRECON data is available then return
    if (!tkrRec)  return;

    ((TH1F*)GetObjectPtr("TKRNUMFITTRACKS"))->Fill(tkrRec->getTrackCol()->GetEntries());

    const TObjArray *trackCol = tkrRec->getTrackCol();
    TIter trackIter(trackCol);
    TkrTrack *track = 0;
    
    while (track = (TkrTrack*)trackIter.Next()) {
        ((TH1F*)GetObjectPtr("TKRNUMHITSPERTRACK"))->Fill(track->getNumHits());
    }
}

void muonCalib::ReconCal() {
    // Purpose and Method:  Process on CalRecon event
    
    CalRecon *calRec = rec->getCalRecon();
    
    if (!calRec) return;
    
    
    return;
}

void muonCalib::ReconAcd() {
    // Purpose and Method:  Processes ACD reconstruction data

    AcdRecon *acdRec = rec->getAcdRecon();
    if (!acdRec) return;

    ((TH1F*)GetObjectPtr("ACDDOCA"))->Fill(acdRec->getDoca());
    ((TH1F*)GetObjectPtr("ACDACTDIST"))->Fill(acdRec->getActiveDist());

    return;
}

void muonCalib::HistDefine() {
    // Purpose and Method:  Setup Histograms
    
    gStyle->SetOptStat(111111);
    
    histFile = new TFile(m_histFileName,"RECREATE");
    
    McHistDefine();
    DigiHistDefine();
    ReconHistDefine();
}

void muonCalib::muSlopestoXML(){
    std::ofstream xml( "mSlopes.xml" );
    short int rg, layer, column, face;
    
    //header
    xml<<"<?xml version=\"1.0\" ?>"<<endl<<endl;
    xml<<"<!-- $Header: my Pdl-->"<<endl<<endl;
    xml<<"<!-- Approximately real  CAL muon slope XML file for EM, according to calCalibv3.dtd -->"<<endl<<endl;

    xml<<"<!DOCTYPE calCalib SYSTEM \"../calCalib_v2.dtd\" [] >"<<endl<<endl;

    //calCalib
    xml<<"<calCalib>"<<endl
      <<"\t<generic\tinstrument=\"EM\" timestamp=\"2003-11-2-12:56\""<<endl
      <<"\t\t\t\tcalibType=\"CAL_MuSlope\" fmtVersion=\"v2r0\">"<<endl<<endl
      <<"\t\t<inputSample\tstartTime=\"2003-2-21-05:49:12\""
      <<" stopTime=\"2003-2-24-07:07:02\""<<endl
      <<"\t\t\t\t\ttriggers=\"random\" mode=\"normal\" source=\"stuff\" >"<<endl
      <<endl;
    
    xml<<"\t\tTimes are start and stop time of calibration run."<<endl
      <<"\t\tOther attributes are just made up for code testing."<<endl;    
    
    xml<<"\t\t</inputSample>"<<endl
      <<"\t</generic>"<<endl<<endl<<endl;
    
    xml<<"<!-- EM instrument: 8 layers, 12 columns -->"<<endl<<endl
      <<"\t<dimension nRow=\"1\" nCol=\"1\" "
      <<"nLayer=\"8\" nXtal=\"12\" nFace=\"1\" />"<<endl<<endl;
    
    xml<<"\t<tower iRow=\"0\" iCol=\"0\">"<<endl;
    
    for( layer=0; layer<8; layer++ ){
      xml<<"\t\t<layer iLayer=\""<<layer<<"\">"<<endl;
      
      for( column=0; column<12; column++ ){
        xml<<"\t\t\t<xtal iXtal=\""<<column<<"\">"<<endl;
        xml<<"\t\t\t\t<face end=\"NA\">"<<endl;
        tree->GetEntry( layer*24 + column*2 + face );
        for( rg=0; rg<4; rg++ ){
          xml<<"\t\t\t\t\t<muSlope slope=\""<<slope<<"\" range=\"";
          xml<<((rg<2)?"L":"H")<<"EX"<<((rg%2==0)?"8":"1")<<"\" />"<<endl;
        }
        xml<<"\t\t\t\t</face>"<<endl;  
        xml<<"\t\t\t</xtal>"<<endl;
      }
      xml<<"\t\t</layer>"<<endl;
    }
    
    //end file
    xml<<"\t</tower>"<<endl;
    xml<<"</calCalib>"<<endl;
    xml.close();
}


void muonCalib::Go(Int_t numEvents)
{    
    // Purpose and Method:  Event Loop
    //   All analysis goes here
    
    //  To read only selected branches - saves processing time
    //  Comment out any branches you are not interested in.
    
    // mc branches:
    if (mcTree) {
        mcTree->SetBranchStatus("*", 0);    // disable all branches
        // Activate desired branches...
        mcTree->SetBranchStatus("m_eventId", 1);
//        mcTree->SetBranchStatus("m_particleCol", 1);
        mcTree->SetBranchStatus("m_runId", 1);        
//        mcTree->SetBranchStatus("m_integratingHitCol", 1);        
//        mcTree->SetBranchStatus("m_positionHitCol", 1);        
    }
    
    if (digiTree) {
        digiTree->SetBranchStatus("*",0);  // disable all branches
        // activate desired brances
        digiTree->SetBranchStatus("m_cal*",1);  
//        digiTree->SetBranchStatus("m_tkr*",1);  
//        digiTree->SetBranchStatus("m_acd*",1);
        digiTree->SetBranchStatus("m_eventId", 1); 
        digiTree->SetBranchStatus("m_runId", 1);
        digiTree->SetBranchStatus("m_timeStamp", 1);
    }
    
    if (reconTree) {
        reconTree->SetBranchStatus("*",0);  // disable all branches
        // activate desired branches
//        reconTree->SetBranchStatus("m_cal", 1);  
//        reconTree->SetBranchStatus("m_tkr", 1);
//        reconTree->SetBranchStatus("m_acd", 1);
    }
        
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
//        if (mc) mc->Clear();
        if (evt) evt->Clear();
        if (rec) rec->Clear();
        
        digiEventId = 0; reconEventId = 0; mcEventId = 0;
        digiRunNum = 0; reconRunNum = 0; mcRunNum = 0;
        
        nb = GetEvent(ievent);
        nbytes += nb;
/*        
        // Monte Carlo ONLY analysis
        if (mc) {  // if we have mc data process it
            mcEventId = mc->getEventId();
            mcRunNum = mc->getRunId();
            McData();
        } 
*/        
        // Digi ONLY analysis
        if (evt) {
            digiEventId = evt->getEventId(); 
            digiRunNum = evt->getRunId();
      if(digiEventId%1000 == 0) 
                std::cout <<" run " << digiRunNum << " event " << digiEventId << endl;
            
//            DigiTkr();
            DigiCal();
//            DigiAcd();
        }
        
/*        
        // RECON ONLY analysis
        if (rec) {  // if we have recon data proccess it            
            ReconTkr();
            ReconCal();
            ReconAcd();
        } 
*/        
        
    }  // end analysis code in event loop
    
    m_StartEvent = curI;
}

