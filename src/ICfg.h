#ifndef ICfg_H
#define ICfg_H 1

/*!
   \class ICfg Abastract template class for config classes used in calibGenCAL
 */


using namespace std;

class ICfg {
public:
  /// reset/initialize all config values
  virtual void clear() = 0;
  /// read in values from config file
  virtual int readCfgFile(const string& path) = 0;
  /// returns if config value set is valid/complete
  virtual bool isValid() = 0;
  /// lists summary of config vars to ostream
  virtual void summarize() = 0;
};


#endif // ICFG_H
