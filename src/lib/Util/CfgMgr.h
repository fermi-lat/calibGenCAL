// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Util/CfgMgr.h,v 1.8 2007/09/07 20:52:10 fewtrell Exp $

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

   Currently supports the following types of commandline parameters :
   
   CmdSwitch - optional, position independnent, takes no argument
          - has short name (-s), long name (--switch)
          - defaults to false

   CmdOptVar  - optional, position independent, takes an argument
           - has short name (-s val), long name (--switch=val)

   CmdArg     - required, position dependent
           - no variable name on commandline, value only
          

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

  /// \brief Abstract Interface Class for Optional position independent commandline variable which takes an argument
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
    virtual bool isOptional() const = 0;
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
    /// \pre longName should consist of word delimited by the '-' character, it should
    ///      avoid commandline special characters
    /// \thows InvalidVardef
    CmdSwitch(const std::string &longName,
              const char shortName,
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
    const std::string longName;
    const char shortName;
    const std::string help;
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
              const char shortName,
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
    const std::string longName;
    const char shortName;
    const std::string help;
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
           const _T &defVal,
           const bool isOpt=false
           ) :
      name(name),
      help(help),
      isOpt(isOpt),
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

    bool isOptional() const {
      return isOpt;
    }

  private:
    const std::string name;
    const std::string help;
    const bool isOpt;
    _T val;
  };

  /// basic implementation of ICmdLineParser
  class CmdLineParser {
  public:
    /// \param appName optional for usage print-outs, etc
    CmdLineParser(const std::string &appName) :
      appName(appName),
      nOptionalArgs(false)
    {
    }

    /// register positional argument
    /// \note non-optional arguments cannot follow optional args
    /// \thows std::invalid_arg if non-optional arg follows optional arg
    void registerArg(ICmdArg &arg);

    /// \throws DulicateNameError
    void registerSwitch(ICmdSwitch &sw);

    /// \throws DulicateNameError
    void registerVar(ICmdOptVar &var);

    /// \brief parse commandline variables & assign values to all registered variableshap
    /// \param allowAnonArgs if # of positional args > number registered positional args then
    ///                      all subsequent args (non-options, no '-' prefix) will be appended
    ///                      to anonArgList.
    /// \note you cannot combine optional arguments and anonymous args int he same specification
    /// \param skipFirst     skip first argument which is often just the program name & not
    ///                      a variable.
    /// \note this method attempts to follow the definitions stated in python optparse documentation
    ///       here: http://docs.python.org/lib/optparse-terminology.html
    /// \throw InvalidCmdLine if actual cmdline does not match specifications
    /// \throw std::invalid_argument if specifications are inconsistent (ex. 'both optional & anonymous args requested)

    void parseCmdLine(const unsigned argc,
                      const char **argv,
                      const bool allowAnonArgs = false,
                      const bool skipFirst = true,
                      const bool ignoreErrors = false
                      );

    void printStatus(std::ostream &strm = std::cout) const;

    void printHelp(std::ostream &strm = std::cout) const;

    void printUsage(std::ostream &strm = std::cout) const;

    const std::vector<std::string> &getAnonArgs() const {
      if (nOptionalArgs > 0)
        throw std::invalid_argument("BUG! Anonymous args cannot be combined w/ optional args!");
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
    VarList varList;

    ArgList argList;

    /// list of anonymous
    std::vector<std::string> anonArgList;

    /// application name
    std::string appName;

    /// non-optional positional args cannot follow optional args
    /// \note cannot be combined w/ anonymous args
    unsigned nOptionalArgs;

  };
};
#endif
