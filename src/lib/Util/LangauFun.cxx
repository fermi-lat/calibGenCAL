// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Util/LangauFun.cxx,v 1.9 2008/04/25 17:55:11 fewtrell Exp $

/** @file
    @author Zach Fewtrell
*/
// LOCAL INCLUDES
#include "LangauFun.h"
#include "string_util.h"

// GLAST INCLUDES

// EXTLIB INCLUDES
#include "TF1.h"
#include "TH1.h"
#include "TNtuple.h"
#include "TMath.h"

// STD INCLUDES
#include <cmath>
#include <sstream>

namespace calibGenCAL {
  using namespace std;
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

    static const double startVals[N_PARMS] = {
      .0526, // LAN_WID
      30.81, //  MPV
      5000, // AREA
      .0618, // GAU_WID
      0.0 // BKG
    };

    static const bool   fixParms[N_PARMS] = {
      true, // LAN_WID
      false, // MPV
      false, // AREA
      true, // GAU_WID
      false // BKG
    };

    static const bool useLimits[N_PARMS] = {
      true, // LAN_WID
      true, // MPV
      true,  // AREA
      true, // GAU_WID
      true // BKG
    };

    static const double parmLo[N_PARMS] = {
      .01, // LAN_WID
      1,   // MPV
      0,   // AREA
      .05, // GAU_WID
      0    // BKG
    };

    static const double parmHi[N_PARMS] = {
      0.2,   // LAN_WID
      10000, // MPV
      1e9,   /// AREA
      0.08,  // GAU_WID
      1e6    // BKG
    };

    static const double fitRange[2] = {
      0, 100
    };

    /// \brief calculate background model for muon peak fit
    ///
    /// background is level @ height below peak mpv & 0 afterwards w/
    /// exponential decay of width sigma between two regions
    static inline double bckgnd_model(double x,
                                     double height,
                                     double mpv,
                                     double sigma) {
      return height/(1 + exp((x-mpv)/sigma));
    }

    static Double_t langaufun(Double_t *x,
                              Double_t *par)
    {
      // Numeric constants

      static const double invsq2pi    = pow(2*M_PI, -.5);      // (2 pi)^(-1/2)
      static const double mpshift     = -0.22278298;           // Landau maximum location

      // Control constants

      static const unsigned short np = 100;                   // number of convolution steps
      static const unsigned short sc = 5;                     // convolution extends to +-sc Gaussian sigmas

      // Variables

      double xx;
      double mpc;
      double fland;
      double sum = 0.0;
      double xlow, xupp;
      double step;
      unsigned short i;

      // landau & gausian wid given as fraction
      // of mpv
      double real_lan = par[PARM_LAN_WID]*par[PARM_MPV];
      double real_gau = par[PARM_GAU_WID]*par[PARM_MPV];


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
      double convolved_width = sqrt(real_gau*real_gau + real_lan*real_lan);
      double bkgnd  = bckgnd_model(x[0],
                                  par[PARM_BKGND_HEIGHT], // bckgnd height
                                  par[PARM_MPV],          // mpv
                                  convolved_width);

      double retVal = par[PARM_LAN_AREA]*step*sum*invsq2pi/real_gau + bkgnd;

      /*         LogStrm::get() << x[0] << " "
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
    tuple_def = tuple_def.substr(0, tuple_def.size()-1);

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


    tuple_data[FIELD_XTAL]        = xtalId.val();
    double mpv = func.GetParameter(PARM_MPV);
    tuple_data[FIELD_MPV]         = mpv;
    tuple_data[FIELD_MPV_ERR]     = func.GetParError(PARM_MPV);
    tuple_data[FIELD_LAN_WID]     = func.GetParameter(PARM_LAN_WID)*mpv;
    tuple_data[FIELD_LAN_WID_ERR] = func.GetParError(PARM_LAN_WID)*mpv;
    tuple_data[FIELD_GAU_WID]     = func.GetParameter(PARM_GAU_WID)*mpv;
    tuple_data[FIELD_GAU_WID_ERR] = func.GetParError(PARM_GAU_WID)*mpv;
    tuple_data[FIELD_BKGND]       = func.GetParameter(PARM_BKGND_HEIGHT);
    tuple_data[FIELD_BKGND_ERR]   = func.GetParError(PARM_BKGND_HEIGHT);
    tuple_data[FIELD_CHISQ]       = func.GetChisquare();
    tuple_data[FIELD_NDF] = func.GetNDF();
    tuple_data[FIELD_NENTRIES]    = hist.GetEntries();

    return tuple.Fill(tuple_data);
  }

}; // namespace calibGenCAL
