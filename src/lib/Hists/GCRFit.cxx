// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Hists/GCRHists.cxx,v 1.10 2007/06/07 17:45:43 fewtrell Exp $

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "GCRFit.h"
#include "GCRHists.h"
#include "HistIdx.h"
#include "../Util/CGCUtil.h"
#include "MPDHists.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"

// EXTLIB INCLUDES
#include "TDirectory.h"
#include "TTree.h"


// STD INCLUDES
#include <string>
#include <iostream>
#include <cassert>
#include <stdexcept>
#include <ostream>

using namespace std;
using namespace CalUtil;

namespace {
  /// used as ptr for each entry in tuple
  class TupleData {
  public:
    unsigned char diode;
    unsigned char inferredZ;
    float peak;
    float width;
    /// num histogram entries
    unsigned nEntries;
    float mevPerDAC;

    /// basic print out
    friend ostream& operator<< (ostream &stream,
                                const TupleData &tupleData);

    
  };

  static ostream& operator<< (ostream &stream,
                              const TupleData &td) {
    stream << "fit_result: diode: " << DiodeNum(td.diode)
           << " z: "                << (unsigned)td.inferredZ
           << " peak: "             << td.peak 
           << " width: "            << td.width
           << " nEntries: "         << td.nEntries
           << " mpd: "              << td.mevPerDAC
           << endl;

    return stream;
  }
  
  TupleData tupleData;

  /// generate ROOT tuple for fit parms
  TTree *genTuple(TDirectory *const writeFile,
                  const std::string &tupleName) {
    TTree *tuple = new TTree(tupleName.c_str(),
                             tupleName.c_str());
    assert(tuple != 0);
    if (!tuple->Branch("diode",
                       &tupleData.diode,
                       "diode/b") ||
        !tuple->Branch("inferredZ",
                       &tupleData.inferredZ,
                       "inferredZ/b")||
        !tuple->Branch("peak",
                       &tupleData.peak,
                       "peak/F")||
        !tuple->Branch("width",
                       &tupleData.width,
                       "width/F")||
        !tuple->Branch("nEntries",
                       &tupleData.nEntries,
                       "width/i")||
        !tuple->Branch("mevPerDAC",
                       &tupleData.mevPerDAC,
                       "mevPerDAC/F"))
      throw runtime_error("Unable to create TTree branches for " + tupleName);

    // set desired directory for tuple
    tuple->SetDirectory(writeFile);

    return tuple;
  }

  /// return  mevPerDAC constant for given particle z & CIDAC peak
  /// \parm z atomic # of particle
  float evalMevPerDAC(const unsigned char z, const float cidac) {
    return (float)z*z*calibGenCAL::MPDHists::MUON_ENERGY / cidac;
  }
} // anonymous namespace

namespace calibGenCAL {
  using namespace CGCUtil;

  void GCRFit::fitHists(GCRHists &histCol,
                        CalMPD &calMPD,
                        TDirectory *const writeFile,
                        const std::string &tupleName) {
    /// fit individual xtals
    GCRHists::MeanDACHistCol *const meanDACHists(histCol.getMeanDACHists());
    
    if (meanDACHists)
      for (GCRHists::MeanDACHistCol::iterator it(meanDACHists->begin());
           it != meanDACHists->end();
           it++) {
      
        //const unsigned short inferredZ(it->first.inferredZ);
        TH1S &hist(*(it->second));
           
        hist.Fit("gaus","Q");
      }

    /// setup tuple
    TTree *tuple = genTuple(writeFile, tupleName + "_meanDACSum");
    assert(tuple != 0);

           
    /// fit summary histograms
    GCRHists::MeanDACSumHistCol &meanDACSumHists(histCol.getMeanDACSumHists());
    for (GCRHists::MeanDACSumHistCol::iterator it(meanDACSumHists.begin());
         it != meanDACSumHists.end();
         it++) {
      
      
      TH1S &hist(*(it->second));

      hist.Fit("gaus","Q");

      tupleData.inferredZ = it->first.getInferredZ();
      tupleData.diode     = it->first.getDiode().val();
      tupleData.peak      = hist.GetFunction("gaus")->GetParameter(1);
      tupleData.width     = hist.GetFunction("gaus")->GetParameter(2);
      tupleData.nEntries  = hist.GetEntries();
      tupleData.mevPerDAC = evalMevPerDAC(it->first.getInferredZ(), tupleData.peak);

      LogStrm::get() << hist.GetName() << " "
                     << tupleData << endl;

      tuple->Fill();
    }
  }
} // namespace calibGenCAL 
