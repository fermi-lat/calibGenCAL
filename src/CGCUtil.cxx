/** @file CGCUtil.cxx

    @author Zachary Fewtrell

    \brief generic utility functions used in calibGenCAL pkg
 */

// LOCAL INCLUDES
#include "CGCUtil.h"

// GLAST INCLUDES

// EXTLIB INCLUDES

// STD INCLUDES
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <sstream>
#include <stdexcept>

namespace CGCUtil {

  /** \brief splits a delmiited string into a vector of shorter token-strings
      stolen from http://www.linuxselfhelp.com/HOWTO/C++Programming-HOWTO-7.html
  */
  void tokenize_str(const string& str,
                    vector<string>& tokens,
                    const string& delimiters)
  {
    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos)
      {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
      }
  }

  /// finds position of last directory delimeter ('/' || '\\')
  /// in path, returns path.npos if no delim is found
  string::size_type path_find_last_delim(string &path) {
    // find last directory delimeter.
    string::size_type fwdslash_pos = path.find_last_of('/');
    string::size_type bckslash_pos = path.find_last_of('\\');

    // check for 'not found' cases
    if (fwdslash_pos == path.npos)
      return (bckslash_pos == path.npos) ? path.npos : bckslash_pos;
    if (bckslash_pos == path.npos)
      return (fwdslash_pos == path.npos) ? path.npos : fwdslash_pos;

    return max(fwdslash_pos,bckslash_pos);
  }

  string &path_remove_dir(string &path) {
    string::size_type slash_pos;
  
    // if there was no delimeter, return path unaltered
    if ((slash_pos = path_find_last_delim(path)) == path.npos) 
      return path;

    // else remove everything up to & including the delimeter
    path.erase(0,slash_pos+1);

    return path;
  }

  /// removes filename extension from end of path string.
  string &path_remove_ext(string &path) {
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

    // otherwise return the string as is
    return path;
  }

  void output_env_banner(ostream &ostrm) {
    // GENERATE TIME STRING
    char time_str[128];
    time_t tmt = time(NULL);
    struct tm *tm_now = localtime(&tmt);
    if (strftime(time_str, sizeof(time_str), 
                 "%c %z", tm_now) == 0) {
      strcpy(time_str,""); // error case
      cerr << __FILE__  << ":"     << __LINE__ << " " 
           << "WARNING. error generating time string!" << endl;
    }
    
    // GENERATE PACKAGE PATH STRINGS

    ostrm << "************** ENVIRONMENT SUMMARY *****************" << endl;
    ostrm << " RUNTIME : " << time_str << endl;
    ostrm << endl;

    // test that enviroment variables are present
    if (!getenv("CALIBGENCALROOT") ||
        !getenv("ROOTROOT")        ||
        !getenv("DIGIROOTDATAROOT")||
        !getenv("EVENTROOT")       ||
        !getenv("IDENTSROOT")      ||
        !getenv("CALIBUTILROOT")) {
      cerr << __FILE__  << ":"     << __LINE__ << " " 
           << "WARNING. error retrieveing packageROOT paths" << endl;
    } else {
      ostrm << " PACKAGE      "  << "PATH" << endl;
      ostrm << " calibGenCAL  "  << getenv("CALIBGENCALROOT")  << endl;  
      ostrm << " ROOT         "  << getenv("ROOTROOT")         << endl;
      ostrm << " digiRootData "  << getenv("DIGIROOTDATAROOT") << endl;
      ostrm << " Event        "  << getenv("EVENTROOT")        << endl;
      ostrm << " idents       "  << getenv("IDENTSROOT")       << endl;
      ostrm << " calibUtil    "  << getenv("CALIBUTILROOT")    << endl;
    }
    ostrm << "****************************************************" << endl;
  }

  string &str_toupper(string &str) {
  //transform(str.begin(),str.end(), str.begin(), toupper<char>);
  std::transform(str.begin(), str.end(), str.begin(), 
                 (int(*)(int)) toupper);
	return str;
  }

  bool stringToBool(const string &str) {
    // non const for modification
    string tmpStr = str;
  
    // convert all to uppper case
    str_toupper(tmpStr);

    if (tmpStr == "1")    return true;
    if (tmpStr == "T")    return true;
    if (tmpStr == "TRUE") return true;
    if (tmpStr == "Y")    return true;
    if (tmpStr == "YES")  return true;
  
    if (tmpStr == "0")     return false;
    if (tmpStr == "F")     return false;
    if (tmpStr == "FALSE") return false;
    if (tmpStr == "N")     return false;
    if (tmpStr == "NO")    return false;

    // bad format, throw exception
    ostringstream tmp;
    tmp << '"' << tmpStr << '"' << " not a boolean string.";

    throw runtime_error(tmp.str());
  }

};
