#include "CfgMgr.h"
//#include "testUtil.h"

#include <stdexcept>
#include <iostream>
#include <string>

using namespace std;
using namespace CfgMgr;


#define TEST_ASSERT(str, test) if (!(test)) retVal = false, \
                                    cout << "TESTFAIL! " __FILE__ \
                                    << ":" << __LINE__ << " "<< str << endl;

int NEW_TEST_INT() {
  static int currentVal = 0;
  return currentVal++;
}

namespace {
  template <typename _T>
  string toStr(const _T &val) {
    ostringstream tmp;

    tmp << val;
    return tmp.str();
  }

  bool test_CmdSwitch() {
    bool retVal = true;


    try {
      CmdSwitch s("",
                  0,
                  "");

      TEST_ASSERT("longName && shortName cannot _both_ be disabled", 0);
    } catch (InvalidVarDef &e) {
      // do nothing, this is what we want.
    }

    try {
      CmdSwitch s("l",
                  0,
                  "");

      TEST_ASSERT("longName.size() must be > 1", 0);
    } catch (InvalidVarDef &e) {
      // do nothing, this is what we want.
    }

    CmdSwitch s("test",
                't',
                "help");

    TEST_ASSERT("get() != ctor", s.getLongName()  == "test");
    TEST_ASSERT("get() != ctor", s.getShortName() == 't');
    TEST_ASSERT("get() != ctor", s.getHelp()      == "help");
    TEST_ASSERT("default val is false", !s.getVal());

    s.setVal();
    TEST_ASSERT("setVal() makes true",
                s.getVal());

    return retVal;
  }

  bool test_CmdArg() {
    bool retVal = true;

    {
      CmdArg<int> s("test", "help", 5);
      TEST_ASSERT("get() != ctor",  s.getLongName() == "test");
      TEST_ASSERT("get() != ctor",   s.getHelp()     == "help");
      TEST_ASSERT("get() != ctor",   s.getVal()      == 5);
      TEST_ASSERT("get() != ctor",   s.getStrVal()   == "5");
      TEST_ASSERT("get() != ctor",   s.isOptional() == false);

      s.setVal("6");
      TEST_ASSERT("setVal() test",
                  s.getVal() == 6);
    }

    {
      CmdArg<int> s("test", "help", 5,true);
      TEST_ASSERT("get() != ctor",   s.isOptional() == true);
      
    }

    return retVal;
  }

  bool test_CmdOptVar() {
    bool retVal = true;


    try {
      CmdOptVar<int> s("", 0, "", 0);
      TEST_ASSERT("longName && shortName cannot _both_ be disabled", 0);
    } catch (InvalidVarDef &e) {
      // do nothing, this is what we want.
    }

    try {
      CmdOptVar<int> s("l", 0, "", 0);
      TEST_ASSERT("longName.size() must be > 1", 0);
    } catch (InvalidVarDef &e) {
      // do nothing, this is what we want.
    }

    CmdOptVar<int> s("test", 't', "help", 5);
    TEST_ASSERT("get() != ctor", s.getLongName()  == "test"   );
    TEST_ASSERT("get() != ctor",  s.getShortName() == 't'      );
    TEST_ASSERT("get() != ctor",  s.getHelp()      == "help"   );
    TEST_ASSERT("get() != ctor",  s.getVal()       == 5        );
    TEST_ASSERT("get() != ctor",  s.getStrVal()    == "5");

    s.setVal("6");
    TEST_ASSERT("setVal() test", s.getVal() == 6);

    return retVal;
  }

  /// setup script shared by individual CmdLineParser tests
#define PARSER_SETUP \
  CmdLineParser c("UNIT_TEST");

#define ARG_SETUP                                                             \
  int argDefVal = NEW_TEST_INT();                                               \
  CmdArg<int> arg("arg",                                                      \
                  "test positional commandline argument",                     \
                  argDefVal);                                                 \
  c.registerArg(arg);

#define SWITCH_SETUP                                                          \
  CmdSwitch sw("switch",                                                      \
               's',                                                           \
               "test commandline switch");                                    \
  c.registerSwitch(sw);

#define VAR_SETUP                                                             \
  int varDefVal = NEW_TEST_INT();                                               \
  CmdOptVar<int> var("var",                                                   \
                     'v',                                                     \
                     "test optional commandline variable",                    \
                     varDefVal);                                              \
  c.registerVar(var);

#define FULL_SETUP PARSER_SETUP; ARG_SETUP; VAR_SETUP; SWITCH_SETUP;

  string toLongSwitch(const string &longName) {
    return string("--") + longName;
  }

  string toShortSwitch(char shortName) {
    return string("-") + shortName;
  }

  bool test_CmdLineParser() {
    bool retVal = true;


    // print help test
    {
      FULL_SETUP;
      c.printHelp();
      c.printStatus();
    }

    // empty commandline test
    {
      PARSER_SETUP;

      unsigned     argc = 0;
      const char **argv = 0;

      try {
        c.parseCmdLine(argc, argv);
      } catch (exception &e) {
        TEST_ASSERT("Testing empty commandline", 0);
      }

      try {
        c.parseCmdLine(argc, argv, false, false);
      } catch (exception &e) {
        TEST_ASSERT("Testing empty commandline, skipFirst = false", 0);
      }
    }

    // test simple arg
    {
      PARSER_SETUP;
      ARG_SETUP;
      int         testVal = NEW_TEST_INT();

      unsigned    argc    = 1;
      const char *argv[]  = {
        toStr(testVal).c_str()
      };

      c.parseCmdLine(argc, argv, false, false);
      TEST_ASSERT("test simple arg", arg.getVal() == testVal);
    }

    // test skip first = true
    {
      PARSER_SETUP;
      ARG_SETUP;
      int         testVal = NEW_TEST_INT();

      unsigned    argc    = 2;
      const char *argv[]  = {
        "progname", toStr(testVal).c_str()
      };

      c.parseCmdLine(argc, argv, false);
      TEST_ASSERT("test simple arg", arg.getVal() == testVal);
    }

    // test skip first = false
    {
      PARSER_SETUP;
      ARG_SETUP;
      int         testVal = NEW_TEST_INT();

      unsigned    argc    = 1;
      const char *argv[]  = {
        toStr(testVal).c_str()
      };

      c.parseCmdLine(argc, argv, false, false);
      TEST_ASSERT("test simple arg", arg.getVal() == testVal);
    }

    // test anonVar - enabled
    {
      PARSER_SETUP;

      unsigned    argc   = 1;
      const char *argv[] = {
        "anon"
      };

      c.parseCmdLine(argc, argv, true, false);
      TEST_ASSERT("test anonVr", c.getAnonArgs().size() == 1);
      TEST_ASSERT("test anonVr", c.getAnonArgs()[0] == "anon");
    }

    // test anonVar - disabled
    {
      PARSER_SETUP;

      unsigned    argc   = 1;
      const char *argv[] = {
        "anon"
      };

      // should fail
      try {
        c.parseCmdLine(argc, argv, false, false);
        TEST_ASSERT("Anon arg disabled should fail on extra arg", 0);
      } catch (InvalidCmdLine &e) {
        // do nothing, this is what we want
      }
    }

    // test long switch
    {
      PARSER_SETUP;
      SWITCH_SETUP;

      unsigned    argc   = 1;
      const char *argv[] = {
        toLongSwitch(sw.getLongName()).c_str()
      };

      c.parseCmdLine(argc, argv, false, false);
      TEST_ASSERT("test long switch", sw.getVal() == true);
    }

    // test short switch
    {
      PARSER_SETUP;
      SWITCH_SETUP;

      unsigned    argc   = 1;
      const char *argv[] = {
        toShortSwitch(sw.getShortName()).c_str()
      };

      c.parseCmdLine(argc, argv, false, false);
      TEST_ASSERT("test short switch", sw.getVal() == true);
    }

    // test missing switch, arg, var
    {
      PARSER_SETUP;

      unsigned    argc   = 1;
      const char *argv[] = {
        "-i"
      };

      try {
        c.parseCmdLine(argc, argv, false, false);
        TEST_ASSERT("unregistered switch test", 0);
      } catch (InvalidCmdLine &e) {
        // do nothing, we're happy
      }

      try {
        argv[0] = "--invalid";
        c.parseCmdLine(argc, argv, false, false);
        TEST_ASSERT("unregistered switch test", 0);
      } catch (InvalidCmdLine &e) {
        // do nothing, we're happy
      }

      try {
        argv[0] = "invalid";
        c.parseCmdLine(argc, argv, false, false);
        TEST_ASSERT("unregistered arg test", 0);
      } catch (InvalidCmdLine &e) {
        // do nothing, we're happy
      }

      try {
        argv[0] = "--invalid=unregistered";
        c.parseCmdLine(argc, argv, false, false);
        TEST_ASSERT("unregistered var test", 0);
      } catch (InvalidCmdLine &e) {
        // do nothing, we're happy
      }
    }

    // test string of short switches
    {
      PARSER_SETUP;
      CmdSwitch sw1("",
                    '1',
                    "one");
      CmdSwitch sw2("",
                    '2',
                    "two");
      CmdSwitch sw3("",
                    '3',
                    "three");

      c.registerSwitch(sw1);
      c.registerSwitch(sw2);
      c.registerSwitch(sw3);

      unsigned    argc   = 1;
      const char *argv[] = {
        "-123"
      };

      c.parseCmdLine(argc, argv, false, false);
      TEST_ASSERT("COMBINED SHORT SWITH TEST", sw1.getVal() == true);
      TEST_ASSERT("COMBINED SHORT SWITH TEST", sw2.getVal() == true);
      TEST_ASSERT("COMBINED SHORT SWITH TEST", sw3.getVal() == true);
    }

    // test switch combo w/ short var
    {
      PARSER_SETUP;
      SWITCH_SETUP;
      VAR_SETUP;

      int      testVal = NEW_TEST_INT();

      unsigned argc    = 2 ;
      string arg1(toShortSwitch (sw.getShortName()));

      arg1 += var.getShortName();
      string arg2(toStr (testVal));

      const char *argv[] = {
        arg1.c_str(), arg2.c_str()
      };

      c.parseCmdLine(argc, argv, false, false);
      TEST_ASSERT("SHORT SWITCH/VAR COMBO TEST", sw.getVal() == true);
      TEST_ASSERT("SHORT SWITCH/VAR COMBO TEST", var.getVal() == testVal);
    }

    // test long var (w/ '=')
    {
      PARSER_SETUP;
      VAR_SETUP;
      int         testVal = NEW_TEST_INT();

      unsigned    argc    = 1;
      string arg1(toLongSwitch (var.getLongName()));

      arg1 += "=";
      arg1 += toStr(testVal);
      const char *argv[]  = {
        arg1.c_str()
      };

      c.parseCmdLine(argc, argv, false, false);
      TEST_ASSERT("LONG var=val TEST", var.getVal() == testVal);
    }

    // test long var (2 word w/out '=')
    {
      PARSER_SETUP;
      VAR_SETUP;
      int         testVal = NEW_TEST_INT();

      unsigned    argc    = 2;
      string arg1(toLongSwitch (var.getLongName()));
      string arg2(toStr (testVal));

      const char *argv[]  = {
        arg1.c_str(), arg2.c_str()
      };

      c.parseCmdLine(argc, argv, false, false);
      TEST_ASSERT("LONG VAR (2 word) TEST", var.getVal() == testVal);
    }

    // test kitchen sink
    {
      PARSER_SETUP;
      CmdSwitch sw1("sw1",
                    '1',
                    "switch one");
      CmdSwitch sw2("sw2",
                    '2',
                    "switch two");
      CmdSwitch sw3("sw3",
                    '3',
                    "switch three");

      c.registerSwitch(sw1);
      c.registerSwitch(sw2);
      c.registerSwitch(sw3);

      CmdOptVar<string> vx("varx", 'x', "variable x", "n/a");
      CmdOptVar<string> vy("vary", 'y', "variable y", "n/a");
      CmdOptVar<string> vz("varz", 'z', "variable z", "n/a");
      c.registerVar(vx);
      c.registerVar(vy);
      c.registerVar(vz);

      CmdArg<string> arg1("arg1", "argument 1", "n/a");
      CmdArg<string> arg2("arg2", "argument 2", "n/a");
      c.registerArg(arg1);
      c.registerArg(arg2);

      const char *argv[] = {
        "progname",
        "--sw1",
        "-23x",
        "vx",
        "arg1",
        "--vary",
        "vy",
        "--varz=vz",
        "arg2",
        "anon"
      };

      unsigned    argc = sizeof(argv)/sizeof(*argv);

      c.parseCmdLine(argc, argv, true);
      TEST_ASSERT("KITCHEN SINK: ", sw1.getVal() == true);
      TEST_ASSERT("KITCHEN SINK: ", sw2.getVal() == true);
      TEST_ASSERT("KITCHEN SINK: ", sw3.getVal() == true);
      TEST_ASSERT("KITCHEN SINK: ", vx.getVal() == "vx");
      TEST_ASSERT("KITCHEN SINK: ", vy.getVal() == "vy");
      TEST_ASSERT("KITCHEN SINK: ", vz.getVal() == "vz");
      TEST_ASSERT("KITCHEN SINK: ", arg1.getVal() == "arg1");
      TEST_ASSERT("KITCHEN SINK: ", arg2.getVal() == "arg2");
      TEST_ASSERT("KITCHEN SINK: ", c.getAnonArgs().size() == 1);
      TEST_ASSERT("KITCHEN SINK: ", c.getAnonArgs()[0] == "anon");

      c.printStatus();
    }

    // test optional args
    {
      PARSER_SETUP;
      CmdArg<string> argOpt("argOpt", "argument 1", "n/a", true);
      CmdArg<string> argReq("argReq", "argument 2", "n/a");

      c.registerArg(argReq);
      c.registerArg(argOpt);
      
      {
        // w/out optional arg
        unsigned    argc   = 1;
        const char *argv[] = {
          "req"
        };


        // w/out optional arg (should work w/out exception)
        c.parseCmdLine(argc,argv,false,false);
      }      

      // w/ optional arg
      unsigned    argc   = 2;
      const char *argv[] = {
        "req",
        "opt"
      };

      c.parseCmdLine(argc,argv,false,false);
      TEST_ASSERT("OPTIONAL ARGS: ", argOpt.getVal() == "opt");
        
    }
      
    // test optional args, order w/ required args
    {
      PARSER_SETUP;
      CmdArg<string> argOpt("argOpt", "argument 1", "n/a", true);
      CmdArg<string> argReq("argReq", "argument 2", "n/a");

      c.registerArg(argOpt);
      
      try {
        c.registerArg(argReq);
        TEST_ASSERT("Optional/nonOptional order enforcement broken",0);
      } catch (invalid_argument &e) {
        // do nothing, this is what we want
      }
    }

    // test optional args, combinged w/ anon args
    {
      PARSER_SETUP;
      CmdArg<string> argOpt("argOpt", "argument 1", "n/a", true);

      c.registerArg(argOpt);

      // w/ optional arg
      unsigned    argc   = 1;
      const char *argv[] = {
        "opt"
      };

      try {
        c.parseCmdLine(argc,argv,true,false);
        TEST_ASSERT("Should not be able to combine optional / anonymous args",0);
      } catch (invalid_argument &e) {
        // do nothing, this is what we want
      }
    }

    

    return retVal;
  }
}; // end  anon namespace

/// used to get the type name into the test msg:
#define RUN_TEST(x) ((std::cout << "Testing: " << # x << " ... " << std::endl), test_ ## x());

int main() {
  try {
    bool pass = true;

    pass &= RUN_TEST(CmdSwitch);
    pass &= RUN_TEST(CmdOptVar);
    pass &= RUN_TEST(CmdArg);
    pass &= RUN_TEST(CmdLineParser);

    cout << "ALL_TESTS_COMPLETE: " <<
      string((pass) ? "PASS" : "FAIL");
    cout << endl;
  } catch (exception &e) {
    cout << "Unexpected exception: " << e.what() << endl;
    return -1;
  }

  return 0;
}

