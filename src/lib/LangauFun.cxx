// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/LangauFun.cxx,v 1.7 2007/01/12 16:03:46 fewtrell Exp $

/** @file
    @author Zach Fewtrell
*/
// LOCAL INCLUDES
#include "LangauFun.h"
#include "CGCUtil.h"

// GLAST INCLUDES

// EXTLIB INCLUDES
#include "TF1.h"
#include "TH1.h"
#include "TNtuple.h"

// STD INCLUDES
#include <cmath>
#include <sstream>

using namespace std;
using namespace CGCUtil;
using namespace CalUtil;

namespace {
  /// function name id
  static const string func_name("langau");

  /// fit parameters
  enum PARMS {
    PARM_LAN_WID,
    PARM_MPV,
    PARM_LAN_AREA,
    PARM_GAU_WID,
    PARM_BKGND_HEIGHT,
    N_PARMS
  };

  static double startVals[N_PARMS] = {
    .0526,
    30.81,
    5000,
    .0618,
    1.76
  };

  static bool fixParms[N_PARMS] = {
    true,
    false,
    false,
    false,
    false
  };

  static bool useLimits[N_PARMS] = {
    true,
    true,
    false,
    true,
    true
  };

  static double parmLo[N_PARMS] = {
    .01,
    1,
    0,
    .01,
    0
  };

  static double parmHi[N_PARMS] = {
    10,
    10000,
    1e9,
    10,
    1e6
  };

  static double fitRange[2] = {0,100};

  /// \brief calculate background model for muon peak fit
  /// 
  /// background is level @ height below peak mpv & 0 afterwards w/ 
  /// exponential decay of width sigma between two regions
  static inline float bckgnd_model(float x, 
                                   float height, 
                                   float mpv, 
                                   float sigma) {
    return height/(1 + exp((x-mpv)/sigma));
  }

  static Double_t langaufun(Double_t *x,
                            Double_t *par)
  {
    // Numeric constants

    static const float invsq2pi = pow(2*M_PI,-.5);   // (2 pi)^(-1/2)
    static const float mpshift  = -0.22278298;       // Landau maximum location

    // Control constants

    static const unsigned short np = 100;                   // number of convolution steps
    static const unsigned short sc = 5;                     // convolution extends to +-sc Gaussian sigmas

    // Variables

    float xx;
    float mpc;
    float fland;
    float sum = 0.0;
    float xlow, xupp;
    float step;
    unsigned short i;

    // landau & gausian wid given as fraction
    // of mpv
    float real_lan = par[PARM_LAN_WID]*par[PARM_MPV];
    float real_gau = par[PARM_GAU_WID]*par[PARM_MPV];


    // MP shift correction

    mpc  = par[PARM_MPV]-mpshift*real_lan;

    // Range of convolution integral

    xlow = x[0]-sc*real_gau;
    xupp = x[0]+sc*real_gau;
    step = (xupp-xlow)/np;

    // Convolution integral of Landau and Gaussian by sum

    for (i = 1; i <= np/2; i++)
      {
        xx    = xlow+(i-.5)* step;
        fland = TMath::Landau(xx, mpc, real_lan)/real_lan;
        sum  += fland *TMath::Gaus(x[0],
                                   xx,
                                   real_gau);

        xx    = xupp-(i-.5)*step;
        fland = TMath::Landau(xx, mpc, real_lan)/real_lan;
        sum  += fland *TMath::Gaus(x[0],
                                   xx,
                                   real_gau);
      }

    // combine sigmas for both landau & gaussian
    float convolved_width = sqrt(real_gau*real_gau + real_lan*real_lan);
    float bkgnd = bckgnd_model(x[0],  
                               par[PARM_BKGND_HEIGHT], // bckgnd height
                               par[PARM_MPV], // mpv
                               convolved_width);

    float retVal = par[PARM_LAN_AREA]*step*sum*invsq2pi/real_gau + bkgnd;

    /*         LogStream::get() << x[0] << " "
               << str_join(par, par+N_PARMS)
               << real_lan << " " 
               << real_gau << " " 
               << retVal << endl;
    */        
    return retVal;
  }

  static TF1 *buildLangauDAC() {

    TF1 *ffit = new TF1(func_name.c_str(), 
                        langaufun, 
                        fitRange[0], 
                        fitRange[1], 
                        N_PARMS);
    ffit->SetParameters(startVals);
    ffit->SetParNames("Landau width", "MP", "Area", "Gaussian #sigma", "Background Level");

    for (int i = 0; i < N_PARMS; i++) {
      if (useLimits[i])
        ffit->SetParLimits(i, parmLo[i], parmHi[i]);
      if (fixParms[i])
        ffit->FixParameter(i, startVals[i]);
    }

    ffit->SetNpx(1000);

    return ffit;
  }

  /// use static auto_ptr so that singletons are properly destroyed on exit.
  /// not entirely necessary, but keeps things clean
  static std::auto_ptr<TF1> langauDAC;

  enum tuple_fields {
    FIELD_XTAL,
    FIELD_MPV,
    FIELD_MPV_ERR,
    FIELD_LAN_WID,
    FIELD_LAN_WID_ERR,
    FIELD_GAU_WID,
    FIELD_GAU_WID_ERR,
    FIELD_BKGND,
    FIELD_BKGND_ERR,
    FIELD_CHISQ,
    FIELD_NDF,
    FIELD_NENTRIES,
    N_TUPLE_FIELDS
  };

  /// list of field names for tuple
  static const string tuple_field_str[N_TUPLE_FIELDS] = {
    "XTAL",
    "MPV",
    "MPV_ERR",
    "LAN_WID",
    "LAN_WID_ERR",
    "GAU_WID",
    "GAU_WID_ERR",
    "BKGND",
    "BKGND_ERR",
    "CHISQ",
    "NDF",
    "NENTRIES"
  };
};

/// retrieve gaussian convolved landau fuction with limits & initial values appropriate for LE CIDAC scale
TF1 &LangauFun::getLangauDAC() {
  if (!langauDAC.get()) langauDAC.reset(buildLangauDAC());
  return *(langauDAC.get());
}


/// build ROOT TNtuple obj w/ fields formatted for this function
TNtuple &LangauFun::buildTuple() {
  string tuple_def(str_join(tuple_field_str,
                            tuple_field_str + N_TUPLE_FIELDS,
                            ":"));
  // remove trailing ':' from join() method
  tuple_def = tuple_def.substr(0,tuple_def.size()-1);
  
  return *(new TNtuple("langau_mpd_fit",
                       "langau_mpd_fit",
                       tuple_def.c_str()));
}

/// fill ROOT TNtuple w/ fitted parms for this func / hist
Int_t LangauFun::fillTuple(XtalIdx xtalId,
                           const TH1 &hist, 
                           TNtuple &tuple) {
  float tuple_data[N_TUPLE_FIELDS];

  const TF1 &func = *(hist.GetFunction(func_name.c_str()));

  tuple_data[FIELD_XTAL] = xtalId.val();
  float mpv = func.GetParameter(PARM_MPV);
  tuple_data[FIELD_MPV] = mpv;
  tuple_data[FIELD_MPV_ERR] = func.GetParError(PARM_MPV);
  tuple_data[FIELD_LAN_WID] = func.GetParameter(PARM_LAN_WID)*mpv;
  tuple_data[FIELD_LAN_WID_ERR] = func.GetParError(PARM_LAN_WID)*mpv;
  tuple_data[FIELD_GAU_WID] = func.GetParameter(PARM_GAU_WID)*mpv;
  tuple_data[FIELD_GAU_WID_ERR] = func.GetParError(PARM_GAU_WID)*mpv;
  tuple_data[FIELD_BKGND] = func.GetParameter(PARM_BKGND_HEIGHT);
  tuple_data[FIELD_BKGND_ERR] = func.GetParError(PARM_BKGND_HEIGHT);
  tuple_data[FIELD_CHISQ] = func.GetChisquare();
  tuple_data[FIELD_NDF] = func.GetNDF();
  tuple_data[FIELD_NENTRIES] = hist.GetEntries();

  return tuple.Fill(tuple_data);
}
