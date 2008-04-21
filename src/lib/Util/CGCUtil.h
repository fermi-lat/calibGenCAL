#ifndef CGCUtil_H
#define CGCUtil_H

//$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Util/CGCUtil.h,v 1.6 2007/06/13 22:42:13 fewtrell Exp $

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
  extern const std::string CVS_TAG;
  extern const std::string CGC_DEFAULT_CFGPATH;
                     
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
                     
  /// return vector of each lin in txt file.
  std::vector<std::string> getLinesFromFile(const std::string &filename);

  std::string to_str(const int n);

}; // namespace calibGenCAL
#endif // CGCUtil_H
