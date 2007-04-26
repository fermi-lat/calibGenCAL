// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Util/MultiPeakGaus.cxx,v 1.3 2007/04/10 14:51:02 fewtrell Exp $

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
#include <stdexcept>
#include <iostream>

using namespace std;

namespace {
  static const string func_name("muti_gaus");

  /// current set of particle Z's to fit for
  static MultiPeakGaus::ZSet zset;

  /// whether to fix each parameter
  //  static vector<bool> fixParms;

  /// whether to use fitting limits for each paramter
  //  static vector<bool> useLims;

  /// current set of initial values for each fitting param
  //  static vector<float> initialVals;

  /// current set of lower limits for each fitting param
  //  static vector<float> lowerLims;

  /// current set of upperLimits for each fitting param
  //  static vector<float> upperLims;

  /// current ROOT function object.
  static auto_ptr<TF1> fitFunc;

  static const unsigned short nParms = 6;

  static void resetMPD(float newMPD) {
    //initialVals[0] = 11.2/newMPD;
  }
  
  //   static Double_t multigaufun(Double_t *x,
  //                               Double_t *par) {

  //     unsigned short zIdx = 0;
  //     Double_t retval = 0;

  //     cout << "x:" << x[0] << " z1_mpv:" << par[0];
    
  //     for (MultiPeakGaus::ZSet::const_iterator it(zset.begin());
  //          it != zset.end();
  //          it++) {
  //       unsigned short peakIdx = 1+zIdx;
  //       unsigned short widIdx  = 1+zset.size()+zIdx;
  //       unsigned short normIdx = 1+zset.size()*2+zIdx;

  //       float mpv  = par[peakIdx]*par[0];
  //       float wid  = par[widIdx];
  //       float norm = par[normIdx];

  //       retval += TMath::Gaus(x[0], mpv, wid, norm);

  //       cout << "p: " << peakIdx
  //            << "mpv: " << mpv
  //            << "wid: " << wid
  //            << "norm: " << norm
  //            << "ret: " << retval
  //            << endl;
  //     }

    
  //     return retval;
  //   }

  static Double_t multigaufun(Double_t *x,
                              Double_t *par) {
    Double_t retval=0;
     
    retval += TMath::Gaus(x[0], par[0], par[1],true)*par[2];
    retval += TMath::Gaus(x[0], par[3], par[4],true)*par[5];

    //if (retval > 1) {
    //  cout << x[0] << " ";
    //  for (unsigned u = 0 ; u < nParms; u++)
    //    cout << par[u] << " ";
    //  cout << retval << endl;
    //}

    return retval;
  }
  

  static string getParName(unsigned short n) {
    if (n == 0) return "MPD";
                        
    unsigned short partype = (n-1)/zset.size();

    unsigned short parnum = (n-1)%zset.size();

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

    retval << parnum;

    return retval.str();
  }

  TF1 * buildFunc() {
    //     if (initialVals.empty())
    //       throw logic_error("MultiPeakGaus func() not initialized");

    //     TF1 *ffit = new TF1(func_name.c_str(),
    //                         multigaufun,
    //                         0,
    //                         4095,
    //                         nParms);

    TF1 *ffit = (TF1*)new TF1(func_name.c_str(),
                              multigaufun,
                              0,
                              4095,
                              nParms);
                        

    //cout << "create tf1 obj, nparms=" << nParms << endl;
          
    //      ffit->SetParName(i,getParName(i).c_str());
    //       if (useLims[i])
    //         ffit->SetParLimits(i,lowerLims[i],upperLims[i]);
    //       if (fixParms[i])
    //         ffit->FixParameter(i,initialVals[i]);

    return ffit;
  }
        
  void init(const MultiPeakGaus::ZSet &zSet) {
    //-- CLEAR OLD CFG --//
    zset.clear();
    //    fixParms.clear();
    //    useLims.clear();
    //    initialVals.clear();
    //    lowerLims.clear();
    //    upperLims.clear();
    fitFunc.reset(0);
    //nParms = 4;

    //-- CREATE NEW CFG --//
    zset = zSet;

    //     // 1st parm is scaling factor, let it float freely
    //     fixParms.push_back(false);
    //     useLims.push_back(false);
    //     initialVals.push_back(11.2);
    //     lowerLims.push_back(0); // unused
    //     upperLims.push_back(0);
    //     nParms++;

    //     // set fitting parms for each successive Z peak
    //     for (MultiPeakGaus::ZSet::const_iterator zit = zSet.begin();
    //          zit != zSet.end();
    //          zit++) {
    //       float z = *zit;

    //       fixParms.push_back(false);
    //       useLims.push_back(false);
    //       initialVals.push_back(pow(z,2));  // ene deposited ratio proportional to z^2
    //       lowerLims.push_back(pow(z-.5,2)); // limit must keep out of range of previous z 
    //       upperLims.push_back(pow(z+.5,2)); // limit must keep out of range of next z 
    //       nParms++;
    //     }

    //     // set fitting parms for each successive Z peak width
    //     for (MultiPeakGaus::ZSet::const_iterator zit = zSet.begin();
    //          zit != zSet.end();
    //          zit++) {
    //       float z = *zit;

    //       fixParms.push_back(false);
    //       useLims.push_back(false);
    //       initialVals.push_back(10);  // ene deposited ratio proportional to z^2
    //       lowerLims.push_back(0); // limit must keep out of range of previous z 
    //       upperLims.push_back(0); // limit must keep out of range of next z 
    //       nParms++;
    //     }

    //     // set fitting parms for each successive Z peak norm
    //     for (MultiPeakGaus::ZSet::const_iterator zit = zSet.begin();
    //          zit != zSet.end();
    //          zit++) {
    //       float z = *zit;

    //       fixParms.push_back(false);
    //       useLims.push_back(false);
    //       initialVals.push_back(100);  // ene deposited ratio proportional to z^2
    //       lowerLims.push_back(0); // limit must keep out of range of previous z 
    //       upperLims.push_back(0); // limit must keep out of range of next z 
    //       nParms++;
    //     }

    fitFunc.reset(buildFunc());
  }

};

void MultiPeakGaus::fitHist(TH1 &hist, const set<unsigned short> &zSet, float initialMPD) {
  if (zset != zSet)
    init(zSet);

  // reset initial MPD each time
  resetMPD(initialMPD);
  double initialVals[] = {
    1000,
    10,
    100,
    1400,
    10,100
  };
  for (unsigned i = 0; i < nParms; i++)
    fitFunc->SetParameter(i,initialVals[i]);

  fitFunc->SetParLimits(0,800,1200);
  //fitFunc->SetParLimits(1,1,300);
  fitFunc->SetParLimits(3,1200,1500);
  //fitFunc->SetParLimits(4,1,300);
  fitFunc->SetRange(800,1500);

  hist.Fit(fitFunc.get(),"BR");
}
