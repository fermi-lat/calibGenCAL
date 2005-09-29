#ifndef CfData_h
#define CfData_h

// LOCAL INCLUDES
#include "CalDefs.h"
#include "CfCfg.h"

// GLAST INCLUDES

// EXTLIB INCLUDES
#include "TF1.h"

// STD INCLUDES


using namespace CalDefs;
/** @class CfData One set of mean adc values & associated charge injection DAC values for GLAST Cal integral nonlinearity calibration
    
    Contains both raw input data. And smoothed summary output data.
*/
class CfData {
  friend class CfRoot;

public:

  CfData(CfCfg &cfg);
  ~CfData() {delete m_dacArr;}

  void FitData();
  void corr_FLE_Thresh(CfData& data_high_thresh);
  void WriteSplinesXML(const string &filename, const string &dtdPath);
  void WriteSplinesTXT(const string &filename);
  void ReadSplinesTXT (const string &filename);

  /// get series of charge injection DAC values used for given energy range in
  const vector<int>& getSplineDAC(RngNum rng) const {return m_splineDAC[rng];}

  /// get series of measured ADC values for given channel
  const vector<float>& getSplineADC(tRngIdx rngIdx) const {
    return m_splineADC[rngIdx];
  }

  /// get number of points
  int getNSplineADC(tRngIdx rngIdx) const {
    return m_splineADC[rngIdx].size();
  }

private:
  TF1 splineFunc;

  CalVec<tRngIdx, vector<float> >  m_adcSum;
  CalVec<tRngIdx, vector<float> >  m_adcN;
  CalVec<tRngIdx, vector<float> >  m_adcMean;

  CalVec<tRngIdx, vector<float> >  m_splineADC;
  CalVec<RngNum, vector<int> >    m_splineDAC;

  CfCfg &m_cfg;
  // int c-style array copy of CfCfg::dacVals needed by some fit routines
  float *m_dacArr;
};


#endif
