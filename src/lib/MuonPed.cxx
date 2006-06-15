// $Header$
/** @file
    @author Zachary Fewtrell
 */

// LOCAL INCLUDES
#include "RootFileAnalysis.h"
#include "MuonPed.h"
#include "CGCUtil.h"

// GLAST INCLUDES

// EXTLIB INCLUDES
#include "TF1.h"
#include "TKey.h"

// STD INCLUDES
#include <sstream>
#include <ostream>

using namespace CGCUtil;

MuonPed::MuonPed(ostream &ostrm) :
  m_peds(RngIdx::N_VALS, INVALID_PED),
  m_pedSig(RngIdx::N_VALS, 0),
  m_ostrm(ostrm)
{
}

void MuonPed::initHists() {
  m_histograms.resize(RngIdx::N_VALS);

  for (RngIdx rngIdx; rngIdx.isValid(); rngIdx++) {
    if (m_histograms[rngIdx] == 0) {
      string histname = genHistName(rngIdx);
      
      m_histograms[rngIdx] = new TH1S(histname.c_str(),
                                      histname.c_str(),
                                      1000,0.5,1000.5);
    }
  }
}

void MuonPed::fillHists(unsigned nEvents, 
                        const vector<string> &rootFileList, 
                        const RoughPed &roughPeds) {

  // build histograms
  initHists();

  RootFileAnalysis rootFile(0, &rootFileList, 0);

  // enable only needed branches in root file
  rootFile.getDigiChain()->SetBranchStatus("*",0);
  rootFile.getDigiChain()->SetBranchStatus("m_calDigiCloneCol");

  nEvents = min<unsigned>(nEvents, rootFile.getEntries());
  m_ostrm << __FILE__ << ": Processing: " << nEvents << " events." << endl;


  // Basic digi-event loop
  for (unsigned eventNum = 0; eventNum < nEvents; eventNum++) {
    if (eventNum % 1000 == 0) {
      m_ostrm << "Event: " << eventNum << endl;
      m_ostrm.flush();
    }

    if (!rootFile.getEvent(eventNum)) {
      m_ostrm << "Warning, event " << eventNum << " not read." << endl;
      continue;
    }

    const DigiEvent *digiEvent = rootFile.getDigiEvent();
    if (!digiEvent) {
      m_ostrm << __FILE__ << ": Unable to read DigiEvent " << eventNum  << endl;
      continue;
    }
    const TClonesArray* calDigiCol = digiEvent->getCalDigiCol();
    if (!calDigiCol) {
      m_ostrm << "no calDigiCol found for event#" << eventNum << endl;
      continue;
    }

    TIter calDigiIter(calDigiCol);
    const CalDigi *pCalDigi = 0;

    //loop through each 'hit' in one event
    while ((pCalDigi = (CalDigi*)calDigiIter.Next())) {
      const CalDigi &calDigi = *pCalDigi; // use reference to avoid -> syntax

      //-- XtalId --//
      idents::CalXtalId id(calDigi.getPackedId()); // get interaction information
      // skip hits not for current tower.
      XtalIdx xtalIdx(id);

      int nRO = calDigi.getNumReadouts();

      if (nRO != 4) {
        ostringstream tmp;
        tmp << __FILE__  << ":"     << __LINE__ << " " 
            << "Event# " << eventNum << " Invalid nReadouts, expecting 4";
        throw tmp.str();
      }

      // 1st look at LEX8 vals
      CalArray<FaceNum, float> adc;
      bool badHit = false;
      for (FaceNum face; face.isValid(); face++) {
        adc[face] = calDigi.getAdcSelectedRange(LEX8, (CalXtalId::XtalFace)face.val());
        // check for missing readout
        if (adc[face] < 0) {
          m_ostrm << "Couldn't get LEX8 readout for event=" << eventNum << endl;
          badHit = true;
          continue;
        }
      }
      if (badHit) continue;

      // skip outliers (outside of 5 sigma on either side)
      if (fabs(adc[NEG_FACE] - roughPeds.getPed(FaceIdx(xtalIdx,NEG_FACE))) < 
          5*roughPeds.getPedSig(FaceIdx(xtalIdx,NEG_FACE)) &&
          fabs(adc[POS_FACE] - roughPeds.getPed(FaceIdx(xtalIdx,POS_FACE))) < 
          5*roughPeds.getPedSig(FaceIdx(xtalIdx,POS_FACE))) {

        for (unsigned short n = 0; n < nRO; n++) {
          const CalXtalReadout &readout = *calDigi.getXtalReadout(n);
          
          for (FaceNum face; face.isValid(); face++) {
            // check that we are in the expected readout mode
            RngNum rng = readout.getRange((CalXtalId::XtalFace)face.val());
            int adc = readout.getAdc((CalXtalId::XtalFace)face.val());
            RngIdx rngIdxP(xtalIdx, face, rng);
            m_histograms[rngIdxP]->Fill(adc);
          }
        }
      }
    }
  }
}

void MuonPed::fitHists() {
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
    for( int iter=0; iter<3;iter++ ) {
      h.SetAxisRange(av-3*err,av+3*err);
      av = h.GetMean(); err= h.GetRMS();
    }
    
    // gaussian fit
    h.Fit("gaus", "Q","", av-3*err, av+3*err );
    h.SetAxisRange(av-150,av+150);

    // assign values to permanent arrays
    m_peds[rngIdx] =
      ((TF1&)*h.GetFunction("gaus")).GetParameter(1);
    m_pedSig[rngIdx] =
      ((TF1&)*h.GetFunction("gaus")).GetParameter(2);
  }
}

void MuonPed::writeTXT(const string &filename) const {
  ofstream outfile(filename.c_str());
  if (!outfile.is_open())
    throw string("Unable to open " + filename);

  for (RngIdx rngIdx; rngIdx.isValid(); rngIdx++)
    if (m_peds[rngIdx] != INVALID_PED)
      outfile << rngIdx.getTwr().val()
              << " " << rngIdx.getLyr().val()
              << " " << rngIdx.getCol().val()
              << " " << rngIdx.getFace().val()
              << " " << rngIdx.getRng().val()
              << " " << m_peds[rngIdx]
              << " " << m_pedSig[rngIdx]
              << endl;
}

  
void MuonPed::readTXT(const string &filename) {
  ifstream infile(filename.c_str());
  if (!infile.is_open())
    throw string("Unable to open " + filename);

  while(infile.good()) {
    float ped, sig;
    unsigned short twr;
    unsigned short lyr;
    unsigned short col;
    unsigned short face;
    unsigned short rng;
    
    infile >> twr
           >> lyr
           >> col
           >> face
           >> rng
           >> ped
           >> sig;
    // quit once we can't read any more values
    if (infile.fail()) break; 

    RngIdx rngIdx(twr, lyr, col, face, rng);
    m_peds[rngIdx]= ped;
    m_pedSig[rngIdx]= sig;
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
