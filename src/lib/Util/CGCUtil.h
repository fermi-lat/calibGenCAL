#ifndef CGCUtil_H
#define CGCUtil_H

//$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Util/CGCUtil.h,v 1.10 2008/05/13 16:54:01 fewtrell Exp $

// LOCAL INCLUDES

// GLAST INCLUDES

// EXTLIB INCLUDES

// STD INCLUDES
#include <string>
#include <cmath>
#include <vector>

/** @file CGCUtil.h
    @author Zachary Fewtrell
    \brief Generic utility functions for calibGenCAL pkg
*/

namespace calibGenCAL {
  /// return string representing calibGenCAL CVS tag #.
  std::string CVS_TAG();
  
  /// return stirng with default cfgFile path for calibGenCAL
  std::string CGC_DEFAULT_CFGPATH();
                     
  /// logStream will support parallel output to mutitple ostream classes
  /// (as many as are added by the addStream method)
  ///
  class LogStrm {
  public:
    static std::ostream & get();
    static void           addStream(std::ostream &strm);
  };
                     
  /// Output string w/ username, hostname, time, relevant CMT package versions
  /// & paths to ostream
  /// output is in multi line text format
  void  output_env_banner(std::ostream &ostrm);
                     
                     
  /** return p3 such that p3 - p2 = p2 - p1
   */
  template <class Ty>
  inline Ty extrap(const Ty &p1,
                   const Ty &p2) {
    return 2*p2 - p1;
  }
                     
  /** return y3 such that (y2 - y1)/(x2 - x1) = (y3 - y2)/(x3 - x2)
   */
  template <class Ty>
  inline Ty linear_extrap(const Ty &x1,
                          const Ty &x2,
                          const Ty &x3,
                          const Ty &y1,
                          const Ty &y2) {
    return (x3-x2)*(y2-y1)/(x2-x1) + y2;
  }
                     
  inline double degreesToRadians(const double &degrees) {
    return degrees*M_PI/180;
  }
                     
  /// return true if min <= x <= max (inclusive)
  template <typename T>
  bool between_incl(const T& min,
               const T& x,
               const T& max) {
    return (x >= min && x <= max);
  }

}; // namespace calibGenCAL
#endif // CGCUtil_H
