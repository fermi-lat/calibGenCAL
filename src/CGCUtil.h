#ifndef CGCUtil_H
#define CGCUtil_H

// LOCAL INCLUDES

// GLAST INCLUDES

// EXTLIB INCLUDES

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

  const string CVS_TAG("$Name: v3r8p2 $");

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
  
      streamvector& getostreams() { return ((multiplexor_streambuf*)rdbuf())->_streams; }
    };

  /// Output string w/ username, hostname, time, relevant CMT package versions & paths
  /// to ostream
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
};
#endif // CGCUtil_H
