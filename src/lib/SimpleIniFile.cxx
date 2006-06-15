// $Header$
/** @file
    @author Zachary Fewtrell
 */

// LOCAL INCLUDES
#include "SimpleIniFile.h"

// STD INCLUDES
#include <fstream>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cctype>


using namespace std;

////////////////////////////////////////////////////////////////////////////
//////////////////// UTIL FUNCTIONS ////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////


/** \brief trim trailing ' ' && '\t'
    \note transform is done in place on caller's string object
    \return ref to resultant string.
*/
static string &trim_right(string &str) {
  string::size_type pos = str.find_last_not_of(' ');
  
  // 'there is nothing that is not space' so return ""
  if (pos == str.npos) return str = "";
  
  // else return up to last char-that-is-not-a-space
  return str = str.substr(0, pos + 1);
} 

/** \brief trim leading ' ' && '\t' from input strings
    \note transform is done in place on caller's string object
    \return ref to resultant string.
*/
static string &trim_left(string &str) {
  string::size_type pos = str.find_first_not_of(' ');

  // 'there is nothing that is not space' so return ""
  if (pos == str.npos) return str = "";

  // else return everything from 1st char-that-is-not-a-space on.
  return str = str.substr(pos);
} 

/** \brief trim leading AND trailing ' ' && '\t' from input string
    \note transform is done in place on caller's string object
    \return ref to resultant string.
*/
static string &trim_whitespace(string &str) {
  // replace all '\t' with ' ' for trim_right & trim_left functions
  string::size_type pos;
  while ((pos = str.find_first_of("\t\n\r")) != string::npos)
    str.replace(pos,1," ");

  return str = trim_right(trim_left(str));
}

/** \brief trim first ';' comment char and all trailing text.
    \note transform is done in place on caller's string object
    \return ref to resultant string.
*/
static string &trim_comment(string &str) {
  string::size_type pos = str.find_first_of(';');
  
  // no ';' found, no change
  if (pos == str.npos) return str;  
  
  // else return up to last char before ';'
  return str = str.substr(0, pos);
} 

// static string &str_toupper(string &str) {
//   //transform(str.begin(),str.end(), str.begin(), toupper<char>);
//   std::transform(str.begin(), str.end(), str.begin(), 
//                  (int(*)(int)) toupper);
//   return str;
// }

SimpleIniFile::SimpleIniFile(const string &filename) 
{
  openIniFile(filename);
}

bool SimpleIniFile::openIniFile(const string &filename)
{
  unsigned n_line = 0;
  
  ifstream infile(filename.c_str());
  if (!infile.is_open())
    return false;

  string line;
  string cur_section("");

  while (infile.good()) {
    getline(infile, line);
    if (infile.fail()) break; // bad get

    n_line++;
    
    // clean up line
    trim_whitespace(line);
    trim_comment(line);
    
    // skip empty lines
    if (line == "") continue;
    
    // do we have a section?
    if (line[0] == '[' && line[line.size()-1] == ']') {
      // trim brackets.
      line = line.substr(1); // trim leading '['
      line = line.substr(0,line.size()-1); // trim trailing ']'

      cur_section = line;
      continue;
    }

    // do we have an entry?
    string::size_type equal_pos;
    if ((equal_pos = line.find_first_of('=')) != line.npos) {
      string key = line.substr(0,equal_pos);
      trim_whitespace(key);
      
      string val = line.substr(equal_pos+1);
      trim_whitespace(val);

      addKey(cur_section, key, val);
    } else {
      cerr << __FILE__ << ' ' << filename << ':' << n_line 
           << " is invalid (neither comment, nor section, nor entry!)" << endl;
      cerr << " '" << line << "'" << endl;
    }
  }
  return true;
}

const string *SimpleIniFile::findKey(const string &section, 
                                     const string &key) const
{
  // if section does not exist, return false
  SectionMap::const_iterator smIter = m_sectionMap.find(section);
  if (smIter == m_sectionMap.end()) 
    return NULL;

  // we found section, move forward.
  const Section &mySec = (*smIter).second;

  // if key does not exist, return false.
  Section::const_iterator iter = mySec.find(key);
  if (iter == mySec.end())
    return NULL;

  // successful find, set val & exit
  return &iter->second;
}

void SimpleIniFile::addKey(const string &section, const string &key, const string &val) {
           
  // first look for section
  SectionMap::iterator smIter = m_sectionMap.find(section);

  // create new seciton if it doesn't exist
  if (smIter == m_sectionMap.end()) {
    // create new section
    Section mySec;
    // insert new key/val pair
    mySec[key] = val;
                
    // add section to list;
    m_sectionMap[section] = mySec;

    // done
    return;
  }
  
  // add key/val to section if it already exists.
  Section &mySec = (*smIter).second;
  mySec[key] = val;

  return;
}



