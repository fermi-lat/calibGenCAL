// $Header:$

/** @file
    @author Zach Fewtrell
*/
// LOCAL INCLUDES
#include "LangauFun.h"
#include "CGCUtil.h"

// GLAST INCLUDES

// EXTLIB INCLUDES
#include "TF1.h"

// STD INCLUDES
#include <cmath>

using namespace std;
using namespace CGCUtil;

namespace {
  /// fit parameters
  enum PARMS {
    PARM_LAN_WID,
    PARM_MPV,
    PARM_LAN_AREA,
    PARM_GAU_WID,
    PARM_BKGND_HEIGHT,
    N_PARMS
  };

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


    // MP shift correction

    mpc  = par[PARM_MPV]-mpshift*par[PARM_LAN_WID];

    // Range of convolution integral

    xlow = x[0]-sc*par[PARM_GAU_WID];
    xupp = x[0]+sc*par[PARM_GAU_WID];
    step = (xupp-xlow)/np;

    // Convolution integral of Landau and Gaussian by sum

    for (i = 1; i <= np/2; i++)
      {
        xx    = xlow+(i-.5)* step;
        fland = TMath::Landau(xx, mpc, par[PARM_LAN_WID])/par[PARM_LAN_WID];
        sum  += fland *TMath::Gaus(x[0],
                                   xx,
                                   par[PARM_GAU_WID]);

        xx    = xupp-(i-.5)*step;
        fland = TMath::Landau(xx, mpc, par[PARM_LAN_WID])/par[PARM_LAN_WID];
        sum  += fland *TMath::Gaus(x[0],
                                   xx,
                                   par[PARM_GAU_WID]);
      }

    // combine sigmas for both landau & gaussian
    float convolved_width = sqrt(par[PARM_GAU_WID]*par[PARM_GAU_WID] + par[PARM_LAN_WID]*par[PARM_LAN_WID]);
    float bkgnd = bckgnd_model(x[0],  
                               par[PARM_BKGND_HEIGHT], // bckgnd height
                               par[PARM_MPV], // mpv
                               convolved_width);
                               
                               

          

    return par[PARM_LAN_AREA]*step*sum*invsq2pi/par[PARM_GAU_WID] + bkgnd;
  }

  TF1 *buildLangauDAC() {
    //----- fit with (Landau-Gauss convolution + linear bkgd) function

    double fitRange[2], 
      pllo[N_PARMS], 
      plhi[N_PARMS], 
      startVal[N_PARMS];
    // whether or not to use limits in fit.
    bool useLimits[N_PARMS];
        
    fill(pllo, pllo+N_PARMS, 0.0);
    fill(plhi, plhi+N_PARMS, 0.0);
    fill(startVal, startVal+N_PARMS, 0.0);
    fill(useLimits, useLimits+N_PARMS, true);
    useLimits[PARM_LAN_AREA] = false;
    useLimits[PARM_BKGND_HEIGHT] = false;

    //----- fitting range

    fitRange[0]   = 0.0; fitRange[1] = 100.0;

    //----- parameters limits

    pllo[PARM_LAN_WID]      = .5;     plhi[PARM_LAN_WID] = 5.0;
    pllo[PARM_MPV]          = 10.0;   plhi[PARM_MPV] = 70.0;
    pllo[PARM_GAU_WID]      = .1;    plhi[PARM_GAU_WID] = 10.0;

    //----- parameters starting values

    startVal[PARM_LAN_WID]        = 1.6;
    startVal[PARM_MPV]            = 30.0;
    startVal[PARM_LAN_AREA]       = 5000.0;
    startVal[PARM_GAU_WID]        = 1.9;
    startVal[PARM_BKGND_HEIGHT]   = 1;

    TF1 *ffit = new TF1("langau", langaufun, fitRange[0], fitRange[1], N_PARMS);
    ffit->SetParameters(startVal);
    ffit->SetParNames("Landau width", "MP", "Area", "Gaussian #sigma", "Background Level");

    for (int i = 0; i < N_PARMS; i++)
      if (useLimits[i])
        ffit->SetParLimits(i, pllo[i], plhi[i]);

    ffit->SetNpx(1000);

    return ffit;
  }

  TF1 *buildLangauADC() {
    //// ADC VERSION (different parameter scaling) /////
    double fitRange[2], 
      pllo[N_PARMS], 
      plhi[N_PARMS], 
      startVal[N_PARMS];
    // whether or not to use limits in fit.
    bool useLimits[N_PARMS];
        
    fill(pllo, pllo+N_PARMS, 0.0);
    fill(plhi, plhi+N_PARMS, 0.0);
    fill(startVal, startVal+N_PARMS, 0.0);
    fill(useLimits, useLimits+N_PARMS, true);
    useLimits[PARM_LAN_AREA] = false;
    useLimits[PARM_BKGND_HEIGHT] = false;

    //----- fitting range

    fitRange[0]   = 0.0; fitRange[1] = 1200.0;

    //----- parameters limits

    pllo[PARM_LAN_WID]      = 1.0;     plhi[PARM_LAN_WID]      = 100;
    pllo[PARM_MPV]          = 100.0;   plhi[PARM_MPV]          = 800.0;
    pllo[PARM_GAU_WID]      = 2;       plhi[PARM_GAU_WID]      = 100.0;
    

    //----- parameters starting values

    startVal[PARM_LAN_WID]      = 20;
    startVal[PARM_MPV]          = 350.0;
    startVal[PARM_LAN_AREA]      = 5000.0;
    startVal[PARM_GAU_WID]      = 20;
    startVal[PARM_BKGND_HEIGHT] = 1;

    TF1 *ffit_adc = new TF1("langau_adc", langaufun, fitRange[0], fitRange[1], N_PARMS);
    ffit_adc->SetParameters(startVal);
    ffit_adc->SetParNames("Landau width", "MP", "Area", "Gaussian #sigma", "Background level");

    for (int i = 0; i < N_PARMS; i++)
      if (useLimits[i])
        ffit_adc->SetParLimits(i, pllo[i], plhi[i]);

    ffit_adc->SetNpx(1000);

    return ffit_adc;
  }

  /// use static auto_ptr so that singletons are properly destroyed on exit.
  /// not entirely necessary, but keeps things clean
  std::auto_ptr<TF1> langauDAC;

  /// use static auto_ptr so that singletons are properly destroyed on exit.
  /// not entirely necessary, but keeps things clean
  std::auto_ptr<TF1> langauADC;

  
};

/// retrieve gaussian convolved landau fuction with limits & initial values appropriate for LE CIDAC scale
TF1 &::LangauFun::getLangauDAC() {
  if (!langauDAC.get()) langauDAC.reset(buildLangauDAC());
  return *(langauDAC.get());
}

/// retrieve gaussian convolved landau fuction with limits & initial values appropriate for LEX8 ADC scale
TF1 &::LangauFun::getLangauADC() {
  if (!langauADC.get()) langauADC.reset(buildLangauADC());
  return *(langauADC.get());
}
