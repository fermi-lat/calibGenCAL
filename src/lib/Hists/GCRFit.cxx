// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Hists/GCRFit.cxx,v 1.1 2007/06/12 17:40:46 fewtrell Exp $

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "GCRFit.h"
#include "GCRHists.h"
#include "HistIdx.h"
#include "MPDHists.h"
#include "../Util/CGCUtil.h"
#include "../Util/ROOTUtil.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"

// EXTLIB INCLUDES
#include "TDirectory.h"
#include "TTree.h"
#include "TF1.h"


// STD INCLUDES
#include <string>
#include <iostream>
#include <cassert>
#include <stdexcept>
#include <ostream>
#include <algorithm>

using namespace std;
using namespace CalUtil;


namespace calibGenCAL {
  
  namespace GCRFit {
    /// return  mevPerDAC constant for given particle z & CIDAC peak
    /// \parm z atomic # of particle
    static float evalMevPerDAC(const unsigned char z, const float cidac) {
      return (float)z*z*calibGenCAL::MPDHists::MUON_ENERGY / cidac;
    }

    static float defaultMPD(DiodeNum diode) {
      const float defaultMPD[DiodeNum::N_VALS] = {.36,25};

      return defaultMPD[diode.val()];
    }
      
    static float defaultDACPeak(DiodeNum diode, unsigned char z) {
      return z*z*calibGenCAL::MPDHists::MUON_ENERGY/defaultMPD(diode);
    }

    /// tools for fitting GCR hists w/ simple gaussian shape
    namespace GCRFitGaus {
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
  
      /// generate ROOT tuple for fit parms
      static TTree *genTuple(TDirectory *const writeFile,
                      const std::string &tupleName,
                      TupleData &tupleData) {
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
    } // namespace GCRFitGaus


    void gcrFitGaus(GCRHists &histCol,
                    CalMPD &calMPD,
                    TDirectory *const writeFile,
                    const std::string &tupleName) {
      using namespace GCRFitGaus;
      const string funcName("gcrGaus");
      
      TF1 func(funcName.c_str(),"gaus",0,4095);

      /// fit individual xtals
      GCRHists::MeanDACHistCol *const meanDACHists(histCol.getMeanDACHists());
    
      if (meanDACHists)
        for (GCRHists::MeanDACHistCol::iterator it(meanDACHists->begin());
             it != meanDACHists->end();
             it++) {
      
          TH1S &hist(*(it->second));
          GCRHists::MeanDACHistCol::IdxType idx(it->first);
          const unsigned int z = idx.getInferredZ();
          /// skip unidentified particles
          if (z == 0)
              continue;
           
          hist.Fit(&func,"Q");
        }

      /// setup tuple
      TupleData tupleData;
      TTree *const tuple = genTuple(writeFile, tupleName + "_meanDACSum", tupleData);
      assert(tuple != 0);

           
      /// fit summary histograms
      GCRHists::MeanDACSumHistCol &meanDACSumHists(histCol.getMeanDACSumHists());
      for (GCRHists::MeanDACSumHistCol::iterator it(meanDACSumHists.begin());
           it != meanDACSumHists.end();
           it++) {      
      
        TH1S &hist(*(it->second));
        const GCRHists::MeanDACSumHistCol::IdxType &idx(it->first);
        const unsigned char z(idx.getInferredZ());
        /// skip unidentified particles
        if (z == 0)
            continue;
        
        removeOutliers(hist);

        /// fit w/in general region of expected peak
        hist.Fit(&func, "Q", "");

        tupleData.inferredZ = z;
        tupleData.diode     = it->first.getDiode().val();
        tupleData.peak      = hist.GetFunction(funcName.c_str())->GetParameter(1);
        tupleData.width     = hist.GetFunction(funcName.c_str())->GetParameter(2);
        tupleData.nEntries  = hist.GetEntries();
        tupleData.mevPerDAC = evalMevPerDAC(it->first.getInferredZ(), tupleData.peak);

        LogStrm::get() << hist.GetName() << " "
                       << tupleData << endl;

        tuple->Fill();
      }
    } // gcrFitGaus
  } // namespace GCRFit
} // namespace calibGenCAL 
