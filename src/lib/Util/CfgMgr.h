// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Util/CfgMgr.h,v 1.3 2007/04/16 20:35:36 fewtrell Exp $

/** @file
    @author Zachary Fewtrell
*/

#ifndef CfgMgr_h
#define CfgMgr_h

#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <stdexcept>
#include <iostream>

/**
   \brief Simple command line option parser.  Inspired by python 'optparse' module

   \note this package attempts to follow the definitions stated in python optparse documentation
   here: http://docs.python.org/lib/optparse-terminology.html

   Example Usage:


*/

namespace CfgMgr {
  template <typename _T>
    std::string toStr(const _T &val) {
    std::ostringstream tmp;
    tmp << val;
    return tmp.str();
  }

  /// \brief Abstract Interface Class for Optional position independent commandline switch which
  /// takes no argument
  /// Switches are boolean variables which default to false.
  /// \note truly 'virtual' class has no data members, suitable for virtual/multiple inheritance
  class ICmdSwitch {
  public:
    virtual const std::string & getLongName() const = 0;
    virtual char                getShortName() const = 0;
    virtual const std::string & getHelp() const     = 0;
    virtual void                setVal() = 0;
    virtual bool                getVal() const = 0;
  };

  /// \brief Abstract Interface Class for Optional position independent commandline which takes an argument
  /// \note truly 'virtual' class has no data members, suitable for virtual/multiple inheritance
  class ICmdOptVar {
  public:
    virtual const std::string & getLongName() const  = 0;
    virtual char                getShortName() const = 0;
    virtual const std::string & getHelp() const      = 0;
    virtual void                setVal(const std::string &newVal)  = 0;
    virtual std::string         getStrVal() const = 0;
  };

  /// \brief Abstract Interface Class for Mandatory: position dependent commandline argument
  /// \note truly 'virtual' class has no data members, suitable for virtual/multiple inheritance
  class ICmdArg {
  public:
    virtual const std::string & getLongName() const = 0;
    virtual const std::string & getHelp() const     = 0;
    virtual void                setVal(const std::string &newVal) = 0;
    virtual std::string         getStrVal() const = 0;
  };

  /// \brief thrown when variable parameters are inconsistant / illegal
  class InvalidVarDef :
    public std::logic_error {
    public:
    InvalidVarDef(const std::string &desc) :
      std::logic_error("Invalid Cfg Variable Definition: " + desc)
      {
      }
  };

  /// \brief thrown when duplicate variable names are registered to same parser
  class DuplicateNameError :
    public std::logic_error {
    public:
    DuplicateNameError(const std::string &desc) :
      std::logic_error("Duplicate variable name registered: " + desc)
      {
      }
  };

  /// \brief thrown when user entered argument list is invalid format
  class InvalidCmdLine :
    public std::runtime_error {
    public:
    InvalidCmdLine(const std::string &desc) :
      std::runtime_error("Invalid commandline: " + desc) {
    }
  };

  /// \brief basic implementation of ICmdSwitch
  class CmdSwitch :
    public virtual ICmdSwitch {
    public:
    /// \param  Optionally may be "" to disable longName
    /// \param shortName may be (char)0 in order to disable shortName
    /// \pre at least one of  longName or shortName needs to be defined.
    /// \pre longName should consist of word delimited by the '-' character, it 
    ///      should avoid commandline special characters
    /// \thows InvalidVardef
    CmdSwitch(const std::string &longName,
              char shortName,
              const std::string &help) :
      longName(longName),
      shortName(shortName),
      help(help),
      val(false) {
      //-- Integrity checks --//
      if (shortName == 0 &&
          longName.empty())
        throw InvalidVarDef("Require at least 1 of a) shortName or b) longName");

      if (longName.size() == 1)
        throw InvalidVarDef("longName must be > 1 char");
    }

    virtual ~CmdSwitch() {
    }

    virtual const std::string &getLongName() const {
      return longName;
    }

    virtual char getShortName() const {
      return shortName;
    }

    virtual const std::string &getHelp() const {
      return help;
    }

    virtual void setVal() {
      val = true;
    }

    virtual bool getVal() const {
      return val;
    }

    private:
    std::string longName;
    char shortName;
    std::string help;
    bool val;
  };

  /// \brief basic template implementation of ICmdOptVar
  ///
  /// - templatized for use w/ multiple destination
  /// data types
  template <typename _T>
    class CmdOptVar :
    public virtual ICmdOptVar {
    public:
    /// \pre longName should contain only letters & '-' character.
    /// Optionally may be "" to disable longName
    /// \pre shortName may be (char)0 in order to disable shortName
    /// \throws InvalidVarDef
    CmdOptVar(const std::string &longName,
              char shortName,
              const std::string &help,
              const _T &defVal) :
      longName(longName),
      shortName(shortName),
      help(help),
      val(defVal) {
      //-- Integrity checks --//
      if (shortName == 0 &&
          longName.empty())
        throw InvalidVarDef("CmdOptVar must at least 1 of a) shortName or b) longName");

      if (longName.size() == 1)
        throw InvalidVarDef("longName must be > 1 char");
    }

    virtual ~CmdOptVar() {
    };

    virtual const std::string &getLongName() const {
      return longName;
    }

    virtual char getShortName() const {
      return shortName;
    }

    virtual const std::string &getHelp() const {
      return help;
    }

    virtual void setVal(const std::string &newVal) {
      std::istringstream strm(newVal);

      strm >> val;
    }

    virtual std::string getStrVal() const {
      return toStr(val);
    }

    const _T &getVal() const {
      return val;
    }

    private:
    std::string longName;
    char shortName;
    std::string help;
    _T val;
  };

  /// \brief basic template implementation of ICmdArg
  ///
  /// - templateized for use w/ mulitple destination data types
  template<typename _T>
    class CmdArg :
    public virtual ICmdArg {
    public:
    CmdArg(const std::string &name,
           const std::string &help,
           const _T &defVal
           ) :
      name(name),
      help(help),
      val(defVal)
      {
      }

    virtual ~CmdArg() {
    };

    virtual const std::string &getLongName() const {
      return name;
    }

    virtual const std::string &getHelp() const {
      return help;
    }

    virtual void setVal(const std::string &newVal) {
      std::istringstream strm(newVal);

      strm >> val;
    }

    virtual std::string getStrVal() const {
      return toStr(val);
    }

    const _T &getVal() const {
      return val;
    }

    private:
    std::string name;
    std::string help;
    _T val;
  };

  /// basic implementation of ICmdLineParser
  class CmdLineParser {
  public:
    /// \param appName optional for usage print-outs, etc
    CmdLineParser(const std::string &appName) :
      appName(appName) {
    }

    void registerArg(ICmdArg &arg);

    /// \throws DulicateNameError
    void registerSwitch(ICmdSwitch &sw);

    /// \throws DulicateNameError
    void registerVar(ICmdOptVar &var);

    /// \brief parse commandline variables & assign values to all registered variableshap
    /// \param allowAnonArgs if # of positional args > number registered positional args then
    ///                      all subsequent args (non-options, no '-' prefix) will be appended
    ///                      to anonArgList
    /// \param skipFirst     skip first argument which is often just the program name & not
    ///                      a variable.
    /// \note this method attempts to follow the definitions stated in python optparse documentation
    ///       here: http://docs.python.org/lib/optparse-terminology.html
    /// \throw InvalidCmdLine
    void parseCmdLine(unsigned argc,
                      const char **argv,
                      bool allowAnonArgs = false,
                      bool skipFirst = true,
                      bool ignoreErrors = false
                      );

    void printStatus(std::ostream &strm = std::cout) const;

    void printHelp(std::ostream &strm = std::cout) const;

    void printUsage(std::ostream &strm = std::cout) const;

    const std::vector<std::string> &getAnonArgs() {
      return anonArgList;
    }

  private:
    typedef std::map<char, ICmdSwitch *>        ShortSwitchMap;
    typedef std::map<std::string, ICmdSwitch *> LongSwitchMap;
    typedef std::vector<ICmdSwitch *>           SwitchList;

    typedef std::map<char, ICmdOptVar *>        ShortVarMap;
    typedef std::map<std::string, ICmdOptVar *> LongVarMap;
    typedef std::vector<ICmdOptVar *>           VarList;

    typedef std::vector<ICmdArg *>              ArgList;

    ShortSwitchMap shortSwitchMap;
    LongSwitchMap  longSwitchMap;
    SwitchList     switchList;

    ShortVarMap    shortVarMap;
    LongVarMap     longVarMap;
    VarList     varList;

    ArgList     argList;

    std::vector<std::string> anonArgList;

    std::string appName;
  };
};
#endif
