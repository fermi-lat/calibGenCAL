#ifndef RootFileAnalysis_h
#define RootFileAnalysis_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/RootFileAnalysis.h,v 1.4 2006/09/21 19:30:00 fewtrell Exp $
/** @file
    @author Zachary Fewtrell
*/


// LOCAL INCLUDES

// GLAST INCLUDES

// EXTLIB INCLUDES
#include "TChain.h"

// STD INCLUDES
#include <iostream>


class McEvent;
class DigiEvent;
class ReconEvent;

/** \brief Makes a VCR for GLAST root event files

RootFileAnalysis contains the following features...

1) ability to read in one or more GLAST digi, recon and/or mc root files.
2) ability to step/rewind through events in those files
*/
class RootFileAnalysis {
  public :
    /**
       \param mcFilenames (set to NULL to disable MC ROOT Chain)
       \param digiFilenames (set to NULL to disable Digi ROOT Chain)
       \param reconFilenames (set to NULL to disable recon ROOT Chain)
       \param ostrm optional logging stream

    */
    RootFileAnalysis(const std::vector<std::string> *mcFilenames,
                     const std::vector<std::string> *digiFilenames,
                     const std::vector<std::string> *reconFilenames,
                     std::ostream &ostrm=std::cout);

  ~RootFileAnalysis();

  /// returns total number of events in all open files
  UInt_t getEntries() const;
  
  /// Retrieve pointers to given event #.
  /// \note events are numbered by position in root chain, not
  /// by EventID field
  UInt_t getEvent(UInt_t iEvt);

  McEvent   *getMcEvent() const {return m_mcEvt;}
  DigiEvent *getDigiEvent() const {return m_digiEvt;}
  ReconEvent  *getReconEvent() const {return m_reconEvt;}

  TChain *getMcChain()   {return &m_mcChain;}
  TChain *getDigiChain() {return &m_digiChain;}
  TChain *getReconChain()  {return &m_reconChain;}
 
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
  TChain       m_reconChain;
  /// pointer to current reconEvent
  ReconEvent  *m_reconEvt;

  /// helpful list of all 3 TChains
  TObjArray   m_chainArr;

  /// current event number
  unsigned m_nextEvt;

  /// log to here
  ostream &m_ostrm;

};

#endif
