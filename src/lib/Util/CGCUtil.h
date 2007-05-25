#ifndef CGCUtil_H
#define CGCUtil_H

//$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Util/CGCUtil.h,v 1.3 2007/04/19 15:16:19 fewtrell Exp $

// LOCAL INCLUDES

// GLAST INCLUDES

// EXTLIB INCLUDES
#include "TFile.h"
#include "TKey.h"
#include "TF1.h"
#include "TDirectory.h"
#include "TROOT.h"

// STD INCLUDES
#include <ostream>
#include <vector>
#include <streambuf>
#include <cmath>
#include <memory>
#include <algorithm>
#include <sstream>

/** @file CGCUtil.h
    @author Zachary Fewtrell
    \brief Generic utility functions for calibGenCAL pkg
*/

namespace calibGenCAL {
  namespace CGCUtil {
    extern const std::string CVS_TAG;
    extern const std::string CGC_DEFAULT_CFGPATH;


    /// Template function fills any STL type container with zero values
    template <class T> void fill_zero(T &container) {
      fill(container.begin(), container.end(), 0);
    }

    /** \brief splits a delmiited string into a vector of shorter token-strings
        stolen from http://www.linuxselfhelp.com/HOWTO/C++Programming-HOWTO-7.html
    */
    void          tokenize_str(const std::string & str,
                               std::vector<std::string> & tokens,
                               const std::string & delimiters = " ");

    /// remove directory portion of full path
    /// \param path is unaltered
    /// \return new string object
    std::string   path_remove_dir(std::string path);

    /// remove filename extention portion of path
    /// \param path is unaltered
    /// \return new string object  
    std::string path_remove_ext(std::string path);

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
    bool stringToBool(const std::string &str);

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

    /// use this method to retrieve a histogram of given
    /// type and name out of a root file
    /// \return ptr to hist obj if successful, NULL ptr otherwise
    template <class T>
    T *retrieveROOTObj(const TDirectory &rootDir,
                       const std::string &objname) {
      TKey *const key = rootDir.FindKey(objname.c_str());
      
      // skip missing hist
      if (!key)
        return NULL;

      const TClass *const cls = gROOT->GetClass(key->GetClassName());
      if (!cls)
        return NULL;

      T  *const hist_ptr = dynamic_cast<T *>(key->ReadObj());
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
      const std::string name(fittedHist.GetName());

      const std::string title(fittedHist.GetTitle());

      const int    nBins = fittedHist.GetNbinsX();
      const double xlo   = fittedHist.GetXaxis()->GetXmin();
      const double xhi   = fittedHist.GetXaxis()->GetXmax();

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
                         const FwdIt stop,
                         const std::string &delim = " ") {
      std::ostringstream tmp;


      while (start != stop) {
        tmp << *start;
        tmp << delim;
        start++;
      }

      return tmp.str();
    }

    /// return vector of each lin in txt file.
    std::vector<std::string> getLinesFromFile(const std::string &filename);

    /// iterator helper class for stl map allows you to index through std::map::mapped_type instead of std::map::value_type (which is sometimes annoying std::pair))
    template <typename MapType>
    class map_val_iterator {
    public:
      explicit map_val_iterator(const typename MapType::iterator &it) :
        it(it) {
      }
    
      typename MapType::mapped_type &operator*() {
        return it->second;
      }
    
      const typename MapType::mapped_type &operator*() const {
        return it->second;
      }
    
      /// prefix ++ operator
      map_val_iterator operator++() {
        map_val_iterator tmp(*this);
        it++;
        return tmp;
      }

      /// postfix ++ operator
      map_val_iterator &operator++(int) {
        it++;
        return *this;
      }

      bool operator!=(const map_val_iterator &that) const {
        return it != that.it;
      }

    private:
      typename MapType::iterator it;
    };
  };
}; // namespace calibGenCAL
#endif // CGCUtil_H
