#ifndef CGCUtil_H
#define CGCUtil_H

//$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/CGCUtil.h,v 1.2 2006/06/22 21:50:22 fewtrell Exp $

// LOCAL INCLUDES

// GLAST INCLUDES

// EXTLIB INCLUDES
#include "TFile.h"
#include "TKey.h"
#include "TClass.h"

// STD INCLUDES
#include <string>
#include <vector>
#include <ostream>

/** @file CGCUtil.h
    @author Zachary Fewtrell
    \brief Generic utility functions for calibGenCAL pkg
*/

using namespace std;

namespace CGCUtil {

  const string CVS_TAG("$Name:  $");

  /// Template function fills any STL type container with zero values
  template <class T> static void fill_zero(T &container) {
    fill(container.begin(), container.end(), 0);
  }

  void tokenize_str(const string& str,
                    vector<string>& tokens,
                    const string& delimiters = " ");

  string &path_remove_dir(string &path);

  string &path_remove_ext(string &path);



  typedef vector<ostream*> streamvector;
  /** wrapper class for treating multiple streambuf objects as one 
   *
   * used by multiplexor_ostream
   */
  class multiplexor_streambuf : public streambuf {
  public:
    multiplexor_streambuf() : streambuf() {}
  
    virtual int overflow(int c) {
      // write the incoming character into each stream
      streamvector::iterator _b = _streams.begin(), _e = _streams.end();
      for(; _b != _e; _b++)
        (*_b)->put(c);
    
      return c;
    }
  
  public:
    streamvector _streams;
  };

  /** ostream class will write to any number of streambuffers simultaneously
   *
   * taken from here: 
   * http://www.gamedev.net/community/forums/topic.asp?topic_id=286078
   *
   * CoffeeMug  GDNet+  Member since: 3/25/2003  From: NY, USA
   * Posted - 12/1/2004 9:41:18 PM
   *
   *
   */
  class multiplexor_ostream : public ostream
    {
    public:
      multiplexor_ostream() : ios(0), ostream(new multiplexor_streambuf()){}
      virtual ~multiplexor_ostream() { delete rdbuf(); }
  
      streamvector& getostreams() { 
        return ((multiplexor_streambuf*)rdbuf())->_streams; 
      }
    };

  /// Output string w/ username, hostname, time, relevant CMT package versions 
  /// & paths to ostream
  /// output is in multi line text format
  void output_env_banner(ostream &ostrm);

  /** \brief convert string to uppercase
      \return ref to converted string
      \note operates in place on given string.
  */
  string &str_toupper(string &str);

  /** convert string to boolean.
      \return boolean interperetation of value
      \throws exception if value is not properly formatted.

      to be interpereted as boolean, value must be '1', '0', '[t]rue', '[f]alse', '[y]es', '[n]o'
      \note interperetation is case-insensitive.
  */
  bool stringToBool(const string &str);

  void genOutputFilename(const string &outputDir,
                         const string &outputCalibType,
                         const string &inputFilename,
                         const string &outputExt,
                         string &outputFilename);

  /// use this method to retrieve a histogram of given
  /// type and name out of a root file
  /// \return ptr to hist obj if successful, NULL ptr otherwise
  template <class T>
    T* retrieveHist(TFile &histFile,
                    const string &histname) {
    TKey *key = histFile.FindKey(histname.c_str());
    // skip missing hist
    if (!key)
      return NULL;
      
    TClass *cls = gROOT->GetClass(key->GetClassName());
    if (!cls)
      return NULL;
      
    T *hist_ptr = (T*)key->ReadObj();
    if (!hist_ptr) return NULL;

    // skip hist if it's the wrong type
    if (!hist_ptr->InheritsFrom(T::Class())) 
      return NULL;

    return hist_ptr;
  } 

  /// return minimum value from an STL vector
  template<typename T> const T& max_val(const vector<T> &vec) {
    return *(max_element(vec.begin(),vec.end()));
  }

  /// return minimum value from an STL vector
  template<typename T> const T& min_val(const vector<T> &vec) {
    return *(min_element(vec.begin(),vec.end()));
  }

  /** return p3 such that p3 - p2 = p2 - p1
   */
  template <class Ty>
    inline Ty extrap(Ty p1, Ty p2) {
    return 2*p2 - p1;
  }


  /** return y3 such that (y2 - y1)/(x2 - x1) = (y3 - y2)/(x3 - x2)
   */
  template <class Ty>
    inline Ty linear_extrap(Ty x1, Ty x2, Ty x3, Ty y1, Ty y2) {
    return (x3-x2)*(y2-y1)/(x2-x1) + y2;
  }

};

#endif // CGCUtil_H
