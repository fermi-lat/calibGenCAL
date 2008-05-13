/** @file 

    @author Zachary Fewtrell

    \brief implement stl_util.h methods

    $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Util/CGCUtil.cxx,v 1.12 2008/04/22 21:17:57 fewtrell Exp $
*/

// LOCAL INCLUDES
#include "stl_util.h"

// GLAST INCLUDES

// EXTLIB INCLUDES

// STD INCLUDES
#include <vector>
#include <fstream>
#include <string>
#include <stdexcept>
#include <sstream>

using namespace std;


namespace calibGenCAL {
  vector<string> getLinesFromFile(istream &infile) {
    vector<string> retval;
    
    string line;
    while (infile.good()) {
      
      getline(infile, line);
      if (infile.fail()) break; // bad get

      retval.push_back(line);
    }

    return retval;
  }

  std::string to_str(const int n) {
    ostringstream tmpstrm;

    tmpstrm << n;

    return tmpstrm.str();
  }

}
