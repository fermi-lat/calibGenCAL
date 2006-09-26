// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/SimpleIniFile.h,v 1.1 2006/06/15 20:58:00 fewtrell Exp $
/** @file
    @author Zachary Fewtrell
*/

#ifndef SimpleIniFile_h
#define SimpleIniFile_h

#include "CGCUtil.h"

#include <vector>
#include <string>
#include <sstream>
#include <map>

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
  SimpleIniFile(const std::string &filename);

  /// template method retrieve a single value, convert to
  /// any desired type that is supported by STL iostream
  /// converters
  template <typename T>
    T getVal(const std::string &section,
             const std::string &key,
             const T& defaultVal) const {
    // retrieve raw std::string from cfg file
    const std::string *ptr = findKey(section, key);
    // quit if key doesn't exist.
    if (!ptr) 
      return defaultVal;
    if (*ptr == "") 
      return defaultVal;

    std::istringstream tmpStrm(*ptr);
    T val;
    tmpStrm >> val;
    return val;
    
  }
  
  /// template method retrieve a vector of values from delimited string.
  /// convert to any desired type that is supported by STL iostream
  /// converters
  template <typename T>
    void getVector(const std::string &section,
                   const std::string &key,
                   std::vector<T> &val,
                   const std::string &delims=",",
                   const std::vector<T> &defaultVal=std::vector<T>()) const {
    // retrieve raw string from cfg file
    const std::string *ptr = findKey(section, key);
    // quit if key doesn't exist.
    bool quitEarly = false;
    if (!ptr) quitEarly = true;
                
    if (*ptr == "") quitEarly = true;
    if (quitEarly) {
      val = defaultVal;
      return;
    }
  
    // break string up into string parts.
    std::vector<std::string> tokens;
    CGCUtil::tokenize_str(*ptr, tokens, delims);
  
    // convert each string part into a value of desired type
    for (unsigned i =0; i < tokens.size(); i++) {
      std::istringstream tmpStrm(tokens[i]);
      T tmp;
      tmpStrm >> tmp;
      val.push_back(tmp);
    }
  }

 private:
  void addKey(const std::string &section,
              const std::string &key,
              const std::string &val);
  
  /// \return null if key is not found
  const std::string *findKey(const std::string &section, 
                             const std::string &key) const;
  
  bool openIniFile (const std::string &filename);
  
  typedef std::map<std::string, std::string> Section; 
  typedef std::map<std::string, Section> SectionMap;
  
  SectionMap m_sectionMap;
};
#endif
