// $Header$
/** @file
    @author Zachary Fewtrell
 */

// LOCAL INCLUDES
#include "RootFileAnalysis.h"
#include "RoughPed.h"

// GLAST INCLUDES

// EXTLIB INCLUDES
#include "TF1.h"
#include "TKey.h"

// STD INCLUDES
#include <sstream>
#include <ostream>

RoughPed::RoughPed(ostream &ostrm) :
  m_peds(FaceIdx::N_VALS, INVALID_PED),
  m_pedSig(FaceIdx::N_VALS, 0),
  m_ostrm(ostrm)
{
}

void RoughPed::initHists() {
  m_histograms.resize(FaceIdx::N_VALS);

  for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++) {
    if (m_histograms[faceIdx] == 0) {
      string histname = genHistName(faceIdx);
      
      m_histograms[faceIdx] = new TH1S(histname.c_str(),
                                       histname.c_str(),
                                       1000,0.5,1000.5);
    }
  }
}

void RoughPed::fillHists(unsigned nEvents, 
                         const vector<string> &rootFileList) {
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

      for (FaceNum face; face.isValid(); face++) {
        float adc = calDigi.getAdcSelectedRange(LEX8, (CalXtalId::XtalFace)face.val());

        // check for missing readout
        bool badHit = false;
        if (adc < 0 ) {
          m_ostrm << "Couldn't get LEX8 readout for event=" << eventNum << endl;
          badHit = true;
          continue;
        }
        if (badHit) continue;

        FaceIdx faceIdx(xtalIdx, face);
        m_histograms[faceIdx]->Fill(adc);
      }
    }
  }
}

void RoughPed::fitHists() {
  for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++) {
    // skip absent histograms
    if (!m_histograms[faceIdx])
      continue;

    // select histogram from list
    TH1S &h= *m_histograms[faceIdx];

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
    m_peds[faceIdx] =
      ((TF1&)*h.GetFunction("gaus")).GetParameter(1);
    m_pedSig[faceIdx] =
      ((TF1&)*h.GetFunction("gaus")).GetParameter(2);
  }
}

void RoughPed::writeTXT(const string &filename) const {
  ofstream outfile(filename.c_str());
  if (!outfile.is_open())
    throw string("Unable to open " + filename);

  for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++)
    if (m_peds[faceIdx] != INVALID_PED)
      outfile << faceIdx.getTwr().val()
              << " " << faceIdx.getLyr().val()
              << " " << faceIdx.getCol().val()
              << " " << faceIdx.getFace().val()
              << " " << m_peds[faceIdx]
              << " " << m_pedSig[faceIdx]
              << endl;
}

  
void RoughPed::readTXT(const string &filename) {
  ifstream infile(filename.c_str());
  if (!infile.is_open())
    throw string("Unable to open " + filename);

  while(infile.good()) {
    float ped, sig;
    unsigned short twr;
    unsigned short lyr;
    unsigned short col;
    unsigned short face;
    
    infile >> twr
           >> lyr
           >> col
           >> face
           >> ped
           >> sig;
    // quit once we can't read any more values
    if (infile.fail()) break; 

    FaceIdx faceIdx(twr, lyr, col, face);
    m_peds[faceIdx]= ped;
    m_pedSig[faceIdx]= sig;
  }
}

void RoughPed::loadHists(const string &filename) {
  TFile histFile(filename.c_str(), "READ");

  for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++) {
    string histname = genHistName(faceIdx);
    TKey *key = histFile.FindKey(histname.c_str());

    // skip missing hist
    if (!key)
      continue;

    TClass *cls = gROOT->GetClass(key->GetClassName());

    if (!cls)
      continue;

    TH1S *hist_ptr = (TH1S*)key->ReadObj();
    if (!hist_ptr) {
      m_ostrm << "Warning: unable to read histogram "
              << histname << endl;
      continue;
    }

    // skip hist if it's the wrong type
    if (!hist_ptr->InheritsFrom(TH1S::Class())) {
      m_ostrm << "Warning: histogram " << histname 
              << " not of type TH1S"   << endl;
      continue;
    }



    // move histogram into Global ROOT memory
    // so it is not deleted when input file is closed.
    // this may be a memory leak, i don't think
    // anyone cares.
    hist_ptr->SetDirectory(0);

    m_histograms[faceIdx] = hist_ptr;
  }
}


string RoughPed::genHistName(FaceIdx faceIdx) {
  ostringstream tmp;
  tmp << "roughpeds_" << faceIdx.val();
  return tmp.str();
}
