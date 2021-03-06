/** @file 

    @author Zachary Fewtrell

    \brief generic utility functions used in calibGenCAL pkg

    $Header: /nfs/slac/g/glast/ground/cvs/GlastRelease-scons/calibGenCAL/src/lib/Util/CGCUtil.cxx,v 1.13 2008/05/13 16:54:01 fewtrell Exp $
*/

// LOCAL INCLUDES
#include "CGCUtil.h"
#include "stl_util.h"

// GLAST INCLUDES
#include "facilities/commonUtilities.h"

// EXTLIB INCLUDES

// STD INCLUDES
#include <ctime>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <cstring>

using namespace std;

namespace calibGenCAL {

  string CVS_TAG() {return "$Name: GlastRelease-HEAD-1-1389 $";}
  string CGC_DEFAULT_CFGPATH() {return"$(CALIBGENCALROOT)/cfg/defaults.cfg";}

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
    if (facilities::commonUtilities::getPackagePath("calibGenCAL") == "" ||
	//        !getenv("ROOTROOT")        ||
        facilities::commonUtilities::getPackagePath("digiRootData") == "" ||
        facilities::commonUtilities::getPackagePath("idents") == "" ||
        facilities::commonUtilities::getPackagePath("calibUtil") == "")
      cerr << __FILE__  << ':'     << __LINE__ << ' '
           << "WARNING. error retrieveing package paths" << endl;
    else {
      ostrm << " PACKAGE      "  << "PATH" << endl;
      ostrm << " calibGenCAL  "  << facilities::commonUtilities::getPackagePath("calibGenCAL")  << endl;
      //ostrm << " ROOT         "  << getenv("ROOTROOT")         << endl;
      ostrm << " digiRootData "  << facilities::commonUtilities::getPackagePath("digiRootData") << endl;
      ostrm << " idents       "  << facilities::commonUtilities::getPackagePath("idents")       << endl;
      ostrm << " calibUtil    "  << facilities::commonUtilities::getPackagePath("calibUtil")    << endl;
    }
    ostrm << "****************************************************" << endl;
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
