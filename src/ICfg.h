#ifndef ICfg_H
#define ICfg_H

// LOCAL INCLUDES

// GLAST INCLUDES

// EXTLIB INCLUDES

// STD INCLUDES
#include <string>

using namespace std;

/*! \brief Abstract template class for config classes used in calibGenCAL
 */

class ICfg {
public:
  /// read in values from config file
  virtual void readCfgFile(const string& path) = 0;
};


#endif // ICFG_H
