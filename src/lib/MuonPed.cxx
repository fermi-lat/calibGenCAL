// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Attic/MuonPed.cxx,v 1.11.12.1 2007/05/29 16:59:57 fewtrell Exp $
/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "RootFileAnalysis.h"
#include "MuonPed.h"
#include "CalPed.h"
#include "CGCUtil.h"

// GLAST INCLUDES
#include "digiRootData/Gem.h"
#include "digiRootData/DigiEvent.h"
#include "CalUtil/CalArray.h"

// EXTLIB INCLUDES
#include "TH1S.h"
#include "TF1.h"

// STD INCLUDES
#include <sstream>
#include <cmath>

using namespace CGCUtil;
using namespace CalUtil;
using namespace std;

MuonPed::MuonPed(ostream &ostrm,float tsl,int ntsl) :
  m_ostrm(ostrm),
  time_slice(tsl),
  n_time_slices(ntsl),
  noisy_channel(FaceIdx::N_VALS),
  m_histograms(RngIdx::N_VALS)
{
}

void MuonPed::initHists() {
  for (RngIdx rngIdx; rngIdx.isValid(); rngIdx++) {
    if (m_histograms[rngIdx] == 0) {
      string histname = genHistName(rngIdx);
      
      m_histograms[rngIdx] = new TH1S(histname.c_str(),
                                      histname.c_str(),
                                      1000,0.5,1000.5);
    }
  }

  if(time_slice>0){
   for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++) noisy_channel[faceIdx] = false;

   noisy_channel[FaceIdx(4,3,4,POS_FACE)]=true;
   noisy_channel[FaceIdx(6,4,1,POS_FACE)]=true;
   noisy_channel[FaceIdx(8,6,2,NEG_FACE)]=true;

   pr_ped_lat = new TProfile("prpedlat","prpedlat",n_time_slices,0,n_time_slices*time_slice,-2,2);
   hcallo = new TH1F("hcallo","CAL_LO rate per time slice",n_time_slices,0,n_time_slices*time_slice);
   hcalhi = new TH1F("hcalhi","CAL_HI rate per time slice",n_time_slices,0,n_time_slices*time_slice);
   hhex8rate = new TH1F("hhex8rate","HEX8 rate per time slice",n_time_slices,0,n_time_slices*time_slice);
   cout << " hhex8rate histogram created " << endl;
  
   for(int twr=0;twr<16;twr++){
    ostringstream prpedtwrname;
    prpedtwrname << "prpedtwr_T" << twr;
    pr_ped_twr[twr]=new TProfile(prpedtwrname.str().c_str(),prpedtwrname.str().c_str(),n_time_slices,0,n_time_slices*time_slice,-10,10);
   }

   for(int twr=0;twr<16;twr++)
    for(int lyr=0;lyr<8;lyr++)
      for(int face=0;face<2;face++){
	for(int col=0;col<12;col++){
	  ostringstream prpedcorname;
	  ostringstream prpedname;
	  prpedname << "prped_T" << twr  << "_L" << lyr << "_C" << col << "_F" << face;
	  prpedcorname << "prpedcor_T" << twr  << "_L" << lyr << "_C" << col << "_F" << face;
	  pr_ped_chan[twr][lyr][col][face]=new TProfile(prpedname.str().c_str(),prpedname.str().c_str(),n_time_slices,0,n_time_slices*time_slice,-50,50);
	}
      }
  }
}

void MuonPed::fillHists(unsigned nEntries, 
                        const vector<string> &rootFileList, 
                        const CalPed *roughPeds,
                        TRIGGER_CUT trigCut) {

  /////////////////////////////////////////
  /// Initialize Object Data //////////////
  /////////////////////////////////////////
  initHists();

  algData.roughPeds = roughPeds;
  algData.trigCut = trigCut;
  

  /////////////////////////////////////////
  /// Open ROOT Event File  ///////////////
  /////////////////////////////////////////
  RootFileAnalysis rootFile(0, &rootFileList, 0);

  // enable only needed branches in root file
  rootFile.getDigiChain()->SetBranchStatus("*",0);
  rootFile.getDigiChain()->SetBranchStatus("m_calDigiCloneCol");
  if (algData.trigCut == PERIODIC_TRIGGER)
    rootFile.getDigiChain()->SetBranchStatus("m_summary");
  if (algData.trigCut == PERIODIC_TRIGGER || algData.trigCut == EXTERNAL_TRIGGER)
    rootFile.getDigiChain()->SetBranchStatus("m_gem");
    rootFile.getDigiChain()->SetBranchStatus("m_metaEvent");


  unsigned nEvents = rootFile.getEntries();
  m_ostrm << __FILE__ << ": Processing: " << nEvents << " events." << endl;

  /////////////////////////////////////////
  /// Event Loop //////////////////////////
  /////////////////////////////////////////
  // in periodic trigger mode we will skip these events
  eventData.init();
  
  time_sec =0;
  time_sec0=0;

  for (eventData.eventNum = 0; eventData.eventNum < nEvents; eventData.next()) {
    /////////////////////////////////////////
    /// Load new event //////////////////////
    /////////////////////////////////////////
    if (eventData.eventNum % 2000 == 0) {
      // quit if we have enough entries in each histogram
      unsigned currentMin = getMinEntries();
      if (currentMin >= nEntries && time_slice<0) break;
      m_ostrm << "Event: " << eventData.eventNum 
              << " min entries per histogram: " << currentMin
              << endl;
      m_ostrm.flush();
    }

    if (!rootFile.getEvent(eventData.eventNum)) {
      m_ostrm << "Warning, event " << eventData.eventNum << " not read." << endl;
      continue;
    }
    
    DigiEvent *digiEvent = rootFile.getDigiEvent();
    if (!digiEvent) {
      m_ostrm << __FILE__ << ": Unable to read DigiEvent " << eventData.eventNum  << endl;
      continue;
    }

    processEvent(*digiEvent);
  }
}

void MuonPed::fitHists(CalPed &calPed) {
  for (RngIdx rngIdx; rngIdx.isValid(); rngIdx++) {
    // skip absent histograms
    if (!m_histograms[rngIdx])
      continue;

    // select histogram from list
    TH1S &h= *m_histograms[rngIdx];

    // skip empty histograms
    if (h.GetEntries() == 0) 
      continue;

    // trim outliers
    float av = h.GetMean();float err =h.GetRMS();
    for( unsigned short iter=0; iter<3;iter++ ) {
      h.SetAxisRange(av-3*err,av+3*err);
      av = h.GetMean(); err= h.GetRMS();
    }
    
    // gaussian fit
    h.Fit("gaus", "Q","", av-3*err, av+3*err );
    h.SetAxisRange(av-150,av+150);

    // assign values to permanent arrays
    calPed.setPed(rngIdx, 
                  ((TF1&)*h.GetFunction("gaus")).GetParameter(1));
    calPed.setPedSig(rngIdx,
                     ((TF1&)*h.GetFunction("gaus")).GetParameter(2));
  }
}

void MuonPed::loadHists(const string &filename) {
  TFile histFile(filename.c_str(), "READ");

  for (RngIdx rngIdx; rngIdx.isValid(); rngIdx++) {
    string histname = genHistName(rngIdx);
    
    TH1S *hist_ptr = retrieveHist<TH1S>(histFile, histname);
    if (!hist_ptr)
      continue;

    // move histogram into Global ROOT memory
    // so it is not deleted when input file is closed.
    // this may be a memory leak, i don't think
    // anyone cares.
    hist_ptr->SetDirectory(0);

    m_histograms[rngIdx] = hist_ptr;
  }
}


string MuonPed::genHistName(RngIdx rngIdx) {
  ostringstream tmp;
  tmp << "muonpeds_" << rngIdx.val();
  return tmp.str();
}

unsigned MuonPed::getMinEntries() {
  unsigned retVal = ULONG_MAX;

  for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
    RngIdx rngIdx(xtalIdx, POS_FACE, LEX8);
    unsigned nEntries = (unsigned)m_histograms[rngIdx]->GetEntries();
    
    // only count histograms that have been filled 
    // (some histograms will never be filled if we are
    // not using all 16 towers)
    if (nEntries != 0) {
      retVal = min(retVal,nEntries);
    }
  }

  // case where there are no fills at all
  if (retVal == ULONG_MAX)
    return 0;

  return retVal;
}

void MuonPed::processEvent(DigiEvent &digiEvent) {

  /////////////////////////////////////////
  /// Event/Trigger level cuts ////////////
  /////////////////////////////////////////

      time_sec = (digiEvent.getMetaEvent()).time().current().timeSecs();
      if(time_sec0==0.0) time_sec0=time_sec;


  //-- retrieve trigger data
  const Gem *gem = 0;
  unsigned gemConditionsWord = 0;
  if (algData.trigCut == PERIODIC_TRIGGER ||
      algData.trigCut == EXTERNAL_TRIGGER) {
    gem = &(digiEvent.getGem());
    if (&gem==0) {
      m_ostrm << "Warning, gem data not found for event: "
              << eventData.eventNum << endl;
      return;
    }
    gemConditionsWord = gem->getConditionSummary();
  }
  if(time_slice>0){
      if(gemConditionsWord & 4)hcallo->Fill(time_sec-time_sec0);
      if(gemConditionsWord & 8)hcalhi->Fill(time_sec-time_sec0);
  }


  const TClonesArray* calDigiCol = digiEvent.getCalDigiCol();
  if (!calDigiCol) {
    m_ostrm << "no calDigiCol found for event#" << eventData.eventNum << endl;
    return;
  }

  TIter calDigiIter1(calDigiCol);
  const CalDigi *pCalDigi = 0;
 if(time_slice>0){  
   while ((pCalDigi = (CalDigi*)calDigiIter1.Next())){
    for (FaceNum face; face.isValid(); face++){
      if(2 == pCalDigi->getRange(0,(CalXtalId::XtalFace)face.val()))hhex8rate->Fill(time_sec-time_sec0);
    }
   }
 }

  //-- PERIODIC_TRIGGER CUT
  if (algData.trigCut == PERIODIC_TRIGGER) {
    // quick check if we are in 4-range mode
    EventSummaryData& summary = digiEvent.getEventSummaryData();
    if (&summary == 0) {
      m_ostrm << "Warning, eventSummary data not found for event: "
              << eventData.eventNum << endl;
      return;
    }
    eventData.fourRange = summary.readout4();

    float gemDeltaEventTime = gem->getDeltaEventTime()*0.05;
    if(gemConditionsWord != 32 ||  // skip unless we are periodic trigger only
       eventData.prev4Range      ||  // avoid bias from 4 range readout in prev event
       gemDeltaEventTime < 500) {   // avoid bias from shaped readout noise from adjacent event
      return;
    }
  }

  //-- EXTERNAL_TRIGGER CUT
  if (algData.trigCut == EXTERNAL_TRIGGER) {
    if (gemConditionsWord != 128)
      return;
  }
  /*
  const TClonesArray* calDigiCol = digiEvent.getCalDigiCol();
  if (!calDigiCol) {
    m_ostrm << "no calDigiCol found for event#" << eventData.eventNum << endl;
    return;
  }
  */
    sumlat=0.0;
    for(int it=0;it<16;it++){
      sumtwr[it]=0.0;
    }

  TIter calDigiIter(calDigiCol);
   pCalDigi = 0;

  /////////////////////////////////////////
  /// Xtal Hit Loop ///////////////////////
  /////////////////////////////////////////
  while ((pCalDigi = (CalDigi*)calDigiIter.Next()))
    processHit(*pCalDigi);

  if(time_slice>0){
	   for(int twr=0;twr<16;twr++){
	     pr_ped_twr[twr]->Fill(double(time_sec-time_sec0),double(sumtwr[twr]/192.0)); 
	   }
	  pr_ped_lat->Fill(double(time_sec-time_sec0),double(sumlat/3072.0)); 
  }

}

void MuonPed::processHit(const CalDigi &calDigi) {
  //-- XtalId --//
  idents::CalXtalId id(calDigi.getPackedId()); // get interaction information
  // skip hits not for current tower.
  XtalIdx xtalIdx(id);

  unsigned nRO = calDigi.getNumReadouts();
  /*
  if (nRO != 4) {
    ostringstream tmp;
    tmp << __FILE__  << ":"     << __LINE__ << " " 
        << "Event# " << eventData.eventNum << " Invalid nReadouts, expecting 4";
    throw runtime_error(tmp.str());
  }
  */

  // 1st look at LEX8 vals
  CalArray<FaceNum, float> adcL8;
  for (FaceNum face; face.isValid(); face++) {
    adcL8[face] = calDigi.getAdcSelectedRange(LEX8, (CalXtalId::XtalFace)face.val());
    // check for missing readout
    if (adcL8[face] < 0) {
      m_ostrm << "Couldn't get LEX8 readout for event=" << eventData.eventNum << endl;
      return;
    }
  }

  /////////////////////////////////////////
  /// 'Rough' pedestal mode (fist pass) ///
  /////////////////////////////////////////
  if (!algData.roughPeds) {
    for (FaceNum face; face.isValid(); face++) {
      RngIdx rngIdx(xtalIdx, face, LEX8);
          
      m_histograms[rngIdx]->Fill(adcL8[face]);
    }
  } else  {
    /////////////////////////////////////////
    /// Cut outliers (2nd pass) /////////////
    /////////////////////////////////////////

  if(time_slice>0){

	int twr = xtalIdx.getTwr();
	int col = xtalIdx.getCol();
	int lyr = xtalIdx.getLyr();
	int xy = lyr%2;
	double dped0 = adcL8[POS_FACE] - algData.roughPeds->getPed(RngIdx(xtalIdx,POS_FACE,LEX8));
	double dped1 = adcL8[NEG_FACE] - algData.roughPeds->getPed(RngIdx(xtalIdx,NEG_FACE,LEX8));					
	if( !noisy_channel[FaceIdx(xtalIdx,POS_FACE)]){
	  sumlat += dped0;
	  sumtwr[twr] += dped0;
	  (pr_ped_chan[twr][lyr][col][0])->Fill(double(time_sec-time_sec0),double(dped0));
	}

	if( !noisy_channel[FaceIdx(xtalIdx,NEG_FACE)]){
	  sumlat += dped1;
	  sumtwr[twr] += dped1;
	  (pr_ped_chan[twr][lyr][col][1])->Fill(double(time_sec-time_sec0),double(dped1));
	}
  }

    // skip outliers (outside of 5 sigma on either side)
    if (fabs(adcL8[NEG_FACE] - algData.roughPeds->getPed(RngIdx(xtalIdx,NEG_FACE,LEX8))) >=
        5*algData.roughPeds->getPedSig(RngIdx(xtalIdx,NEG_FACE,LEX8)) ||
        fabs(adcL8[POS_FACE] - algData.roughPeds->getPed(RngIdx(xtalIdx,POS_FACE,LEX8))) >=
        5*algData.roughPeds->getPedSig(RngIdx(xtalIdx,POS_FACE,LEX8)))
      return;
      
    //-- Fill histograms for all 4 ranges
    for (unsigned short n = 0; n < nRO; n++) {
      const CalXtalReadout &readout = *calDigi.getXtalReadout(n);
          
      for (FaceNum face; face.isValid(); face++) {
        // check that we are in the expected readout mode
        RngNum rng = readout.getRange((CalXtalId::XtalFace)face.val());
        unsigned short adc = readout.getAdc((CalXtalId::XtalFace)face.val());
        RngIdx rngIdx(xtalIdx, face, rng);
        m_histograms[rngIdx]->Fill(adc);
      }
    }

  }
}

void MuonPed::trimHists() {
  for (RngIdx rngIdx; rngIdx.isValid(); rngIdx++) 
    if (m_histograms[rngIdx])
      if (!m_histograms[rngIdx]->GetEntries()) {
        delete m_histograms[rngIdx];
        m_histograms[rngIdx] = 0;
      }

}
