// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Util/CfgMgr.cxx,v 1.2 2007/04/10 14:51:02 fewtrell Exp $

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "CfgMgr.h"

#include <sstream>
#include <string>

using namespace std;

namespace CfgMgr {
  void CmdLineParser::registerArg(ICmdArg &arg) {
    argList.push_back(&arg);
  }

  void CmdLineParser::registerSwitch(ICmdSwitch &sw) {
    char  shortName = sw.getShortName();
    const string &longName = sw.getLongName();


    if (shortName != 0) {
      // check for duplicates
      if (shortSwitchMap.find(shortName) != shortSwitchMap.end() ||
          shortVarMap.find(shortName) != shortVarMap.end())
        throw DuplicateNameError(""+shortName);

      shortSwitchMap[shortName] = &sw;
    }

    if (!longName.empty()) {
      // check for duplicates
      if (longSwitchMap.find(longName) != longSwitchMap.end() ||
          longVarMap.find(longName) != longVarMap.end())
        throw DuplicateNameError(longName);

      longSwitchMap[longName] = &sw;
    }

    switchList.push_back(&sw);
  }

  void CmdLineParser::registerVar(ICmdOptVar &var) {
    char  shortName = var.getShortName();
    const string &longName = var.getLongName();


    if (shortName != 0) {
      // check for duplicates
      if (shortSwitchMap.find(shortName) != shortSwitchMap.end() ||
          shortVarMap.find(shortName) != shortVarMap.end())
        throw DuplicateNameError(""+shortName);

      shortVarMap[shortName] = &var;
    }

    if (!longName.empty()) {
      // check for duplicates
      if (longSwitchMap.find(longName) != longSwitchMap.end() ||
          longVarMap.find(longName) != longVarMap.end())
        throw DuplicateNameError(longName);

      longVarMap[longName] = &var;
    }

    varList.push_back(&var);
  }

  void CmdLineParser::parseCmdLine(unsigned argc,
                                   const char **argv,
                                   bool allowAnonArgs,
                                   bool skipFirst,
                                   bool ignoreErrors
                                   ) {
    // skip empty lists
    if (argc == 0 || argv == 0)
      return;

    // optionally skip first param
    if (skipFirst) {
      argc--;
      argv++;
    }

    unsigned nPositionalArgs = 0;
    // loop through
    for (unsigned idx = 0; idx < argc; idx++) {
      string str(argv[idx]);

      // CASE 1: positional argument (no '-')
      if (str[0] != '-' ||
          str[0] == '-' && str.size() == 1 // case of single '-' as positional cmd argument
          ) {
        nPositionalArgs++;

        // CASE 1A: use one of pre-registered positional args
        if (nPositionalArgs <= argList.size())
          argList[nPositionalArgs-1]->setVal(str);

        // CASE 1B: append to indefinite list of anonymous positional args
        else if (allowAnonArgs)
          anonArgList.push_back(str);

        // CASE 1C: anonymous positional args not allowed
        else if (!ignoreErrors)
          throw InvalidCmdLine("Too many positional args @: " + str);

        // CASE 2: short optional var (single '-')
        //         length guaranteed >= 2 by CASE 1
      } else if (str[1] != '-') {
        unsigned charIdx = 1;

        // Loop over possibly chained option characters
        do {
          // get current letter
          char shortName = str[charIdx];

          // CASE 2A: 1st check if we need to scan forward for an argument.
          ShortVarMap::const_iterator shortVar(shortVarMap.find (shortName));

          if (shortVar != shortVarMap.end()) {
            // args for short option variables always follow in next token.

            // check that next token exists...
            idx++;
            if (idx >= argc && !ignoreErrors)
              throw InvalidCmdLine("No arg found for option: " + str);

            shortVar->second->setVal(argv[idx]);

            // done w/ current single-letter-option chain
            break;
          }

          // CASE 2B: must be a simple switch
          ShortSwitchMap::const_iterator shortSwitch(shortSwitchMap.find (shortName));

          if (shortSwitch != shortSwitchMap.end())
            shortSwitch->second->setVal();
          else if (!ignoreErrors)
            throw InvalidCmdLine("Undefined cmdline switch: " + str);
        } while (++charIdx < str.size()); // keep looping until we're out of characters

        // CASE 3: long option (double '--'
        //         length guaranteed >=2 by CASE 1
        //         1st 2 chars guaranteed '--' by CASE 1 & 2
      } else if (str.size() > 2) {
        // trim initial '--' off of all variable names
        str = str.substr(2);
        // CASE 3A: simple long-name-switch
        LongSwitchMap::const_iterator longSwitch(longSwitchMap.find (str));

        if (longSwitch != longSwitchMap.end()) {
          longSwitch->second->setVal();
          continue;
        }

        // CASE 3B: long-name-var takes arg

        // CASE 3B-1: equal sign delimits arg
        string::size_type eqpos = str.find('=');
        if (eqpos != str.npos) {
          // separate var name & value
          string                     name(str.substr(0, eqpos));
          string                     val(str.substr (eqpos+1, str.npos));

          LongVarMap::const_iterator longVar(longVarMap.find (name));

          if (longVar == longVarMap.end() && !ignoreErrors)
            throw InvalidCmdLine("Unknown cmdline variable: " + name);
          longVar->second->setVal(val);
        }

        // CASE 3B-2: take next token as arg
        else {
          // check that current var is registered
          LongVarMap::const_iterator longVar(longVarMap.find (str));

          if (longVar == longVarMap.end() && !ignoreErrors)
            throw InvalidCmdLine("Unknown cmdline variable: " + str);

          // check that next token exists...
          idx++;
          if (idx >= argc && !ignoreErrors)
            throw InvalidCmdLine("No arg found for option: " + str);

          longVar->second->setVal(argv[idx]);
        }
      } else if (!ignoreErrors)
        throw InvalidCmdLine("Undefined argument: " + str);
    } // argc looop

    //-- Final checks --//

    // check that all required arguments have been filled
    if (nPositionalArgs < argList.size() && !ignoreErrors)
      throw InvalidCmdLine("Not enough required arguments, need " + toStr(argList.size()));
  }

  void CmdLineParser::printStatus(std::ostream &strm) const {
    strm << "Cfg Status: " << endl;
    for (SwitchList::const_iterator sw(switchList.begin());
         sw != switchList.end();
         sw++) {
      char  shortName = (**sw).getShortName();
      const string &longName = (**sw).getLongName();

      // print out short name only if we have no long name
      if (longName.empty())
        strm << "-" << shortName;
      else
        strm << longName;
      strm << "\t";

      strm << (**sw).getVal() << endl;
    }

    for (VarList::const_iterator var(varList.begin());
         var != varList.end();
         var++) {
      char  shortName = (**var).getShortName();
      const string &longName = (**var).getLongName();

      if (longName.empty())
        strm << "-" << shortName;
      else
        strm << "--" << longName;
      strm << "\t";

      strm  << (**var).getStrVal() << "\t"
            << (**var).getHelp() << "\t"
            << endl;
    }

    for (ArgList::const_iterator arg(argList.begin());
         arg != argList.end();
         arg++) {
      const string &longName = (**arg).getLongName();

      strm << longName << "\t"
           << (**arg).getStrVal() << "\t"
           << (**arg).getHelp() << "\t"
           << endl;
    }

    for (unsigned idx = 0; idx < anonArgList.size(); idx++) {
      strm << "ananArg_" << idx << "\t";
      strm << anonArgList[idx] << "\t";
      strm << endl;
    }
  }

  void CmdLineParser::printUsage(std::ostream &strm) const {
    strm << "Usage: '" << appName << " ";

    //-- INITIAL USAGE STR --//
    // patterned after "grep [options] PATTERN [FILE...]"
    if (switchList.size() || varList.size())
      strm << "[options] ";

    for (unsigned i = 0; i < argList.size(); i++)
      strm << argList[i]->getLongName() << " ";

    strm << "'" << endl;

    //-- OPTIONS / ARG DESCRIPTIONS --//
    strm << "Where: " << endl;

    for (SwitchList::const_iterator sw(switchList.begin());
         sw != switchList.end();
         sw++) {
      char  shortName = (**sw).getShortName();
      const string &longName = (**sw).getLongName();

      strm << "\t";
      if (shortName != 0)
        strm << "-" << shortName;
      strm << "\t";
      strm << "--" << longName << "\t"
           << (**sw).getHelp() << "\t"
           << endl;
    }

    for (VarList::const_iterator var(varList.begin());
         var != varList.end();
         var++) {
      char  shortName = (**var).getShortName();
      const string &longName = (**var).getLongName();

      strm << "\t";
      if (shortName != 0)
        strm << "-" << shortName;
      strm << "\t";
      strm << "--" << longName << "\t"
           << (**var).getHelp() << "\t"
           << endl;
    }

    for (ArgList::const_iterator arg(argList.begin());
         arg != argList.end();
         arg++) {
      const string &longName = (**arg).getLongName();

      strm << "\t";
      strm << "\t";
      strm << longName << "\t"
           << (**arg).getHelp() << "\t"
           << endl;
    }

  }
};
