#ifndef CGCUtil_H
#define CGCUtil_H 1

// LOCAL INCLUDES

// GLAST INCLUDES

// EXTLIB INCLUDES

// STD INCLUDES
#include <string>
#include <vector>
#include <ostream>

using namespace std;

namespace CGCUtil {

  const string CVS_TAG("$Name: v3r1p5 $");

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
      multiplexor_ostream() : ostream(new multiplexor_streambuf()), ios(0) {}
      virtual ~multiplexor_ostream() { delete rdbuf(); }
  
      streamvector& getostreams() { return ((multiplexor_streambuf*)rdbuf())->_streams; }
    };
};
#endif // CGCUtil_H
