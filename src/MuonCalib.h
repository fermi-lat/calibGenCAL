#ifndef MuonCalib_h
#define MuonCalib_h 1

// LOCAL INCLUDES
#include "CalDefs.h"
#include "RootFileAnalysis.h"
#include "McCfg.h"

// GLAST INCLUDES

// EXTLIB INCLUDES
#include "TFile.h"
#include "TSpline.h"
#include "TProfile.h"
#include "TH2F.h"

// STD INCLUDES
#include <string>
#include <vector>

using namespace std;
using namespace CalDefs;

/** @class MuonCalib 
    @brief GLAST Calorimeter values extracted from muons.
*/
class MuonCalib : public RootFileAnalysis {
 public:

  /// only one constructor, but it has defaults for many values
  MuonCalib(McCfg &cfg);
  
  ~MuonCalib() {freeChildren();}

  /// Fill roughpedhist histograms w/ nEvt event data
  void fillRoughPedHists(int nEvt); 
  /// Fit roughpedhist[]'s, assign means to m_calRoughPed
  void fitRoughPedHists(); 
  /// write rough LEX8 pedestals to simple columnar .txt file
  void writeRoughPedsTXT(const string &filename); 
  
  /// Fill pedhist histograms w/ nEvt event data
  void fillPedHists(int nEvt); 
  /// Fit 4-range pedestals pedhist[], assign means to m_calPed
  void fitPedHists();   
  /// read 4-range pedestals in from .txt file created in previous run w/ WritePedsTXT
  void readCalPeds(const string &filename); 
  /// write 4-range pedestals out to .xml file, using official .dtd format
  void writePedsXML(const string &filename, const string &dtdPath); 
  /// write 4-range pedestals to simple columnar .txt file 
  void writePedsTXT(const string &filename); 

  /// read in TXT table of integral nonlinearity values (adc2dac) from ciFit.exe
  void readIntNonlin(const string &filename); 

  /// populate asymmetry profiles w/ nEvt worth of data.
  void fillAsymHists(int nEvt, bool genOptHists = false); 
  /// load mean values from asymmetry profiles into m_calAsym*** arrays
  void populateAsymArrays(); 
  /// write asymmetry tables out to text file.
  void writeAsymTXT(const string &filenameLL,
                    const string &filenameLS,
                    const string &filenameSL,
                    const string &filenameSS);
  /// write asymmetry data to official XML file
  void writeAsymXML(const string &filename, const string &dtdPath); 
  /// read asymmetry tables in from text file(s)
  void readAsymTXT(const string &filenameLL,
                   const string &filenameLS,
                   const string &filenameSL,
                   const string &filenameSS);

  /// Fill MevPerDAC histograms w/ nEvt worth of event data.
  void fillMPDHists(int nEvt); 
  /// Fit MevPerDAC calibration group
  void fitMPDHists();   
  /// write out both MPD calibrations to text file
  void writeMPDTXT(const string &filenameL, const string &filenameS); 
  /// write asymmetry file to official XML file
  void writeMPDXML(const string &filename, const string &dtdPath); 
  /// writes adc to energy conversion file to be used by online software
  void writeADC2NRGXML(const string &filename); 


  /// writes histograms to file & closes file if m_histFile is open.  deletes all open histograms
  void flushHists();    
  /// opens new histogram file.  closes current m_histFile if it is open
  void openHistFile(const string &filename); 

  /////////////////////////////////////
  // DEBUG / SUMMARY PRINT FUNCTIONS //
  /////////////////////////////////////
  /// prints ASCII art summary of data for one xtal-hit
  void printAsciiHit(tXtalIdx xtalIdx); 
  /// prints ASCII art summary of data for one full event
  void printAsciiEvt(int nEvt); 

 private:
  /// allocate & create rough pedestal histograms & pointer array
  void initRoughPedHists(); 
  /// allocate & create final pedestal histograms & pointer array
  void initPedHists();  
  /// allocate & create asymmetry histograms & pointer arrays
  void initAsymHists(bool genOptHists);
  /// allocate & create MevPerDAC histograms & pointer arrays
  void initMPDHists();  



  // HISTOGRAM/PROFILE PARAMETERS - num bins, limits, etc

  /// # of points per xtal for asymmetry type data.  
  /// 1 point for the center of each orthogonal xtal excluding the two outermost xtals
  static const short N_ASYM_PTS = 10;
  /// # of bins in dacL2S profiles
  static const short N_L2S_PTS = 20; 

  ///////////////////////////////////////////
  //          HIT SUMMARY                  //
  //                                       //
  //   Summarize all hits in one event     //
  ///////////////////////////////////////////

  /// Summarizes hit information for one cal DigiEvent.
  class HitSummary {
  public:
    /// default ctor
    HitSummary() :      
      adc_ped(tDiodeIdx::N_VALS),
      perLyrX(4),
      perLyrY(4),
      perColX(12),
      perColY(12)
      {
        clear();
      }

    void clear();       ///< 'zero-out' all members

    CalVec<tDiodeIdx, float> adc_ped; ///< pedestal subtracted adc values 1 per diode

    // Hit summary histograms
    vector<short> perLyrX; ///< number of hits per layer
    vector<short> perLyrY; ///< number of hits per layer
    CalVec<ColNum, short> perColX; ///< # of hits per X column
    CalVec<ColNum, short> perColY; ///< # of hits per Y column

    // Hit lists
    vector<tXtalIdx> hitListX; ///< list of X direction xtalId's which were hit 
    vector<tXtalIdx> hitListY; ///< list of Y direction xtalId's which were hit

    // His summary 
    int count;          ///< total # of hit xtals 
    short nLyrsX;       ///< total # of x layers hit
    short nLyrsY;       ///< total # of y layers hit
    short nColsX;       ///< total # of x columns hit
    short nColsY;       ///< total # of y columns hit
    short maxPerLyr;    ///< max # of hits in any layer
    short maxPerLyrX;   ///< max # of hits in any x layer
    short maxPerLyrY;   ///<  max # of hits in any y layer
    short firstColX;    ///< first hit X col (will be only hit col in good X track)
    short firstColY;    ///< fisrt hit Y col (will be only hit col in good Y track)

    bool status;        ///< true if all event data has been loaded;

    bool goodXTrack;    ///< true if track passed selection in x direction (clean vertical 4-in row)
    bool goodYTrack;    ///< true if track passed selection in y direction (clean vertical 4-in row)
  };

  // CURRENT EVENT DATA
  int m_evtId;          ///< current eventId #

  /** \brief clear and populate m_hitSum with data from current m_evt.  

  This function is not a member the eventSumamry class b/c it needs to access data members from MuonCalib as
  well as HitSummary.
  */
  void summarizeHits(HitSummary &hs);  

  // CONSTRUCTOR/DESTRUCTOR HELPERS //

  /// Free any dynamic memory associated with class & other associeated resources that 
  /// are not automatically cleared (such as file *s)
  void freeChildren(); 

  // EVENT RETRIEVAL //

  /// check to see if there are nEvt following the current event.  
  /// return # of events left to process (up to Req amount)
  int chkForEvts(int nEvt);
  /// retrieve new event from digi root file
  UInt_t getEvent(UInt_t iEvt);

  auto_ptr<TFile> m_histFile;  ///< Current histogram file
  string m_histFilename;  ///< name of the current output histogram ROOT file

  ///////////////////////////////////////////////////////////
  // ROOT OBJECT COLLECTIONS: HISTOGRAMS/SPLINES/PROFILES  //
  //                                                       //
  // NOTE: ALL COLLECTIONS BEGIN SIZE=0.  THEY ARE EXPANDED// 
  // JUST BEFORE POPULATION                                //
  ///////////////////////////////////////////////////////////

  /// list of histograms for 'rough' pedestals
  CalVec<tFaceIdx, TH1F*> m_roughPedHists; 
  /// list of histograms for 'final' 4-range pedestals
  CalVec<tRngIdx,  TH1F*> m_pedHists; 

  /// list of profiles for logratio values Large diode vs Large diode over position 1 per xtal
  CalVec<tXtalIdx, TH2F*> m_asymHistsLL; 
  /// list of profiles for logratio values Large diode vs Small diode over position 1 per xtal
  CalVec<tXtalIdx, TH2F*> m_asymHistsLS; 
  /// list of profiles for logratio values Small diode vs Large diode over position 1 per xtal
  CalVec<tXtalIdx, TH2F*> m_asymHistsSL; 
  /// list of profiles for logratio values Small diode vs Small diode over position 1 per xtal
  CalVec<tXtalIdx, TH2F*> m_asymHistsSS; 
  
  /// profile X=bigdiodedac Y=smalldiodedac 1 per xtal
  CalVec<tXtalIdx, TProfile*> m_dacL2SProfs;  

  /// Collection of integral non-linearity splines, 1 per diode
  CalVec<tDiodeIdx, TSpline3*> m_inlSplines; 
  /// Collection of integral non-linearity splines, 1 per diode
  CalVec<tDiodeIdx, TSpline3*> m_inlSplinesInv; 
  /// collection of spline functions based on LEX8 vs LEX8 asymmetry for calculating hit position in muon gain calibration (1 per xtal)
  CalVec<tXtalIdx, TSpline3*> m_asym2PosSplines; 

  /// list of histograms of geometric mean(large diode dacs) for both ends on each xtal.
  CalVec<tXtalIdx, TH1F*> m_dacLLHists; 

  /// optional histograms of all dac values used in asymmetry calculations
  CalVec<tDiodeIdx, TH1F*> m_asymDACHists; 
  /// optional histograms of all LL loratios used in asymmetry calculations
  CalVec<tXtalIdx, TH2F*> m_logratHistsLL; 
  /// optional histograms of all LS loratios used in asymmetry calculations
  CalVec<tXtalIdx, TH2F*> m_logratHistsLS; 
  /// optional histograms of all SL loratios used in asymmetry calculations
  CalVec<tXtalIdx, TH2F*> m_logratHistsSL; 
  /// optional histograms of all SS loratios used in asymmetry calculations
  CalVec<tXtalIdx, TH2F*> m_logratHistsSS; 
  
  ///////////////////////////////////////////////////////////
  //            CALIBRATION RESULT VECTORS                 //
  //                                                       //
  // NOTE: ALL COLLECTIONS BEGIN SIZE=0.  THEY ARE EXPANDED// 
  // JUST BEFORE POPULATION                                //
  ///////////////////////////////////////////////////////////

  /// 'rough' first pass cal pedestals, all signals used (i.e. 'hits' and 'misses'), indexed by tFaceIdx()
  CalVec<tFaceIdx, float> m_calRoughPed; 
  /// corresponding err values for m_calRoughPed
  CalVec<tFaceIdx, float> m_calRoughPedErr; 
  
  /// final pedestal values, all 4 ranges, indexed by ADC getNRng()
  CalVec<tRngIdx, float> m_calPed; 
  /// corresponding err values for m_calPed
  CalVec<tRngIdx, float> m_calPedErr; 

  /// 2d vector N_ASYM_PTS lograt asymvals per xtal. large pos vs large neg: idx is [xtal][pos(1-N_ASYM_PTS)]
  CalVec<tXtalIdx, vector<float> > m_calAsymLL; 
  /// 2d vector N_ASYM_PTS lograt asymvals per xtal. large pos vs small neg: idx is [xtal][pos(1-N_ASYM_PTS)]
  CalVec<tXtalIdx, vector<float> > m_calAsymLS; 
  /// 2d vector N_ASYM_PTS lograt asymvals per xtal. small pos vs large neg: idx is [xtal][pos(1-N_ASYM_PTS)]
  CalVec<tXtalIdx, vector<float> > m_calAsymSL; 
  /// 2d vector N_ASYM_PTS lograt asymvals per xtal. small pos vs small neg: idx is [xtal][pos(1-N_ASYM_PTS)]
  CalVec<tXtalIdx, vector<float> > m_calAsymSS; 
  
  /// corresponding error value
  CalVec<tXtalIdx, vector<float> > m_calAsymLLErr; 
  /// corresponding error value
  CalVec<tXtalIdx, vector<float> > m_calAsymLSErr; 
  /// corresponding error value
  CalVec<tXtalIdx, vector<float> > m_calAsymSLErr; 
  /// corresponding error value
  CalVec<tXtalIdx, vector<float> > m_calAsymSSErr; 

  /// final mevPerDAC values for Large Diodes (1 per xtal)
  CalVec<tXtalIdx, float> m_calMPDLarge; 
  /// final mevPerDAC values for Small Diodes (1 per xtal)
  CalVec<tXtalIdx, float> m_calMPDSmall; 
  /// adc to energy conversion factors for LEX8/HEX8 ranges to be used in online software
  CalVec<tDiodeIdx, float> m_adc2nrg;

  /// corresponding error value
  CalVec<tXtalIdx, float> m_calMPDLargeErr; 
  /// corresponding error value
  CalVec<tXtalIdx, float> m_calMPDSmallErr; 

  /// Integral Nonlinearity spline values (adc2dac), 2d vector necessary b/c each range has different # of elements.  
  /// indexed by [tDiodeIdx()][n].  since we have 4 range output we only need to use the X8 adc values for each diode
  /// 2nd dimension is initialized empty, so use push_back() to add values.
  CalVec<tDiodeIdx, vector<float> > m_calInlADC;
  /// Corresponsding DAC values to go with intNonlin spline vals.  2d vector necessary b/c each range has different # of elements
  /// indexed by [diode][n]
  CalVec<tDiodeIdx, vector<float> > m_calInlDAC;
  

  //-- Generic --//
  /// loop through digi & return adc value for given face & range
  /// NOTE: they're not allways in order 0-3
  float getADCByRng(const CalDigi &calDigi, XtalRng xRng);

  //-- Integral Nonlinearity --//

  /// creates & populates INL splines from m_calIntNonlin;
  void loadInlSplines(); 
  /// uses intNonlin to convert adc 2 dac for specified xtal/adc range
  double adc2dac(tDiodeIdx diodeIdx, double adc); 
  /// uses intNonlin to convert dac 2 adc for specified xtal/adc range
  double dac2adc(tDiodeIdx diodeIdx, double dac); 


  //-- Asymmetry 2 Pos conversion --//

  /// creates & populates INL splines from m_calAsym
  void loadA2PSplines(); 
  /// uses asym2pos splines to convert asymmetry value to xtal position for energy centroid
  /// \note uses calibGenCAL internal xtal-pitch units w/ origin at negative face.
  double asym2pos(tXtalIdx xtalIdx, double asym); 

  /// return longitudinal position (in mm) of a crystal center along the length of an 
  /// orthogonal crystal given a cystal column numnber
  double xtalCenterPos(short col) {
    return (m_cfg.csiLength/ColNum::N_VALS)*
      ((float)col + 0.5)  // calc for middle of segment
      - 0.5*m_cfg.csiLength;     // put 0 in middle of xtal
  }

  

  /// contains all application config data.
  const McCfg &m_cfg;   
};

#endif
