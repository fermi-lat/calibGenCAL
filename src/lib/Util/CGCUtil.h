#ifndef CGCUtil_H
#define CGCUtil_H

//$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Util/CGCUtil.h,v 1.4 2007/05/25 21:06:48 fewtrell Exp $

// LOCAL INCLUDES

// GLAST INCLUDES

// EXTLIB INCLUDES
#include "TFile.h"
#include "TKey.h"
#include "TF1.h"
#include "TDirectory.h"
#include "TROOT.h"
#include "TList.h"

// STD INCLUDES
#include <ostream>
#include <vector>
#include <streambuf>
#include <cmath>
#include <memory>
#include <algorithm>
#include <sstream>
#include <string>
#include <functional>
#include <cassert>
#include <iterator>

/** @file CGCUtil.h
    @author Zachary Fewtrell
    \brief Generic utility functions for calibGenCAL pkg
*/

namespace calibGenCAL {
  namespace CGCUtil {
    extern const std::string CVS_TAG;
    extern const std::string CGC_DEFAULT_CFGPATH;
    
    /// append elements of RH collection to LH collection
    template <class LHContainer, class RHContainer>
    void append(LHContainer &lhs, const RHContainer &rhs) {
      std::copy(rhs.begin(),
                rhs.end(),
                std::back_inserter(lhs));
    }
                     
    /// Template function fills any STL type container with zero values
    template <class T> void fill_zero(T &container) {
      fill(container.begin(), container.end(), 0);
    }
                     
    /** \brief splits a delmiited string into a vector of shorter token-strings
        stolen from http://www.linuxselfhelp.com/HOWTO/C++Programming-HOWTO-7.html
    */
    std::vector<std::string> tokenize_str(const std::string & str,
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
    class LogStrm {
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
      TKey *const key(rootDir.FindKey(objname.c_str()));
   
      // skip missing hist
      if (!key)
        return NULL;
              
      const TClass *const cls(gROOT->GetClass(key->GetClassName()));
      if (!cls)
        return NULL;
                           
      T  *const hist_ptr(dynamic_cast<T *>(key->ReadObj()));
      if (!hist_ptr) return NULL;
                                              
      // skip hist if it's the wrong type
      if (!hist_ptr->InheritsFrom(T::Class()))
        return NULL;
                                                                                        
      return hist_ptr;
    }

    /// return collection of all ROOT objects below given dir which match given type & string prefix.
    template <class T>
    std::vector<T*> harvestROOTObjs(const TDirectory &rootDir,
                                    const std::string &prefix="") {

      /// retrieve list of objects in dir
      TList *const keyList = rootDir.GetListOfKeys();
      /// critical error
      assert(keyList != 0);

      /// construct (initially empty) list to return
      std::vector<T*> retVal;

      /// iterate through list.
      TIterator *keyIter = keyList->MakeIterator();
      while (TKey *const key = dynamic_cast<TKey *const>(keyIter->Next())) {
        const TClass *const cls(gROOT->GetClass(key->GetClassName()));
        assert(cls != 0);

        /// if key is sub dir, recursively scan it
        if (cls->InheritsFrom(TDirectory::Class())) {
          TDirectory *const subdir = dynamic_cast<TDirectory *const>(key->ReadObj());
          append(retVal, harvestROOTObjs<T>(*subdir, prefix));          
        }

        /// if key is correct type, check name match & append it to list
        else if (cls->InheritsFrom(T::Class())) {
          /// check name prefix
          if (!prefix.empty()) {
            const std::string name(key->GetName());
            
            /// skip if name does not match
            if (name.find(prefix) != 0)
              continue;

            /// finally add obj to list
            retVal.push_back(dynamic_cast<T*>(key->ReadObj()));
          }
        }
      }

      return retVal;
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
                       
  
    /// generic type2string converter
    template <typename T>
    std::string toString(const T &val) {
      std::ostringstream tmp;
      tmp << val;
      return tmp.str();
    }

    /// execute functor(ptr) only if ptr != 0                                                                                                                                                                  
    template <class _Functor>
    struct ifptr_fun_t : public std::unary_function<typename _Functor::argument_type,
                                                    typename _Functor::result_type>
    {
    private:
      _Functor op;

    public:
      typedef typename _Functor::result_type result_type;
      typedef typename _Functor::argument_type argument_type;

      explicit ifptr_fun_t(const _Functor &_op) : op(_op) {}

      result_type operator() (const argument_type &x) const {
        if (x != 0)
          return op(x);
      }
    };

    /// return ifptr_fun_t obj
    template <class _Functor>
    ifptr_fun_t<_Functor> ifptr_fun( const _Functor &op) {
      return ifptr_fun_t<_Functor>(op);
    }

    /// return ROOT TDirectory *, create subfolders as needed
    TDirectory *deliverROOTDir(TDirectory *const parent,
                               const std::string &childPath);

  };

}; // namespace calibGenCAL
#endif // CGCUtil_H
