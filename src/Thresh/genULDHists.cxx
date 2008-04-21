/** @file
    @author Zachary Fewtrell

    Generate ULD threshold histograms for each crystal face from best-range-first LPA data.
    Histograms are in non pedestal subtracted adc units.
    Histograms are filled from first readout only
*/

// LOCAL INCLUDES
#include "src/lib/Util/CfgMgr.h"
#include "src/lib/Util/RootFileAnalysis.h"
#include "src/lib/Util/CGCUtil.h"
#include "src/lib/Util/string_util.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalVec.h"
#include "digiRootData/DigiEvent.h"

// EXTLIB INCLUDES
#include "TFile.h"
#include "TH1S.h"

// STD INCLUDES
#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;
using namespace CfgMgr;
using namespace calibGenCAL;
using namespace CalUtil;


// CONSTANTS
namespace {
  static const unsigned short ULDHIST_MIN_ADC = 3095;
  static const unsigned short ULDHIST_MAX_ADC = 4095;
  static const unsigned short ULDHIST_N_BINS = 100;
}

/// Manage application configuraiton parameters
class AppCfg {
public:
  AppCfg(const int argc,
         const char **argv) :
    cmdParser(path_remove_ext(__FILE__)),
    digiFilenames("digiFilenames",
                  "text file w/ newline delimited list of input digi ROOT files",
                  ""),
    outputBasename("outputBasename",
                   "all output files will use this basename + some_ext",
                   ""),
    nEvts("numEvents",
          "number of events to process",
          0),
    help("help",
         'h',
         "print usage info")
  {
    cmdParser.registerArg(digiFilenames);
    cmdParser.registerArg(outputBasename);
    cmdParser.registerArg(nEvts);
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

  CmdArg<string> digiFilenames;
  CmdArg<string> outputBasename;

  CmdArg<unsigned> nEvts;

  /// print usage string
  CmdSwitch help;

};


int main(const int argc, const char **argv) {
  // libCalibGenCAL will throw runtime_error
  try {
    AppCfg cfg(argc,argv);

    //-- SETUP LOG FILE --//
    /// multiplexing output streams
    /// simultaneously to cout and to logfile
    LogStrm::addStream(cout);

    // generate logfile name
    const string logfile(cfg.outputBasename.getVal() + ".uld_hist.log.txt");
    ofstream tmpStrm(logfile.c_str());
    LogStrm::addStream(tmpStrm);

    // generate output ROOT filename
    const string outputPath(cfg.outputBasename.getVal() + ".uld_hist.root");

    // open input files
    // input file(s)
    vector<string> digiFileList(getLinesFromFile(cfg.digiFilenames.getVal()));
    if (digiFileList.size() < 1) {
      cout << __FILE__ << ": No input files specified" << endl;
      return -1;
    }
    RootFileAnalysis rootFile(0,
                              &digiFileList,
                              0);

    // open output files
    TFile output(outputPath.c_str(),"RECREATE");

    // ENABLE / REGISTER TUPLE BRANCHES
    rootFile.getDigiChain()->SetBranchStatus("*", 0);
    rootFile.getDigiChain()->SetBranchStatus("m_calDigiCloneCol");

    // create ULD threshold histograms, one per range (skip HEX1)
    CalVec<RngIdx, TH1S*> uldHist;
    for (RngIdx rngIdx; rngIdx.isValid(); rngIdx++) {
      if (rngIdx.getRng() == HEX1)
        continue;
      ostringstream histname;
      histname << "uld_" << rngIdx.toStr();

      uldHist[rngIdx] = new TH1S(histname.str().c_str(),
                                 histname.str().c_str(),
                                 ULDHIST_N_BINS,
                                 ULDHIST_MIN_ADC,
                                 ULDHIST_MAX_ADC);
    }

    // EVENT LOOP
    const unsigned  nEvents = min<unsigned>(rootFile.getEntries(), cfg.nEvts.getVal());
    LogStrm::get() << __FILE__ << ": Processing: " << nEvents << " events." << endl;

    for (unsigned nEvt = 0;
         nEvt < nEvents;
         nEvt++) {

      // read new event
      rootFile.getEvent(nEvt);
      DigiEvent const*const digiEvent = rootFile.getDigiEvent();
      if (!digiEvent) {
        LogStrm::get() << __FILE__ << ": Unable to read DigiEvent " << nEvt  << endl;
        continue;
      }

      // status print out
      if (nEvt % 10000 == 0)
        LogStrm::get() << nEvt << endl;

      //-- loop through each 'hit' in one event --//
      const TClonesArray *calDigiCol = digiEvent->getCalDigiCol();
      if (!calDigiCol) {
        LogStrm::get() << "no calDigiCol found." << endl;
        return -1;
      }
      TIter calDigiIter(calDigiCol);
      const CalDigi      *pCalDigi = 0;
      while ((pCalDigi = dynamic_cast<CalDigi *>(calDigiIter.Next()))) {
        const     CalDigi &calDigi = *pCalDigi;    // use reference to avoid -> syntax
        //-- XtalId --//
        const idents::CalXtalId id(calDigi.getPackedId());   // get interaction information
        const XtalIdx xtalIdx(id);

        for (FaceNum face; face.isValid(); face++) {
          const RngNum rng(calDigi.getRange(0, (CalXtalId::XtalFace)face.val()));
          if (rng == HEX1)
            continue;

          const unsigned short adc(calDigi.getAdc(0, (CalXtalId::XtalFace)face.val()));
          if (adc > ULDHIST_MIN_ADC) {
            const RngIdx rngIdx(xtalIdx, face, rng);
            uldHist[rngIdx]->Fill(adc);
          }
        }
      }
    }

    output.Write();
    output.Close();
      
  } catch (exception &e) {
    cout << __FILE__ << ": exception thrown: " << e.what() << endl;
    return -1;
  }

  return 0;
}
