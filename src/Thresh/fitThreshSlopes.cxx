// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/Thresh/fitThreshSlopes.cxx,v 1.1 2008/04/21 20:43:14 fewtrell Exp $
/** @file
    @author Zachary Fewtrell

    Fit THRESHOLD DAC slopes given thresholds @ different DAC settings.
*/

// LOCAL INCLUDES
#include "src/lib/Util/CfgMgr.h"
#include "src/lib/Util/string_util.h"
#include "src/lib/Util/CGCUtil.h"

// GLAST INCLUDES
#include "CalUtil/SimpleCalCalib/CalDAC.h"

// EXTLIB INCLUDES
#include "TNtuple.h"
#include "TFile.h"

// STD INCLUDES
#include <fstream>

using namespace std;
using namespace CfgMgr;
using namespace calibGenCAL;
using namespace CalUtil;

/// Manage application configuration parameters
class AppCfg {
public:
  AppCfg(const int argc,
         const char **argv) :
    cmdParser(path_remove_ext(__FILE__)),
    dac1Path("dac1Path",
             "pathname for dac settings for 1st fitted point",
             ""),
    thresh1Path("thresh1Path",
                "pathname for energy thresholds for 1st fitted point",
                ""),
    dac2Path("dac2Path",
             "pathname for dac settings for 2nd fitted point",
             ""),
    thresh2Path("thresh2Path",
                "pathname for energy thresholds for 2nd fitted point",
                ""),
    outputBasename("outputBasename",
                   "all output files will use this basename + some_ext",
                   ""),
    help("help",
         'h',
         "print usage info")
  {
    cmdParser.registerArg(dac1Path);
    cmdParser.registerArg(thresh1Path);
    cmdParser.registerArg(dac2Path);
    cmdParser.registerArg(thresh2Path);
    cmdParser.registerArg(outputBasename);
    cmdParser.registerSwitch(help);

    try {
      cmdParser.parseCmdLine(argc, argv);
    } catch (exception &e) {
      // ignore invalid commandline if user asked for help.
      if (!help.getVal())
        cout << e.what() << endl;
      cmdParser.printUsage();
      exit(-1);
    }


  }

  /// construct new parser
  CmdLineParser cmdParser;

  /// threshold_dac settings for 1st fitting point
  CmdArg<string> dac1Path;
  /// thresholds (MeV) for 1st fitting point
  CmdArg<string> thresh1Path;
  /// threshold_dac settings for 2st fitting point
  CmdArg<string> dac2Path;
  /// thresholds (MeV) for 2st fitting point
  CmdArg<string> thresh2Path;
  /// all output files will add unique filename extensions to shared basename
  CmdArg<string> outputBasename;

  /// print usage string
  CmdSwitch help;

};

/// used to mark missing data items
static const float INVALID_THRESH = -5e6;

/// read in threshold data for each xtal face from TXT
/// missing elements of output array will be populated with INVALID_THRESH
/// @parm path path to input file
/// @parm tholds output array
void readThresholdsTXT(const std::string &path,
                       CalVec<FaceIdx, float> &tholds) {
  /// mark all channels invalid @ first
  fill(tholds.begin(),
       tholds.end(),
       INVALID_THRESH);
       

  ifstream infile(path.c_str());

  if (!infile.is_open())
    throw runtime_error(string("Unable to open " + path));

  string line;
  while (infile.good()) {
    float thresh;
    float threshErr;
    unsigned short twr;
    unsigned short lyr;
    unsigned short col;
    unsigned short face;

    getline(infile, line);
    if (infile.fail()) break; // bad get

    // check for comments
    if (line[0] == ';')
      continue;

    istringstream istrm(line);

    istrm >> twr
          >> lyr
          >> col
          >> face
          >> thresh 
          >> threshErr;

    const FaceIdx faceIdx(twr,
                          LyrNum(lyr),
                          col,
                          FaceNum((idents::CalXtalId::XtalFace)face));

    tholds[faceIdx] = thresh;
  }
}

int main(const int argc, const char **argv) {
  // libCalibGenCAL will throw runtime_error
  try {
    AppCfg cfg(argc,argv);

    //-- SETUP LOG FILE --//
    /// multiplexing output streams
    /// simultaneously to cout and to logfilePath
    LogStrm::addStream(cout);

    // generate logfilePath name
    const string logfilePath(cfg.outputBasename.getVal() + ".thold_slopes.log.txt");
    ofstream tmpStrm(logfilePath.c_str());
    LogStrm::addStream(tmpStrm);

    // generate output filename
    const string outfilePath(cfg.outputBasename.getVal() + ".thold_slopes.txt");
    LogStrm::get() << __FILE__ << ": Opening output TXT file: " << outfilePath << endl;
    ofstream outfile(outfilePath.c_str());
    // output column headers
    outfile << ";twr lyr col face slope offset" << endl;

    /// ROOT output filename
    const string rootFilePath(cfg.outputBasename.getVal()  + ".thold_slopes.root");
    

    // open dac settings files
    CalDAC dac1;
    LogStrm::get() << __FILE__ << ": reading dac settings file: " << cfg.dac1Path.getVal() << endl;
    dac1.readTXT(cfg.dac1Path.getVal());
    CalDAC dac2;
    LogStrm::get() << __FILE__ << ": reading dac settings file: " << cfg.dac2Path.getVal() << endl;
    dac2.readTXT(cfg.dac2Path.getVal());

    // fill thold arrays
    CalVec<FaceIdx, float> thresh1, thresh2;
    readThresholdsTXT(cfg.thresh1Path.getVal(),
                     thresh1);
    readThresholdsTXT(cfg.thresh2Path.getVal(),
                     thresh2);
    

    LogStrm::get() << ": Opening output ROOT file: " << rootFilePath << endl;
    TFile f(rootFilePath.c_str(),"RECREATE");
    TNtuple* nt = new TNtuple("nttholddac","nttholddac","twr:lyr:col:face:tholdoff:tholdslope:dac0:dac1:thresh0:thresh1");

    LogStrm::get() << ";twr lyr col face slope offset dac0 dac1 thresh0 thresh1" << endl;

    for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++) {
      const float thresh0 = thresh1[faceIdx];
      // check that threshold exists
      if (thresh0 == INVALID_THRESH)
        continue;
      const float thresh1 = thresh2[faceIdx];
      if (thresh1 == INVALID_THRESH) {
        LogStrm::get() << "ERROR: missing threshold info for channel : " << faceIdx.toStr() << " in " << cfg.thresh2Path.getVal() << endl;
        continue;
      }


      const float ftwr = faceIdx.getTwr().val();
      const float flyr = faceIdx.getLyr().val();
      const float fcol = faceIdx.getCol().val();
      const float fface = faceIdx.getFace().val();

      const float dac0 = dac1.getDAC(faceIdx);
      const float dac1 = dac2.getDAC(faceIdx);

      /// test that both DAC settings are in the same range
      if (dac0 >= 64 != dac1 >= 64) {
        LogStrm::get() << "ERROR: dac settings (" << dac0 << ", " << dac1 << ") not in same range, channel= " << faceIdx.toStr() << endl;
        continue;
      }

      /// select COARSE or FINE DAC range
      const unsigned short dac_rng = (dac0 < 64) ? 0 : 1;


      const float slope = (thresh1-thresh0)/(dac1-dac0);
      float offset;

      /// move offset to dac=64 for coarse range channels
      if (dac1 <= 63) {
        offset = thresh1-dac1*slope;
      } else {
        offset = thresh1 - (dac1-64)*slope;
      }

      outfile << (unsigned)ftwr 
              << " " << (unsigned)flyr 
              << " " << (unsigned)fcol 
              << " " << (unsigned)fface 
              << " " << slope 
              << " " << offset 
              << " " << dac_rng 
              << endl; 
                                        
      nt->Fill(ftwr,flyr,fcol,fface,offset,slope,dac0,dac1,thresh0,thresh1);

      LogStrm::get() << (int)ftwr<< " "
                     << (int)flyr<< " "
                     << (int)fcol<< " "
                     << (int)fface<< " "
                     << slope<< " "
                     << offset<< " "
                     << dac0<< " "
                     << dac1<< " "
                     << thresh0<< " "
                     << thresh1 << endl;
    }

    nt->Write();
    f.Close();

  } catch (exception &e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
    return -1;
  }

  return 0;
}
