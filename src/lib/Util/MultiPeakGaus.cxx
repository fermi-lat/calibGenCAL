// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Util/MultiGausFun.cxx,v 1.1 2007/03/27 18:50:51 fewtrell Exp $

/** @file
    @author Zach Fewtrell
*/
// LOCAL INCLUDES
#include "MultiPeakGaus.h"

// GLAST INCLUDES

// EXTLIB INCLUDES
#include "TF1.h"
#include "TH1.h"

// STD INCLUDES
#include <string>
#include <set>
#include <memory>
#include <algorithm>
#include <cmath>
#include <sstream>

using namespace std;

namespace {
  static const string func_name("muti_gaus");

  /// current set of particle Z's to fit for
  static MultiPeakGaus::ZSet zset;

  /// whether to fix each parameter
  static vector<bool> fixParms;
        
  /// whether to use fitting limits for each paramter
  static vector<bool> useLims;

  /// current set of initial values for each fitting param
  static vector<float> initialVals;

  /// current set of lower limits for each fitting param
  static vector<float> lowerLims;

  /// current set of upperLimits for each fitting param
  static vector<float> upperLims;

  /// current ROOT function object.
  static auto_ptr<TF1> fitFunc;

  static void resetMPD(float newMPD) {
	  initialVals[0] = 11.2*newMPD;
  }
  
  static Double_t multigaufun(Double_t *x,
                              Double_t *par) {

  unsigned short zIdx = 0;

  Double_t retval = 0;
  for (MultiPeakGaus::ZSet::const_iterator it = zset.begin();
	  it != zset.end();
	  it++) {
		  unsigned short z = *it;

		  unsigned short peakIdx = 1+zIdx;
		  unsigned short widIdx  = 1+zset.size()+zIdx;
		  unsigned short normIdx = 1+zset.size()*2+zIdx;

		  float mpv     = par[peakIdx]*par[0];
		  float wid     = par[widIdx];
		  float norm = par[normIdx];

		  retval += TMath::Gaus(x[0], mpv, wid, norm);
	  }

    return retval;
  }

  

  static string getParName(unsigned short n) {
    if (n == 0) return "MPD";
                        
    unsigned short partype = (n-1)/initialVals.size();

    unsigned short parnum = (n-1)%initialVals.size();

    ostringstream retval;
    switch (partype) {
    case 0:
      retval << "peak_ratio_";
      break;
    case 1:
      retval << "peak_width_";
      break;
    case 2:
      retval << "peak_norm_";
      break;
    default:
      throw logic_error("getParName() n > num_parms");
    }

	return retval.str();

  }

  TF1 * buildFunc() {
    if (initialVals.empty())
      throw logic_error("MultiPeakGaus func() not initialized");

    TF1 *ffit = new TF1(func_name.c_str(),
                        multigaufun,
                        0,
                        4095,
                        initialVals.size());
          
    for (unsigned i = 0; i < initialVals.size(); i++) {
      ffit->SetParameter(i,initialVals[i]);
      ffit->SetParName(i,getParName(i).c_str());
      if (useLims[i])
        ffit->SetParLimits(i,lowerLims[i],upperLims[i]);
      if (fixParms[i])
        ffit->FixParameter(i,initialVals[i]);
    }

    return ffit;
  }
        
  void init(const MultiPeakGaus::ZSet &zSet) {
    // first blank out current cfg
    zset.clear();
    fixParms.clear();
    useLims.clear();
    initialVals.clear();
    lowerLims.clear();
    upperLims.clear();
    fitFunc.reset(0);

    // 1st parm is scaling factor, let it float freely
    fixParms.push_back(false);
    useLims.push_back(false);
    initialVals.push_back(11.2);
    lowerLims.push_back(0); // unused
    upperLims.push_back(0);

    // set fitting parms for each successive Z
	for (MultiPeakGaus::ZSet::const_iterator zit = zSet.begin();
         zit != zSet.end();
         zit++) {
      float z = *zit;

      fixParms.push_back(false);
      useLims.push_back(true);
      initialVals.push_back(pow(z,2));  // ene deposited ratio proportional to z^2
      lowerLims.push_back(pow(z-.5,2)); // limit must keep out of range of previous z 
      upperLims.push_back(pow(z+.5,2)); // limit must keep out of range of next z 

                
    }

    fitFunc.reset(buildFunc());

  }

};

void MultiPeakGaus::fitHist(TH1 &hist, const set<unsigned short> &zSet, float initialMPD) {
  if (zset != zSet)
    init(zSet);

  // reset initial MPD each time
  resetMPD(initialMPD);

  hist.Fit(fitFunc.get());
}
