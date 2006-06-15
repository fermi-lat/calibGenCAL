// $Header$
/** @file
    @author Zachary Fewtrell
*/

#ifndef SimpleIniFile_h
#define SimpleIniFile_h

#include "CGCUtil.h"

#include <string>
#include <map>
#include <vector>
#include <sstream>

using namespace std;
using namespace CGCUtil;

/** 
    \brief Simple Ini file reading class.
    
Specification:
- whitespace all leading & trailing ' ' && '\t' are ignored from every line
- _all_ '\t' characters are converted to ' '
- comments ';' chars are recognized as comments.  all text following first ';' 
is ignored
- escape_chars there are none :)
- sections names are embraced in '[]' chars (ex. [section_name]) there should be
no other text on the line
- entries are of the format "varname = val...."

\note leading & trailing whitespace is removed from entry names _and_ values.
\note all text w/in '[]' section names is included.... 
therefore "[mysection]" != "[mysection ]"
\note entries with duplicate names will overwrite their predecessors 
\note sections with dupliate names will be concatenated.
\note some error messages will be printed to stderr
\note entries defined before any section defnition are stored in the empty "" 
section
*/

class SimpleIniFile
{
public:
  SimpleIniFile(const string &filename);

  /// template method retrieve a single value, convert to
  /// any desired type that is supported by STL iostream
  /// converters
  template <typename T>
  T getVal(const string &section,
           const string &key,
           const T& defaultVal) const {
    // retrieve raw string from cfg file
    const string *ptr = findKey(section, key);
    // quit if key doesn't exist.
    if (!ptr) 
      return defaultVal;
    if (*ptr == "") 
      return defaultVal;

    istringstream tmpStrm(*ptr);
    T val;
    tmpStrm >> val;
    return val;
    
  }
  
  /// template method retrieve a vector of values from delimited string.
  /// convert to any desired type that is supported by STL iostream
  /// converters
  template <typename T>
  void getVector(const string &section,
                 const string &key,
                 vector<T> &val,
                 const string &delims=",",
                 const vector<T> &defaultVal=vector<T>()) const {
    // retrieve raw string from cfg file
    const string *ptr = findKey(section, key);
    // quit if key doesn't exist.
    bool quitEarly = false;
    if (!ptr) quitEarly = true;
                
    if (*ptr == "") quitEarly = true;
    if (quitEarly) {
      val = defaultVal;
      return;
    }
  
    // break string up into string parts.
    vector<string> tokens;
    tokenize_str(*ptr, tokens, delims);
  
    // convert each string part into a value of desired type
    for (unsigned i =0; i < tokens.size(); i++) {
      istringstream tmpStrm(tokens[i]);
      T tmp;
      tmpStrm >> tmp;
      val.push_back(tmp);
    }
  }

private:
  void addKey(const string &section,
              const string &key,
              const string &val);
  
  /// \return null if key is not found
  const string *findKey(const string &section, 
                        const string &key) const;
  
  bool openIniFile (const string &filename);
  
  typedef map<string, string> Section; 
  typedef map<string, Section> SectionMap;
  
  SectionMap m_sectionMap;
};
#endif
