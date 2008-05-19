// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/CIDAC2ADC/smoothCIDAC2ADC.cxx,v 1.1 2008/05/14 18:39:46 fewtrell Exp $

/** @file Generate smoothed IntNonlin (cidac2adc) curves from raw
    cidac2adc points, expects adcmean output from genCIDAC2ADC as input.
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "IntNonlinAlg.h"
#include "src/lib/Util/CfgMgr.h"
#include "src/lib/Util/string_util.h"
#include "src/lib/Util/CGCUtil.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/SimpleCalCalib/CIDAC2ADC.h"

// EXTLIB INCLUDES
#include "TF1.h"
#include "TGraph.h"

// STD INCLUDES
#include <iostream>
#include <string>
#include <fstream>

using namespace std;
using namespace calibGenCAL;
using namespace CalUtil;
using namespace CfgMgr;

namespace {
  /// how many points for each smoothing 'group'?  (per adc range)
  static const unsigned short smoothGrpWidth[] = {
    3, 4, 3, 4
  };
  /// how many points at beginning of curve to extrapolate from following points
  static const unsigned short extrapPtsLo[]    = {
    2, 2, 2, 2
  };
  /// how many points to extrapolate beginning of curve _from_
  static const unsigned short extrapPtsLoFrom[] = {
    5, 5, 5, 5
  };
  /// how many points at end of curve not to smooth (simply copy them over verbatim from raw data)
  static const unsigned short smoothSkipHi[]   = {
    6, 10, 6, 10
  };

  /// smooth individual IntNonlin spline
  void smoothSpline(const vector<float> &curADC,
                    vector<float> &splineADC,
                    vector<float> &splineDAC,
                    const RngNum rng
                    ) {
    // following vals only change w/ rng, so i get them outside the other loops.
    const unsigned short grpWid       = smoothGrpWidth[rng.val()];
    const unsigned short extrapLo     = extrapPtsLo[rng.val()];
    const unsigned short extrapLoFrom = extrapPtsLoFrom[rng.val()];
    const unsigned short skpHi        = smoothSkipHi[rng.val()];


    // 2 dimensional poly line f() to use for spline fitting.
    float *tmpADC(new float[singlex16::N_CIDAC_VALS]);

    TF1 splineFunc("spline_fitter",
                   "pol2",
                   0,
                   singlex16::MAX_DAC);

    //-- GET UPPER ADC BOUNDARY for this channel --//
    const float adc_max  = curADC[singlex16::N_CIDAC_VALS-1];
    // last idx will be last index that is <= 0.99*adc_max
    // it is the last point we intend on using.
    unsigned short last_idx = 0;
    while (curADC[last_idx] <= 0.99*adc_max  
           && 
           last_idx < singlex16::N_CIDAC_VALS-1) //bounds check
      last_idx++;
    if (last_idx > 0)
      last_idx--;

    //-- CREATE GRAPH OBJECT for fitting --//
    copy(curADC.begin(),
         curADC.end(),
         tmpADC);
    TGraph myGraph(last_idx+1,
                   singlex16::CIDAC_TEST_VALS,
                   tmpADC);

    // PART I: EXTRAPOLATE INITIAL POINTS FROM MEAT OF CURVE
    for (unsigned short i = 0; i < extrapLo; i++) {
      // put new DAC val on global output list
      const float &  dac  = singlex16::CIDAC_TEST_VALS[i];
      splineDAC.push_back(dac);

      // extrapolate associated adc value from points later in curve
      // 1st non extrapolated point
      const unsigned short pt2  = extrapLo;
      // n points into curve from pt2
      const unsigned short pt1  = pt2+extrapLoFrom-1;

      const float &  dac2 = singlex16::CIDAC_TEST_VALS[pt2];
      const float &  adc2 = curADC[pt2];

      const float &  dac1 = singlex16::CIDAC_TEST_VALS[pt1];
      const float &  adc1 = curADC[pt1];

      float adc  = linear_extrap(dac1, dac2, dac,
                                 adc1, adc2);

      splineADC.push_back(adc);
    }

    // PART II: GROUP & SMOOTH MIDDLE RANGE  ///////////////////////////////
    // start one grp above skiplo & go as hi as you can w/out entering skpHi
    for (unsigned short ctrIdx = extrapLo + grpWid - 1;
         ctrIdx <= last_idx - max<unsigned short>(skpHi, grpWid-1);  // allow last group to overlap skpHi section.

         ctrIdx += grpWid) {
      const unsigned short lp  = ctrIdx - grpWid;                          // 1st point in group
      const unsigned short hp  = ctrIdx + grpWid;                          // last point in group

      // fit curve to grouped points
      myGraph.Fit(&splineFunc, "QN", "",
                  singlex16::CIDAC_TEST_VALS[lp],
                  singlex16::CIDAC_TEST_VALS[hp]);
      const float myPar1 = splineFunc.GetParameter(0);
      const float myPar2 = splineFunc.GetParameter(1);
      const float myPar3 = splineFunc.GetParameter(2);

      // use DAC value from center point
      const float fitDAC = singlex16::CIDAC_TEST_VALS[ctrIdx];
      // eval smoothed ADC value
      const float fitADC = myPar1 + fitDAC*(myPar2 + fitDAC*myPar3);

      // put new ADC val on list
      splineADC.push_back(fitADC);

      // put new DAC val on global output list
      splineDAC.push_back(fitDAC);
    }

    // PART III: NO SMOOTHING/GROUPING FOR LAST SKPHI PTS //////////////////
    // copy SKPHI points directly from face of array.
    for (unsigned short i = last_idx+1 - skpHi;
         i <= last_idx;
         i++) {
      // put new ADC val on list
      splineADC.push_back(curADC[i]);

      // put new DAC val on global output list
      splineDAC.push_back(singlex16::CIDAC_TEST_VALS[i]);
    }

    //-- EXTRAPOLATE FINAL POINT --//
    const unsigned short nPts = splineADC.size();
    const float dac1          = splineDAC[nPts-2];
    const float dac2          = splineDAC[nPts-1];
    const float adc1          = splineADC[nPts-2];
    const float adc2          = splineADC[nPts-1];
    const float slope         = (dac2 - dac1)/(adc2 - adc1);
    const float dac_max       = dac2 + (adc_max - adc2)*slope;

    // put final point on the list.
    splineADC.push_back(adc_max);
    splineDAC.push_back(dac_max);

    delete [] tmpADC;
  }

  /// generate smoothed versions for each IntNonlin channel
  void smoothSplinePts(const CIDAC2ADC &adcMeans,
                       CIDAC2ADC &cidac2adc) {
    // Loop through all 4 energy ranges
    for (RngNum rng; rng.isValid(); rng++)
      // loop through each xtal face.
      for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++) {
        const RngIdx rngIdx(faceIdx,
                            rng);

        // skip missing channels
        if (adcMeans.getPtsADC(rngIdx).empty())
          continue;

        // point to current adc vector
        const vector<float> &curADC = adcMeans.getPtsADC(rngIdx);

        // point to output splines
        vector<float> &splineADC = cidac2adc.getPtsADC(rngIdx);
        vector<float> &splineDAC = cidac2adc.getPtsDAC(rngIdx);

        smoothSpline(curADC, splineADC, splineDAC, rng);
      } // xtalFace lop
    // range loop

    // subtract pedestals
    cidac2adc.pedSubtractADCSplines();
  }

};

/// Manage application configuration parameters
class AppCfg {
public:
  AppCfg(const int argc,
         const char **argv) :
    cmdParser(path_remove_ext(__FILE__)),
    adcmeanPath("adcmean",
                "input cidac2adc data",
                ""),
    outputBasename("outputBasename",
                   "all output files will use this basename + some_ext",
                   "")
  {
    cmdParser.registerArg(adcmeanPath);
    cmdParser.registerArg(outputBasename);

    try {
      cmdParser.parseCmdLine(argc, argv);
    } catch (exception &e) {
      cmdParser.printUsage();
      exit(-1);
    }

  }
  /// construct new parser
  CmdLineParser cmdParser;
  
  CmdArg<string> adcmeanPath;
  CmdArg<string> outputBasename;

};

int main(int argc,
         const char **argv) {
  // libCalibGenCAL will throw runtime_error
  try {
    AppCfg cfg(argc,argv);

    //-- SETUP LOG FILE --//
    /// multiplexing output streams
    /// simultaneously to cout and to logfile
    LogStrm::addStream(cout);
    // generate logfile name
    const string logfile(cfg.outputBasename.getVal() + ".log.txt");
    ofstream tmpStrm(logfile.c_str());
    LogStrm::addStream(tmpStrm);

    //-- LOG SOFTWARE VERSION INFO --//
    output_env_banner(LogStrm::get());
    LogStrm::get() << endl;
    cfg.cmdParser.printStatus(LogStrm::get());
    LogStrm::get() << endl;

    /// txt output filename
    const string outputTXTPath(cfg.outputBasename.getVal() + ".txt");

    CIDAC2ADC    adcMeans;
    CIDAC2ADC    cidac2adc;

    LogStrm::get() << __FILE__ << ": reading adc means from txt file: "
                   << cfg.adcmeanPath.getVal() << endl;
    adcMeans.readTXT(cfg.adcmeanPath.getVal());
    
    LogStrm::get() << __FILE__ << ": generating smoothed spline points: " << endl;
    smoothSplinePts(adcMeans, cidac2adc);

    LogStrm::get() << __FILE__ << ": writing smoothed spline points: " << outputTXTPath << endl;
    cidac2adc.writeTXT(outputTXTPath);

    LogStrm::get() << __FILE__ << "Successfully completed." << endl;
  } catch (exception &e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
    return -1;
  }

  return 0;
}

