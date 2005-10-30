#ifndef CfRoot_h
#define CfRoot_h

// LOCAL INCLUDES
#include "CfData.h"
#include "RootFileAnalysis.h"

// GLAST INCLUDES

// EXTLIB INCLUDES
#include "TObjArray.h"

// SYS INCLUDES


/** @class CfRoot
    \brief populates CfData class from singlex16 charge injection event file

    \note CfRoot functions process one diode at a time, use SetDiode()
    to  alternate
*/
class CfRoot : public RootFileAnalysis {
public:
  /// Standard ctor, where user provides the names of the input root files
  CfRoot(const vector<string> &digiFileNames,
         CfData  &data, 
         CfCfg   &cfg,
         DiodeNum diode);

  ~CfRoot();

  /// Processes a single event.  Populates histograms w/ xtal hit info
  void ProcessEvt();

  /// Process a series of events from a digi-root file
  void EvtLoop(Int_t nEvtAsked);

private:
  /// initialize histograms (called by constructor)
  void createHists();

  /// checks rng against m_curDiode setting
  bool isRngEnabled(RngNum rng);

  /// current working diode
  DiodeNum m_curDiode;

  /// output data stucture
  CfData &m_cfData;
  /// application configuration settings
  CfCfg  &m_cfg;

  /// current event id
  int m_evtId;     
  /// max event to loop to
  int m_nEvtMax;   
  /// count good events
  int m_iGoodEvt;  

  /** \brief per channel histograms re-used for each dac setting.
      \note for internal use, not intended to be saved in output histograms
  */
  TObjArray m_ciHists;

};


#endif
