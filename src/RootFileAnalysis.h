/*
  RootFileAnalysis contains the following features...

  1) ability to read in one or more GLAST digi, recon and/or mc root files.
  2) ability to step/rewind through events in those files
*/

#ifndef RootFileAnalysis_h
#define RootFileAnalysis_h 1

#include "digiRootData/DigiEvent.h"
#include "reconRootData/ReconEvent.h"
#include "mcRootData/McEvent.h"

//ROOT INCLUDES
#include "TChain.h"

#include <string>
#include <vector>

class RootFileAnalysis {
public :
  /// Special ctor which accepts TChains for input files
  RootFileAnalysis(std::vector<std::string> *digiFileNames,
						 std::vector<std::string> *recFileNames,
						 std::vector<std::string> *mcFileNames);

  ~RootFileAnalysis();

  /// start next Go with this event
  void StartWithEvent(Int_t event) { m_StartEvent = event; };
  /// reset for next Go to start at beginning of file
  void Rewind() { m_StartEvent = 0; };

  /// returns number of events in all open files
  UInt_t GetEntries() const;
  /// retrieve a pointer to event number.
  UInt_t GetEvent(UInt_t ievt);

protected:
  /// Optional TChain input
  TChain      *m_digiChain, *m_recChain, *m_mcChain;
  /// pointer to a DigiEvent
  DigiEvent   *evt;
  /// pointer to a ReconEvent
  ReconEvent  *rec;
  /// Pointer to a McEvent
  McEvent     *mc;

  /// pointers to TChains
  TObjArray   *chainArr;
  double prevTimeStamp;

  UInt_t digiEventId, reconEventId, mcEventId;
  UInt_t digiRunNum, reconRunNum, mcRunNum;

  /// starting event number
  Int_t m_StartEvent;

  /// Zeros out all member vars, does NOT free memory,for use in constructor
  void ZeroMembers();

};

#endif
