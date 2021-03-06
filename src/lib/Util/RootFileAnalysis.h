#ifndef RootFileAnalysis_h
#define RootFileAnalysis_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Util/RootFileAnalysis.h,v 1.4 2007/10/15 15:17:32 golpa Exp $

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES

// GLAST INCLUDES

// EXTLIB INCLUDES
#include "TChain.h"

// STD INCLUDES
#include <iostream>
#include <vector>

class McEvent;
class DigiEvent;
class ReconEvent;
class GcrSelectEvent;

namespace calibGenCAL {


  /** \brief Makes a VCR for GLAST root event files

  RootFileAnalysis contains the following features...

  1) ability to read in one or more GLAST digi, recon and/or mc root files,
  also svac tuple.
  2) ability to step/rewind through events in those files
  3) events are read / stepped in parallel so that digi/recon/mc trees are always
  in sync.
  4) User is responsible for enabling branches within each TTree/TChain
  5) for event data classes (digi,mc,recon) this class will store the pointers
  to the event objects and make them available via getXXXEvent() methods.
  6) for tuple event files, user is responsible for maintaining and assigning
  branch data destination pointers.

  */
  class RootFileAnalysis {
  public:

    /**
       \param mcFilenames (set to NULL to disable MC ROOT Chain)
       \param digiFilenames (set to NULL to disable Digi ROOT Chain)
       \param reconFilenames (set to NULL to disable recon ROOT Chain)
       \param gcrSelectFilenames (set to NULL to disable gcrSelect ROOT Chain)
       \param calTupleFileanmes (set to NULL to disable calTuple ROOT Chanin)

    */
    RootFileAnalysis(const std::vector<std::string> *mcFilenames = 0,
                     const std::vector<std::string> *digiFilenames = 0,
                     const std::vector<std::string> *reconFilenames = 0,
                     const std::vector<std::string> *svacFilenames = 0,
                     const std::vector<std::string> *gcrSelectFilenames = 0,
                     const std::vector<std::string> *calTupleFileanmes = 0
                     );

    ~RootFileAnalysis();

    /// returns total number of events in all open files
    UInt_t getEntries() const;

    /// Retrieve pointers to given event #.
    /// \note events are numbered by position in root chain, not
    /// by EventID field
    UInt_t getEvent(UInt_t iEvt);

    const McEvent   *getMcEvent() const {
      return m_mcEvt;
    }

    const DigiEvent *getDigiEvent() const {
      return m_digiEvt;
    }

    const ReconEvent  *getReconEvent() const {
      return m_reconEvt;
    }

    const GcrSelectEvent *getGcrSelectEvent() const {
      return m_gcrSelectEvt;
    }

    TChain *getMcChain()   {
      return &m_mcChain;
    }

    TChain *getDigiChain() {
      return &m_digiChain;
    }

    TChain *getReconChain()  {
      return &m_reconChain;
    }

    TChain *getSvacChain() {
      return &m_svacChain;
    }

    TChain *getGcrSelectChain()  {
      return &m_gcrSelectChain;
    }

    TChain *getCalTupleChain() {
      return &m_calTupleChain;
    }

  private:

    /// helpful list of all TChains
    TObjArray        m_chainArr;

    /// Chains store event data for all files
    TChain           m_mcChain;
    /// Pointer to current McEvent
    McEvent     *    m_mcEvt;

    /// Chains store event data for all files
    TChain           m_digiChain;
    /// pointer to current DigiEvent
    DigiEvent   *    m_digiEvt;

    /// Chains store event data for all files
    TChain           m_reconChain;
    /// pointer to current reconEvent
    ReconEvent  *    m_reconEvt;

    /// Chains store event data for all files
    TChain           m_svacChain;

    /// Chains store event data for all files
    TChain           m_gcrSelectChain;

    /// Chains store event data for all files
    TChain           m_calTupleChain;

    /// pointer to current gcrSelectEvent
    GcrSelectEvent  *m_gcrSelectEvt;


    /// current event number
    unsigned         m_nextEvt;
  };

}; // namespace calibGenCAL
#endif
