#ifndef CfData_h
#define CfData_h

/** @file
    $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/CfData.h,v 1.3 2006/01/13 17:25:58 fewtrell Exp $
 */

// LOCAL INCLUDES
#include "CalDefs.h"
#include "CalVec.h"
#include "CfCfg.h"

// GLAST INCLUDES

// EXTLIB INCLUDES
#include "TF1.h"
#include "TProfile.h"
#include "TMultiGraph.h"

// STD INCLUDES

using namespace CalDefs;

/** @class CfData One set of mean adc values & associated charge injection DAC values for GLAST Cal integral nonlinearity calibration
    
    Contains both raw input data and smoothed summary output data.
*/
class CfData {
  friend class CfRoot;

public:

  CfData(CfCfg &cfg, const string &histFilename);
  ~CfData();

  /// \brief generate smoothed spline points from raw data.
  void FitData();
  void corr_FLE_Thresh(CfData& data_high_thresh);
  /// write spline points out to xml calibration file
  void WriteSplinesXML(const string &filename, const string &dtdPath);
  /// write spline points out to space-delimited txt file
  void WriteSplinesTXT(const string &filename);
  /// read spline points in from space-delimited txt file
  void ReadSplinesTXT (const string &filename);

  /// get series of charge injection DAC values used for given energy range in
  const vector<float>& getSplineDAC(tRngIdx rngIdx) const 
    {return m_splineDAC[rngIdx];}

  /// get series of measured ADC values for given channel
  const vector<float>& getSplineADC(tRngIdx rngIdx) const 
    {return m_splineADC[rngIdx];}

  /// get number of points
  int getNSplineADC(tRngIdx rngIdx) const {
    return m_splineADC[rngIdx].size();
  }

  /// generate ROOT graph objects of spline points to be saved in
  /// current ROOT directory
  void makeGraphs();

private:
  /// initialize histograms (called by constructor)
  void createHists();
  /// writes histograms to file & closes file if m_histFile is open.  deletes all open histograms
  void closeHistfile();    
  /// opens new histogram file.  closes current m_histFile if it is open
  void openHistfile(); 

  /// function object is reused for smoothing
  TF1 splineFunc;

  /// store mean ADC for each channel / DAC setting
  CalVec<tRngIdx, vector<float> >  m_adcMean;

  /// output ADC spline points
  CalVec<tRngIdx, vector<float> >  m_splineADC;
  /// output DAC spline points
  CalVec<tRngIdx, vector<float> >  m_splineDAC;

  /// application configuration
  CfCfg &m_cfg;

  /** \brief profile histogram for each channel, adc vs dac
  */
  CalVec<tRngIdx, TProfile*> m_ciRawProfs;

  /** \brief graphs containing cleaned input points w/ overlayed output spline points
   */
  CalVec<tRngIdx, TMultiGraph*> m_ciGraphs;


  /// int c-style array copy of CfCfg::dacVals needed by some fit routines
  float *m_testDAC;

  /// Current histogram file
  auto_ptr<TFile> m_histFile;  
  /// name of the current output histogram ROOT file
  string m_histFilename;  

};


#endif
