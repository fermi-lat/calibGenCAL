// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/Thresh/fitULDSlopes.cxx,v 1.1 2008/04/21 20:43:14 fewtrell Exp $
/** @file
    @author Zachary Fewtrell

    Fit ULD THRESHOLD DAC slopes given thresholds @ different DAC settings.

    INPUTS:
    ULD thresholds in non-pedestal subtracted ADC units (TXT file) for 2 configurations.
    ULD DAC settings for 2 configurations (TXT file)
    pedestal, adc2nrg calibration file (TXT file)

    OUTPUTS:
    outut.txt: space delimited txt:
    "twr lyr col face rng mev/uld_dac mev_offset ulddac_rng uld_saturation_mev"
    stdout: full set of fit results, quality of fit metrics
    root: tuple with fit results, quality of fit metrics
*/

// LOCAL INCLUDES
#include "src/lib/Util/CfgMgr.h"
#include "src/lib/Util/string_util.h"
#include "src/lib/Util/CGCUtil.h"

// GLAST INCLUDES
#include "CalUtil/SimpleCalCalib/CalDAC.h"
#include "CalUtil/SimpleCalCalib/CalPed.h"
#include "CalUtil/SimpleCalCalib/ADC2NRG.h"

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
    pedFilename("pedFilename",
                "text file with pedestal calibration data",
                ""),
    adc2nrgFilename("muSlopeFilename",
                    "text file with muSlope (adc2mev) calibration data",
                    ""),
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
    cmdParser.registerArg(pedFilename);
    cmdParser.registerArg(adc2nrgFilename);
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

  /// pedestal calibration file
  CmdArg<string> pedFilename;

  /// adc2nrg calibration file
  CmdArg<string> adc2nrgFilename;
  
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
                       CalVec<RngIdx, float> &tholds) {
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
    unsigned short rng;

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
          >> rng
          >> thresh 
          >> threshErr;

    const RngIdx rngIdx(twr,
                         LyrNum(lyr),
                         col,
                         FaceNum((idents::CalXtalId::XtalFace)face),
                         rng);

    tholds[rngIdx] = thresh;
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
    ofstream outfile(outfilePath.c_str());
    // output column headers
    outfile << ";twr lyr col face rng slopeMeV offsetMeV dac_rng" << endl;

    /// ROOT output filename
    const string rootFilePath(cfg.outputBasename.getVal()  + ".thold_slopes.root");

    // open dac settings files
    CalDAC dac1;
    dac1.readTXT(cfg.dac1Path.getVal());
    CalDAC dac2;
    dac2.readTXT(cfg.dac2Path.getVal());

    /// load up previous calibrations
    CalPed calPed;
    LogStrm::get() << __FILE__ << ": calib file: " << cfg.pedFilename.getVal() << endl;
    calPed.readTXT(cfg.pedFilename.getVal());
    /// load up previous calibrations
    ADC2NRG adc2nrg;
    LogStrm::get() << __FILE__ << ": calib file: " << cfg.adc2nrgFilename.getVal() << endl;
    adc2nrg.readTXT(cfg.adc2nrgFilename.getVal());

    // fill thold arrays
    CalVec<RngIdx, float> thresh1, thresh2;
    readThresholdsTXT(cfg.thresh1Path.getVal(),
                     thresh1);
    readThresholdsTXT(cfg.thresh2Path.getVal(),
                     thresh2);
    

    TFile f(rootFilePath.c_str(),"RECREATE");
    TNtuple* nt = new TNtuple("nttholddac","nttholddac",
                              "twr:lyr:col:face:rng:tholdoffMeV:tholdslopeMeV:dac1:dac2:thresh1ADC:thresh2ADC:thresh1MeV:thresh2MeV:uldSaturationMeV");

    LogStrm::get() << ";twr lyr col face rng slopeMeV offsetMeV dac1 dac2 thresh1 thresh2 thresh1MeV thresh2MeV uldSaturationMeV" << endl;

    for (RngIdx rngIdx; rngIdx.isValid(); rngIdx++) {
      /// there is no ULD for HEX1 range
      if (rngIdx.getRng() == HEX1)
        continue;

      const float thresh1ADC = thresh1[rngIdx];
      // check that threshold exists
      if (thresh1ADC == INVALID_THRESH)
        continue;
      const float thresh2ADC = thresh2[rngIdx];
      if (thresh2ADC == INVALID_THRESH) {
        LogStrm::get() << "ERROR: missing threshold info for channel : " << rngIdx.toStr() << " in " << cfg.thresh2Path.getVal() << endl;
        continue;
      }

      const float ped = calPed.getPed(rngIdx);
      const float adc2mev = adc2nrg.getADC2NRG(rngIdx);
      const float thresh1MeV = (thresh1ADC - ped)*adc2mev;
      const float thresh2MeV = (thresh2ADC - ped)*adc2mev;
      

      const float ftwr  = rngIdx.getTwr().val();
      const float flyr  = rngIdx.getLyr().val();
      const float fcol  = rngIdx.getCol().val();
      const float fface = rngIdx.getFace().val();
      const float frng  = rngIdx.getRng().val();

      const float d1 = dac1.getDAC(rngIdx.getFaceIdx());
      const float d2 = dac2.getDAC(rngIdx.getFaceIdx());

      /// test that both DAC settings are in the same range
      if (d1 >= 64 != d2 >= 64) {
        LogStrm::get() << "ERROR: dac settings (" << d1 << ", " << d2 << ") not in same range, channel= " << rngIdx.toStr() << endl;
        continue;
      }

      /// select COARSE or FINE DAC range
      const unsigned short dac_rng = (d1 < 64) ? 0 : 1;


      const float slope = (thresh2MeV-thresh1MeV)/(d2-d1);
      const float saturationMeV = (4095 - ped)*adc2mev;
      float offset;

      /// move offset to dac=64 for coarse range channels
      if (d1 <= 63) {
        offset = thresh1MeV - d1*slope;
      } else {
        offset = thresh1MeV - (d1-64)*slope;
      }

      outfile << (unsigned)ftwr 
              << " " << (unsigned)flyr 
              << " " << (unsigned)fcol 
              << " " << (unsigned)fface 
              << " " << (unsigned)frng
              << " " << slope 
              << " " << offset 
              << " " << dac_rng 
              << " " << saturationMeV
              << endl; 
                                        
      nt->Fill(ftwr,flyr,fcol,fface,frng,offset,slope,d1,d2,thresh1ADC,thresh2ADC,thresh1MeV,thresh2MeV, saturationMeV);

      LogStrm::get() << (int)ftwr << " "
                     << (int)flyr << " "
                     << (int)fcol << " "
                     << (int)fface << " "
                     << (int)frng << " "
                     << slope << " "
                     << offset << " "
                     << d1 << " "
                     << d2 << " "
                     << thresh1ADC << " "
                     << thresh2ADC << " "
                     << thresh1MeV << " "
                     << thresh2MeV << " "
                     << saturationMeV << " " 
                     << endl;
    }

    nt->Write();
    f.Close();

  } catch (exception &e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
    return -1;
  }

  return 0;
}
