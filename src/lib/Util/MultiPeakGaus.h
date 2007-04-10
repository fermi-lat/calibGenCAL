#ifndef MultiPeakGaus_h
#define MultiPeakGaus_h

// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Util/MultiPeakGaus.h,v 1.1 2007/03/28 17:48:37 fewtrell Exp $

// LOCAL INCLUDES

// GLAST INCLUDES

// EXTLIB INCLUDES

// STD INCLUDES
#include <set>

class TH1;

/** @file
    @author Zach Fewtrell
*/

/** @brief Factory class provides pointers ROOT TF1 function objects which fit sum of
    n gaussian peaks to a ROOT histogram
    
    Fit parameters:
    - par[0] = mpv scaling factor for Z=1 
    - par[1 ...   n ] = ratio of each successive gaussian mpv to z=1
    - par[n+1 ...  2n ] = width of each gaussian
    - par[2n+1 ... 3n ] = gaussian area
*/
class MultiPeakGaus {
 public:
  typedef std::set<unsigned short> ZSet;

  /// fit given histogram for given list of Z particles
  /// \param hist histogram to fit
  /// \param zSet list of particle z peaks to fit for
  /// \arapm initialMPD initial guess @ mevPerDAC (should help fitting converge)
  static void fitHist(TH1 &hist, const ZSet &zSet, float initialMPD);
};

#endif
