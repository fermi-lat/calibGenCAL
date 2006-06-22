#ifndef RootFileAnalysis_h
#define RootFileAnalysis_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/RootFileAnalysis.h,v 1.1 2006/06/15 20:58:00 fewtrell Exp $
/** @file
    @author Zachary Fewtrell
 */


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

using namespace std;

/** \brief Makes a VCR for GLAST root event files

RootFileAnalysis contains the following features...

1) ability to read in one or more GLAST digi, recon and/or mc root files.
2) ability to step/rewind through events in those files
*/
class RootFileAnalysis {
  public :
    /**
       \param mcFilenames set to NULL to disable MC ROOT Chain
       \param digiFilenames set to NULL to disable Digi ROOT Chain
       \param recFilenames set to NULL to disable Rec ROOT Chain
       \param ostrm optional logging stream

     */
    RootFileAnalysis(const vector<string> *mcFilenames,
                     const vector<string> *digiFilenames,
                     const vector<string> *recFilenames,
                     ostream &ostrm = cout);

  ~RootFileAnalysis();

  /// returns total number of events in all open files
  UInt_t getEntries() const;
  
  /// Retrieve pointers to given event #.
  /// \note events are numbered by position in root chain, not
  /// by EventID field
  UInt_t getEvent(UInt_t iEvt);

  McEvent   *getMcEvent() const {return m_mcEvt;}
  DigiEvent *getDigiEvent() const {return m_digiEvt;}
  ReconEvent  *getRecEvent() const {return m_recEvt;}

  TChain *getMcChain()   {return &m_mcChain;}
  TChain *getDigiChain() {return &m_digiChain;}
  TChain *getRecChain()  {return &m_recChain;}
 
 private:

  /// Chains store event data for all files
  TChain      m_mcChain;
  /// Pointer to current McEvent
  McEvent     *m_mcEvt;

  /// Chains store event data for all files
  TChain      m_digiChain;
  /// pointer to current DigiEvent
  DigiEvent   *m_digiEvt;

  /// Chains store event data for all files
  TChain       m_recChain;
  /// pointer to current ReconEvent
  ReconEvent  *m_recEvt;

  /// helpful list of all 3 TChains
  TObjArray   m_chainArr;

  /// current event number
  Int_t m_nextEvt;

  /// log to here
  ostream &m_ostrm;

};

#endif
