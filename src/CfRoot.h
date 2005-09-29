#ifndef CfRoot_h
#define CfRoot_h

// LOCAL INCLUDES
#include "CfData.h"
#include "RootFileAnalysis.h"

// GLAST INCLUDES

// EXTLIB INCLUDES
#include "TProfile.h"
#include "TObjArray.h"

// SYS INCLUDES


/** @class CfRoot
    \brief populates CfData class from singlex16 charge injection event file

    \note CfRoot functions process one diode at a time, use SetDiode() to  alternate
*/
class CfRoot : public RootFileAnalysis {
public:
  /// @enum Diode Specify LE, HE, BOTH_DIODES
  typedef enum Diode {
    LE,
    HE,
    BOTH_DIODES};

  /// Standard ctor, where user provides the names of the input root files
  CfRoot(const vector<string> &digiFileNames,
         CfData  &data, 
         CfCfg   &cfg, 
         const string &histFilename);

  ~CfRoot();

  /// Processes a single event.  Populates histograms w/ xtal hit info
  void DigiCal();

  /// Process a series of events from a digi-root file
  void Go(Int_t nEvtAsked);

  /// Select diode to process, CfRoot only processes one diode at a time
  void SetDiode(CfRoot::Diode d) {m_curDiode = d;}

private:
  /// initialize histograms (called by constructor)
  void createHists();
  /// writes histograms to file & closes file if m_histFile is open.  deletes all open histograms
  void closeHistfile();    
  /// opens new histogram file.  closes current m_histFile if it is open
  void openHistfile(const string &filename); 

  /// checks rng against m_curDiode setting
  bool isRngEnabled(RngNum rng);

  /// current working diode
  Diode m_curDiode;

  /// output data stucture
  CfData &m_CfData;
  /// application configuration settings
  CfCfg  &m_cfg;

  /// current event id
  int m_evtId;     
  /// max event to loop to
  int m_nEvtMax;   
  /// count good events
  int m_iGoodEvt;  

  /** \brief profile histogram for each channel, adc vs dac
      \note not used internally, used to populate output histograms. Profiles do not contain the outlier reduction that is used during the actual calculation phase.
  */
  
  CalVec<tRngIdx, TProfile*> m_ciProfs;

  /** \brief per channel histograms re-used for each dac setting.
      \note for internal use, not intended to be saved in output histograms
  */
  TObjArray m_ciHists;

  /// Current histogram file
  auto_ptr<TFile> m_histFile;  
  /// name of the current output histogram ROOT file
  string m_histFilename;  
};


#endif
