#ifndef string_util_h
#define string_util_h

//$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Util/string_util.h,v 1.1 2007/06/13 22:42:13 fewtrell Exp $

// STD INCLUDES
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <sstream>


/** @file
    @author  Zachary Fewtrell
    
    @brief collection of utilities for working w/ C++ STL string objects
*/

namespace calibGenCAL {
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

  /** convert string to boolean.
      \return boolean interperetation of value
      \throws exception if value is not properly formatted.
                         
      to be interpereted as boolean, value must be '1', '0', '[t]rue', '[f]alse', '[y]es', '[n]o'
      \note interperetation is case-insensitive.
  */
  bool stringToBool(const std::string &str);


  /** \brief convert string to uppercase
      \return ref to converted string
      \note operates in place on given string.
  */
  std::string & str_toupper(std::string &str);
                       
  
  /// generic type2string converter
  template <typename T>
  std::string toString(const T &val) {
    std::ostringstream tmp;
    tmp << val;
    return tmp.str();
  }
    
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

};

#endif
