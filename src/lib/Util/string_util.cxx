/** @file
    @author Zachary Fewtrell
    @brief implementation of string_util.h
*/

// LOCAL INCLUDES
#include "string_util.h"

// STD INCLUDES
#include <algorithm>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace calibGenCAL {
  vector<string> tokenize_str(const string & str,
                              const string & delims)
  {
    // Skip delims at beginning.
    string::size_type lastPos = str.find_first_not_of(delims, 0);
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of(delims, lastPos);

    vector<string> tokens;

    while (string::npos != pos || string::npos != lastPos)
      {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delims.  Note the "not_of"
        lastPos = str.find_first_not_of(delims, pos);
        // Find next "non-delimiter"
        pos     = str.find_first_of(delims, lastPos);
      }

    return tokens;
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


  string path_remove_dir(string path) {
    string::size_type slash_pos;

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

  string &str_toupper(string &str) {
    transform(str.begin(), str.end(), str.begin(),
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

    throw runtime_error(tmp.str());
  }

  /// left pad inStr out to padLen with padchar
  /// \note new string object is created and returned on stack
  std::string lpad(const std::string &inStr, const size_t padLen, const char padChar) {
    const size_t inLen = inStr.size();
    /// only pad if inStr is shorted than padLen
    if (padLen <= inLen)
      return inStr;

    const size_t padSize = padLen - inLen;

    string retVal(padLen,padChar);

    retVal.replace(padSize, inStr.size(), inStr);
      
    return retVal;
  }


};
