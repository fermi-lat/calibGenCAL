#ifndef RootFileAnalysis_h
#define RootFileAnalysis_h 1

// LOCAL INCLUDES

// GLAST INCLUDES
#include "digiRootData/DigiEvent.h"
#include "reconRootData/ReconEvent.h"
#include "mcRootData/McEvent.h"

// EXTLIB INCLUDES
#include "TChain.h"

// STD INCLUDES
#include <string>
#include <vector>
#include <iostream>

/*
  RootFileAnalysis contains the following features...

  1) ability to read in one or more GLAST digi, recon and/or mc root files.
  2) ability to step/rewind through events in those files
*/

using namespace std;

class RootFileAnalysis {
  public :
    /// Special ctor which accepts TChains for input files
    RootFileAnalysis(const vector<string> &mcFilenames,
                     const vector<string> &digiFilenames,
                     const vector<string> &recFilenames,
                     ostream &ostr = cout);
  ~RootFileAnalysis();

  /// start next Go with this event
  void startWithEvt(Int_t nEvt) { m_startEvt = nEvt; };
  /// reset for next Go to start at beginning of file
  void rewind() { m_startEvt = 0; };

  /// returns number of events in all open files
  UInt_t getEntries() const;
  /// retrieve a pointer to event number.
  UInt_t getEvent(UInt_t ievt);

 protected:
  /// Optional TChain input
  TChain      m_mcChain, m_digiChain, m_recChain;

  /// Pointer to a McEvent
  McEvent     *m_mcEvt;
  /// pointer to a DigiEvent, w/ each get event, ROOT will
  DigiEvent   *m_digiEvt;
  /// pointer to a ReconEvent
  ReconEvent  *m_recEvt;

  /// pointers to TChains
  TObjArray   m_chainArr;

  bool m_mcEnabled, m_digiEnabled, m_recEnabled;

  /// starting event number
  Int_t m_startEvt;

  /// Zeros out all member vars, does NOT free memory,for use in constructor
  void zeroMembers();

  vector<string> m_mcFilenames;
  vector<string> m_digiFilenames;
  vector<string> m_recFilenames;

  ostream &m_ostrm;

};

#endif
