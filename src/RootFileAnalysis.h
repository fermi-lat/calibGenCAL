#ifndef RootFileAnalysis_h
#define RootFileAnalysis_h 1

#include <string>
#include <vector>
#include <iostream>

/*
  RootFileAnalysis contains the following features...

  1) ability to read in one or more GLAST digi, recon and/or mc root files.
  2) ability to step/rewind through events in those files
*/

#include "digiRootData/DigiEvent.h"
#include "reconRootData/ReconEvent.h"
#include "mcRootData/McEvent.h"

//ROOT INCLUDES
#include "TChain.h"

using namespace std;

class RootFileAnalysis {
public :
  /// Special ctor which accepts TChains for input files
  RootFileAnalysis(const vector<string> &digiFilenames,
		   const vector<string> &recFilenames,
		   const vector<string> &mcFilenames,
                   ostream &ostr = cout);

  /// start next Go with this event
  void startWithEvent(Int_t event) { m_startEvent = event; };
  /// reset for next Go to start at beginning of file
  void rewind() { m_startEvent = 0; };

  /// returns number of events in all open files
  UInt_t getEntries() const;
  /// retrieve a pointer to event number.
  UInt_t getEvent(UInt_t ievt);

protected:
  /// Optional TChain input
  TChain      m_digiChain, m_recChain, m_mcChain;
  /// pointer to a DigiEvent, w/ each get event, ROOT will
  DigiEvent   *m_evt;
  /// pointer to a ReconEvent
  ReconEvent  *m_rec;
  /// Pointer to a McEvent
  McEvent     *m_mc;

  /// pointers to TChains
  TObjArray   m_chainArr;

  bool m_mcEnabled, m_digiEnabled, m_recEnabled;

  /// starting event number
  Int_t m_startEvent;

  /// Zeros out all member vars, does NOT free memory,for use in constructor
  void zeroMembers();

  vector<string> m_digiFilenames;
  vector<string> m_recFilenames;
  vector<string> m_mcFilenames;

  ostream &m_ostr;

};

#endif
