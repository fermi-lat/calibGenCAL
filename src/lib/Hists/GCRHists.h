#ifndef GCRHists_h
#define GCRHists_h
// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Hists/GCRHists.h,v 1.7 2007/06/12 17:40:46 fewtrell Exp $

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "HistIdx.h"
#include "HistVec.h"
#include "../Specs/singlex16.h"
#include "HistMap.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalVec.h"
#include "CalUtil/CalArray.h"

// EXTLIB INCLUDES
#include "TProfile.h"
#include "TH1S.h"

// STD INCLUDES
#include <string>
#include <memory>

class TDirectory;

namespace calibGenCAL {
  class CalMPD;

  /** \brief Represents GLAST Cal Optical gain calibration constants
      (MeV <-> CIDAC)

      contains read & write methods to various file formats 

      @author Zachary Fewtrell
  */
  class GCRHists {
  public:
    /// \param summaryMode enable to skip individual xtal histograms & only keep 'summary' histograms
    /// \property rootDir all histograms for this collection are loaded / saved inside this directory
    /// \param writeDir (if non-zero) all new histograms will be written out to this directory opun class destruction.
    /// \param readDir (if non-zero) any associated histograms will be read from this directory upon construction 
    GCRHists(const bool summaryMode,
             TDirectory *const writeDir=0,
             TDirectory *const readDir=0);

    /// print histogram summary info to output stream
    void summarizeHists(ostream &ostrm) const;

    /// fill all associated histograms w/ valid CIDAC hit for given channel & particle Z
    /// \parm inferredZ inferred z for particle
    void fillMeanCIDACZ(const CalUtil::XtalIdx xtalIdx,
                        const CalUtil::DiodeNum diode, 
                        const unsigned short inferredZ,
                        const float cidac);

    /// fill all associated histograms w/ valid CIDAC hit for given channel
    void fillMeanCIDAC(const CalUtil::XtalIdx xtalIdx,
                       const CalUtil::DiodeNum diode, 
                       const float cidac);

    /// fill all associated histograms w/ ratio of signal between given adc range 
    /// & next higher range
    void fillAdcRatio(const CalUtil::RngIdx rngIdx, 
                      const float thisADC,
                      const float nextADC);

    /// fill all associated histograms w/ ratio of le & he cidac signals
    void fillDACRatio(const CalUtil::FaceIdx faceIdx,
                      const float leDAC,
                      const float heDAC);
  
    /// report minimum # of entries in any histogram
    unsigned minEntries() const;

    typedef HistMap<MeanDacZId, TH1S> MeanDACHistCol;
    typedef HistMap<ZDiodeId, TH1S> MeanDACSumHistCol;
    
    /// \return 0 if collection does not exist.
    MeanDACHistCol *getMeanDACHists() { return m_meanDACHists.get();}
    MeanDACSumHistCol &getMeanDACSumHists() { return *m_meanDACSumHists.get();}
  private:
    /// load all associated histogram from m_readDir
    void loadHists(TDirectory &dir);

    /// build new empty histogram data
    void initHists();

    /// set current directory for all histograms
    void setDirectory(TDirectory *const dir);

    /// collection stores histograms indexed by tuple(inferredZ,XtalIdx,diode)
    /// as needed.
    std::auto_ptr<MeanDACHistCol> m_meanDACHists;

    /// collection stores histograms indexed by tuple(XtalIdx,DiodeNum)
    /// as needed.
    typedef HistMap<MeanDACId, TH1S> MeanDACAllZHistCol;
    std::auto_ptr<MeanDACAllZHistCol> m_meanDACAllZHists;
    
    
    /// ratio between mean LE & HE CIDAC per xtal
    typedef HistVec<CalUtil::FaceIdx, TProfile> DACRatioProfCol;
    /// optionally disbaled when m_summaryMode = false
    std::auto_ptr<DACRatioProfCol> m_dacRatioProfs;


    /// ratio between 2 adjacent ADC ranges (mean of both faces) per xtal
    /// \note outermost index from 0 -> 2 by lower of 2 compared ranges
    /// (i.e. index 0 is rng 0 vs rng 1, index 2 is rng 2 vs rng 3)
    typedef HistVec<CalUtil::RngIdx, TProfile> ADCRatioProfCol;
    /// optionally disbaled when m_summaryMode = false
    std::auto_ptr<ADCRatioProfCol> m_adcRatioProfs;

    /// sum over all xtals, outer map is for particle inferredZ, inner array for diode
    std::auto_ptr<MeanDACSumHistCol> m_meanDACSumHists;

    /// sum over all xtals & all inferredZ's
    TH1S *m_meanDACSumAllZ;

    /// histogrm inferred INFERREDZ values
    TH1S *m_zHist;

    /// sum over all xtals
    TProfile *m_dacRatioSumProf;
  
    /// sum over all xtals
    typedef HistVec<CalUtil::RngNum, TProfile> ADCRatioSumProfCol;
    std::auto_ptr<ADCRatioSumProfCol> m_adcRatioSumProfs;
  
    /// generate summary histograms only (no individual channels)
    const bool m_summaryMode;

    /// all new (& modified) histograms written to this directory
    TDirectory *const m_writeDir;
  };
}; // namespace calibGenCAL
#endif
