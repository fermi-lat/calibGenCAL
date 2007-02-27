#ifndef CGCUtil_H
#define CGCUtil_H

//$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/CGCUtil.h,v 1.9 2007/02/26 17:09:46 fewtrell Exp $

// LOCAL INCLUDES

// GLAST INCLUDES

// EXTLIB INCLUDES
#include "TFile.h"
#include "TKey.h"
#include "TF1.h"
#include "TDirectory.h"

// STD INCLUDES
#include <ostream>
#include <vector>
#include <streambuf>
#include <cmath>
#include <memory>
#include <algorithm>

/** @file CGCUtil.h
    @author Zachary Fewtrell
 \brief Generic utility functions for calibGenCAL pkg
 */

namespace CGCUtil {
  const std::string CVS_TAG("$Name:  $");

  /// Template function fills any STL type container with zero values
  template <class T> void fill_zero(T &container) {
    fill(container.begin(), container.end(), 0);
  }

  void          tokenize_str(const std::string & str,
                             std::vector<std::string> & tokens,
                             const std::string & delimiters = " ");

  /// remove directory portion of full path
  std::string   path_remove_dir(std::string path);

  /// remove filename extention portion of path
  std::string   path_remove_ext(std::string path);

  /// logStream will support parallel output to mutitple ostream classes
  /// (as many as are added by the addStream method)
  ///
  class LogStream {
public:
    static std::ostream & get();
    static void           addStream(std::ostream &strm);
  };

  /// Output string w/ username, hostname, time, relevant CMT package versions
  /// & paths to ostream
  /// output is in multi line text format
  void          output_env_banner(std::ostream &ostrm);

  /** \brief convert string to uppercase
   \return ref to converted string
   \note operates in place on given string.
   */
  std::string & str_toupper(std::string &str);

  /** convert string to boolean.
   \return boolean interperetation of value
   \throws exception if value is not properly formatted.

      to be interpereted as boolean, value must be '1', '0', '[t]rue', '[f]alse', '[y]es', '[n]o'
   \note interperetation is case-insensitive.
   */
  bool          stringToBool(const std::string &str);

  std::string   genOutputFilename(const std::string &outputDir,
                                  const std::string &outputCalibType,
                                  const std::string &inputFilename,
                                  const std::string &outputExt);

  /// return minimum value from an STL vector
  template<typename T> const T & max_val(const std::vector<T> &vec) {
    return *(max_element(vec.begin(), vec.end()));
  }

  /// return minimum value from an STL vector
  template<typename T> const T & min_val(const std::vector<T> &vec) {
    return *(min_element(vec.begin(), vec.end()));
  }

  /** return p3 such that p3 - p2 = p2 - p1
   */
  template <class Ty>
  inline Ty extrap(Ty p1,
                   Ty p2) {
    return 2*p2 - p1;
  }

  /** return y3 such that (y2 - y1)/(x2 - x1) = (y3 - y2)/(x3 - x2)
   */
  template <class Ty>
  inline Ty linear_extrap(Ty x1,
                          Ty x2,
                          Ty x3,
                          Ty y1,
                          Ty y2) {
    return (x3-x2)*(y2-y1)/(x2-x1) + y2;
  }

  inline double degreesToRadians(const double degrees) {
    return degrees*M_PI/180;
  }

  /// use this method to retrieve a histogram of given
  /// type and name out of a root file
  /// \return ptr to hist obj if successful, NULL ptr otherwise
  template <class T>
  T *retrieveHist(const TDirectory &rootDir,
                  const std::string &histname) {
    TKey *key = rootDir.FindKey(histname.c_str());


    // skip missing hist
    if (!key)
      return NULL;

    TClass *cls      = gROOT->GetClass(key->GetClassName());
    if (!cls)
      return NULL;

    T      *hist_ptr = (T *)key->ReadObj();
    if (!hist_ptr) return NULL;

    // skip hist if it's the wrong type
    if (!hist_ptr->InheritsFrom(T::Class()))
      return NULL;

    return hist_ptr;
  }

  /// create new 1D histogram w/ residuals from fitted 1D histogram
  /// and 1st TF1 on histogram list-of-fuctions
  template<class T>
  T *createResidHist(const T &fittedHist) {
    // retrieve previous histogram info
    std::string name(fittedHist.GetName());

    std::string title(fittedHist.GetTitle());

    int    nBins = fittedHist.GetNbinsX();
    double xlo   = fittedHist.GetXaxis()->GetXmin();
    double xhi   = fittedHist.GetXaxis()->GetXmax();

    name  += "_resid";
    title += "_resid";

    T *    resid = new T(name.c_str(),
                         title.c_str(),
                         nBins,
                         xlo,
                         xhi);

    resid->Add(&fittedHist);
    resid->Add(dynamic_cast<TF1*>(fittedHist.GetListOfFunctions()->At(0)),
               -1);

    return resid;
  }

  /// create new sub-directory inside of parent if it doesn't exist
  /// \return ref to new subdir regardless of whether I had to make one or not.
  /// \note leave pwd unchanged
  TDirectory &root_safe_mkdir(TDirectory &parent,
                              const std::string &dirName);

  /// template method joins a sequence of data items ino
  /// a string, separating each by delim.
  template <class FwdIt>
  std::string str_join(FwdIt start,
                       FwdIt stop,
                       const std::string &delim = " ") {
    std::ostringstream tmp;


    while (start != stop) {
      tmp << *start;
      tmp << delim;
      start++;
    }

    return tmp.str();
  }
};
#endif // CGCUtil_H
