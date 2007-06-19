/** @file 

    @author Zachary Fewtrell

    \brief generic utility functions used in calibGenCAL pkg

    $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Util/CGCUtil.cxx,v 1.5 2007/06/13 22:42:13 fewtrell Exp $
*/

// LOCAL INCLUDES
#include "CGCUtil.h"
#include "stl_util.h"

// GLAST INCLUDES

// EXTLIB INCLUDES

// STD INCLUDES
#include <ctime>
#include <iostream>
#include <fstream>
#include <stdexcept>

using namespace std;

namespace calibGenCAL {

  const string CVS_TAG("$Name:  $");
  const string CGC_DEFAULT_CFGPATH("$(CALIBGENCALROOT)/cfg/defaults.cfg");

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

  

  vector<string> getLinesFromFile(const string &filename) {
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

  /// hidden static instance is accessed by other classes
  /// through LogStrm::get() method.
  static multiplexor_ostream _logStrm;


  std::ostream &LogStrm::get() {
    return _logStrm;
  }

  void LogStrm::addStream(ostream &ostrm) {
    _logStrm.getostreams().push_back(&ostrm);
  }

}; // namespace calibGenCAL
