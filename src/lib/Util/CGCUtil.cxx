/** @file CGCUtil.cxx

@author Zachary Fewtrell

\brief generic utility functions used in calibGenCAL pkg

$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Util/CGCUtil.cxx,v 1.2 2007/04/10 14:51:02 fewtrell Exp $
*/

// LOCAL INCLUDES
#include "CGCUtil.h"

// GLAST INCLUDES

// EXTLIB INCLUDES

// STD INCLUDES
#include <algorithm>
#include <iostream>
#include <sstream>
#include <ctime>
#include <stdexcept>
#include <fstream>

namespace calibGenCAL {

  using namespace std;

  namespace CGCUtil {
    const string CVS_TAG("$Name:  $");
    const string CGC_DEFAULT_CFGPATH("$(CALIBGENCALROOT)/cfg/defaults.cfg");


    void tokenize_str(const string & str,
                      vector<string> & tokens,
                      const string & delims)
    {
      // Skip delims at beginning.
      string::size_type lastPos = str.find_first_not_of(delims, 0);
      // Find first "non-delimiter".
      string::size_type pos     = str.find_first_of(delims, lastPos);


      while (string::npos != pos || string::npos != lastPos)
        {
          // Found a token, add it to the vector.
          tokens.push_back(str.substr(lastPos, pos - lastPos));
          // Skip delims.  Note the "not_of"
          lastPos = str.find_first_not_of(delims, pos);
          // Find next "non-delimiter"
          pos     = str.find_first_of(delims, lastPos);
        }
    }

    /// finds position of last directory delimeter ('/' || '\\')
    /// in path, returns path.npos if no delim is found
    string::size_type path_find_last_delim(const string &path) {
      // find last directory delimeter.
      const string::size_type fwdslash_pos(path.find_last_of('/'));
      const string::size_type bckslash_pos(path.find_last_of('\\'));


      // check for 'not found' cases
      if (fwdslash_pos == path.npos)
        return (bckslash_pos == path.npos) ? path.npos : bckslash_pos;
      if (bckslash_pos == path.npos)
        return (fwdslash_pos == path.npos) ? path.npos : fwdslash_pos;

      return max(fwdslash_pos, bckslash_pos);
    }

    string::size_type slash_pos;


    string path_remove_dir(string path) {
      // if there was no delimeter, return path unaltered
      if ((slash_pos = path_find_last_delim(path)) == path.npos)
        return path;

      // else remove everything up to & including the delimeter
      path.erase(0, slash_pos+1);

      return path;
    }

    string path_remove_ext(string path) {
      // return path unaltered if there is no '.'
      string::size_type dot_pos;


      if ((dot_pos = path.find_last_of('.')) == path.npos)
        return path;

      // find last delim (extension must be after this point)
      string::size_type slash_pos = path_find_last_delim(path);

      // if there is no '/' then just erase everything from '.' onward
      // or if slash is before the '.'
      if (slash_pos == path.npos || slash_pos < dot_pos)
        path.erase(dot_pos, path.size());

      return path;
    }

    void output_env_banner(ostream &ostrm) {
      // GENERATE TIME STRING
      char time_str[128];
      const time_t tmt = time(NULL);
      const struct tm * const tm_now = localtime(&tmt);


      if (strftime(time_str, sizeof(time_str),
                   "%c %z", tm_now) == 0) {
        strcpy(time_str, "");   // error case
        cerr << __FILE__  << ':'     << __LINE__ << ' '
             << "WARNING. error generating time string!" << endl;
      }

      // GENERATE PACKAGE PATH STRINGS

      ostrm << "************** ENVIRONMENT SUMMARY *****************" << endl;
      ostrm << " RUNTIME : " << time_str << endl;
      ostrm << endl;

      // test that enviroment variables are present
      if (!getenv("CALIBGENCALROOT") ||
          !getenv("ROOTROOT")        ||
          !getenv("DIGIROOTDATAROOT") ||
          !getenv("IDENTSROOT")      ||
          !getenv("CALIBUTILROOT"))
        cerr << __FILE__  << ':'     << __LINE__ << ' '
             << "WARNING. error retrieveing packageROOT paths" << endl;
      else {
        ostrm << " PACKAGE      "  << "PATH" << endl;
        ostrm << " calibGenCAL  "  << getenv("CALIBGENCALROOT")  << endl;
        ostrm << " ROOT         "  << getenv("ROOTROOT")         << endl;
        ostrm << " digiRootData "  << getenv("DIGIROOTDATAROOT") << endl;
        ostrm << " idents       "  << getenv("IDENTSROOT")       << endl;
        ostrm << " calibUtil    "  << getenv("CALIBUTILROOT")    << endl;
      }
      ostrm << "****************************************************" << endl;
    }

    string &str_toupper(string &str) {
      std::transform(str.begin(), str.end(), str.begin(),
                     (int (*)(int))toupper);
      return str;
    }

    bool stringToBool(string str) {
      // convert all to uppper case
      str_toupper(str);

      if (str == "1") return true;
      if (str == "T") return true;
      if (str == "TRUE") return true;
      if (str == "Y") return true;
      if (str == "YES") return true;

      if (str == "0") return false;
      if (str == "F") return false;
      if (str == "FALSE") return false;
      if (str == "N") return false;
      if (str == "NO") return false;

      // bad format, throw exception
      ostringstream tmp;
      tmp << '"' << str << '"' << " not a boolean string.";

      throw std::runtime_error(tmp.str());
    }

    typedef std::vector<std::ostream *> streamvector;

    /** wrapper class for treating multiple streambuf objects as one
     *
     * used by multiplexor_ostream
     */
    class multiplexor_streambuf :
      public std::streambuf {
    public:
      multiplexor_streambuf() :
        std::streambuf() {
      }

      virtual int overflow(int c) {
        // write the incoming character into each stream
        streamvector::iterator _b = _streams.begin(), _e = _streams.end();


        for (; _b != _e; _b++)
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
    class multiplexor_ostream :
      public ostream
    {
    public:
      multiplexor_ostream() :
        std::ios(0),
        std::ostream(new multiplexor_streambuf()) {
      }

      virtual ~multiplexor_ostream() {
        delete rdbuf();
      }

      streamvector & getostreams() {
        return (dynamic_cast<multiplexor_streambuf *>(rdbuf())->_streams);
      }
    };

    /// hidden static instance is accessed by other classes
    /// through LogStream::get() method.
    static multiplexor_ostream _logStrm;

    ostream &LogStream::get() {
      return _logStrm;
    }

    void LogStream::addStream(ostream &ostrm) {
      _logStrm.getostreams().push_back(&ostrm);
    }

    TDirectory &root_safe_mkdir(TDirectory &parent,
                                const std::string &dirName) {
      TDirectory &cwd = *gDirectory;

      // only create new dir if it doesn't already exist
      if (!parent.cd(dirName.c_str())) {
        cwd.cd();
        return *(parent.mkdir(dirName.c_str()));
      }

      // else return existing directory
      cwd.cd();
      return *(parent.GetDirectory(dirName.c_str()));
    }

    vector<string> getLinesFromFile(const std::string &filename) {
      vector<string> retval;
      ifstream infile(filename.c_str());
      
      if (!infile.is_open())
        throw runtime_error(string("Unable to open " + filename));
    
      string line;
      while (infile.good()) {
      
        getline(infile, line);
        if (infile.fail()) break; // bad get

        retval.push_back(line);
      }

      return retval;
    }
  };
}; // namespace calibGenCAL
