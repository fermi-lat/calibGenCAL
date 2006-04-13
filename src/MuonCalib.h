#ifndef MuonCalib_h
#define MuonCalib_h

// LOCAL INCLUDES
#include "RootFileAnalysis.h"
#include "McCfg.h"
#include "CalDefs.h"
#include "CalVec.h"
#include "CalArray.h"

// GLAST INCLUDES

// EXTLIB INCLUDES
#include "TFile.h"
#include "TSpline.h"
#include "TProfile.h"
#include "TH2S.h"

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
  
  /// Fill roughpedhist histograms w/ nEvt event data
  void fillRoughPedHists(int nEvt); 
  /// Fit roughpedhist[]'s, assign means to m_calRoughPed
  void fitRoughPedHists(); 
  /// write rough LEX8 pedestals to simple columnar .txt file
  void writeRoughPedsTXT(const string &filename) const; 
  
  /// Fill pedhist histograms w/ nEvt event data
  void fillPedHists(int nEvt); 
  /// Fit 4-range pedestals pedhist[], assign means to m_calPed
  void fitPedHists();
  /// read 4-range pedestals in from .txt file created in previous run w/ WritePedsTXT
  void readPedsTXT(const string &filename); 
  /// write 4-range pedestals out to .xml file, using official .dtd format
  void writePedsXML(const string &filename, const string &dtdPath) const; 
  /// write 4-range pedestals to simple columnar .txt file 
  void writePedsTXT(const string &filename) const; 

  /// read in TXT table of integral nonlinearity values (adc2dac) from ciFit.exe
  void readIntNonlin(const string &filename); 

  /// populate asymmetry profiles w/ nEvt worth of data.
  void fillAsymHists(int nEvt); 
  /// load mean values from asymmetry profiles into m_calAsym*** arrays
  void fitAsymHists(); 
  /// write asymmetry tables out to text file.
  void writeAsymTXT(const string &filename) const;
  /// write asymmetry data to official XML file
  void writeAsymXML(const string &filename, const string &dtdPath) const; 
  /// read asymmetry tables in from text file(s)
  void readAsymTXT(const string &filename);

  /// Fill MevPerDAC histograms w/ nEvt worth of event data.
  void fillMPDHists(int nEvt); 
  /// Fit MevPerDAC calibration group
  void fitMPDHists();   
  /// write out both MPD calibrations to text file
  void writeMPDTXT(const string &filename) const; 
  /// write mevPerDAC file to official XML file
  void writeMPDXML(const string &filename, const string &dtdPath) const; 
  /// writes adc to energy conversion file to be used by online software
  /// also calculates & stores adc2nrg values.
  void writeADC2NRGXML(const string &filename); 


  /// writes histograms to file & closes file if m_histFile is open.  deletes all open histograms
  void flushHists();    
  /// opens new histogram file.  closes current m_histFile if it is open
  void openHistFile(const string &filename); 

 private:
  /// allocate & create rough pedestal histograms & pointer array
  void initRoughPedHists(); 
  /// allocate & create final pedestal histograms & pointer array
  void initPedHists();  
  /// allocate & create asymmetry histograms & pointer arrays
  void initAsymHists();
  /// allocate & create MevPerDAC histograms & pointer arrays
  void initMPDHists();  


  //-- HISTOGRAM/PROFILE PARAMETERS - num bins, limits, etc --//
  /// # of points per xtal for asymmetry type data.  
  /// 1 point for the center of each orthogonal xtal excluding the two outermost xtals
  static const short N_ASYM_PTS = 10;
  /// # of bins in dacL2S profiles
  static const short N_L2S_PTS = 20;
  /// min LEDAC val for L2S fitting
  static const short L2S_MIN_LEDAC = 10;
  /// max LEDAC val for L2S fitting
  static const short L2S_MAX_LEDAC = 60;


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
      adc_ped(tDiodeIdx::N_VALS)
      {
        clear();
      }

    void clear();       ///< 'zero-out' all members

    CalVec<tDiodeIdx, float> adc_ped; ///< pedestal subtracted adc values 1 per diode

    // Hit summary histograms
    CalArray<LyrNum, short> perLyrX; ///< number of hits per layer
    CalArray<LyrNum, short> perLyrY; ///< number of hits per layer
    CalArray<ColNum, short> perColX; ///< # of hits per X column
    CalArray<ColNum, short> perColY; ///< # of hits per Y column

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
  CalVec<tFaceIdx, TH1S*> m_roughPedHists; 
  /// list of histograms for 'final' 4-range pedestals
  CalVec<tRngIdx,  TH1S*> m_pedHists; 

  /// list of profiles for logratio values 
  CalVec<AsymType, CalArray<tXtalIdx, TH2S*> > m_asymHists; 
  
  /// profile X=bigdiodedac Y=smdiodedac 1 per xtal
  CalVec<tXtalIdx, TH1S*> m_dacL2SHists;  
  /// profile X=bigdiodedac Y=smdiodedac 1 per xtal (not used in main calib, only for finding extra l2s slope info)
  CalVec<tXtalIdx, TProfile*> m_dacL2SSlopeProfs;


  /// Collection of integral non-linearity splines, 1 per diode
  CalVec<tDiodeIdx, TSpline3*> m_inlSplines; 
  /// Collection of integral non-linearity splines, 1 per diode
  CalVec<tDiodeIdx, TSpline3*> m_inlSplinesInv; 
  /// collection of spline functions based on LEX8 vs LEX8 asymmetry for calculating hit position in muon gain calibration (1 per xtal)
  CalVec<tXtalIdx, TSpline3*> m_asym2PosSplines; 

  /// list of histograms of geometric mean for both ends on each xtal.
  CalVec<tXtalIdx, TH1S*> m_dacLLHists; 

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

  /// 2d vector N_ASYM_PTS lograt asymvals per xtal/asymType
  CalVec<AsymType, CalArray<tXtalIdx, vector<float> > > m_calAsym; 
  
  /// corresponding error value
  CalVec<AsymType, CalArray<tXtalIdx, vector<float> > > m_calAsymErr; 

  /// final mevPerDAC values for Lrg Diodes (1 per xtal)
  CalVec<DiodeNum, CalArray<tXtalIdx, float> > m_calMPD; 
  /// adc to energy conversion factors for LEX8/HEX8 ranges to be used in online software
  CalVec<tDiodeIdx, float> m_adc2nrg;

  /// corresponding error value
  CalVec<DiodeNum, CalArray<tXtalIdx, float> > m_calMPDErr; 

  /// Integral Nonlinearity spline values (adc2dac), 2d vector necessary b/c each range has different # of elements.  
  /// indexed by [tDiodeIdx()][n].  since we have 4 range output we only need to use the X8 adc values for each diode
  /// 2nd dimension is initialized empty, so use push_back() to add values.
  CalVec<tDiodeIdx, vector<float> > m_calInlADC;
  /// Corresponsding DAC values to go with intNonlin spline vals.  2d vector necessary b/c each range has different # of elements
  /// indexed by [diode][n]
  CalVec<tDiodeIdx, vector<float> > m_calInlDAC;

  //-- Integral Nonlinearity --//

  /// creates & populates INL splines from m_calIntNonlin;
  void loadInlSplines(); 
  /// uses intNonlin to convert adc 2 dac for specified xtal/adc range
  float adc2dac(tDiodeIdx diodeIdx, float adc) const; 
  /// uses intNonlin to convert dac 2 adc for specified xtal/adc range
  float dac2adc(tDiodeIdx diodeIdx, float dac) const; 


  //-- Asymmetry 2 Pos conversion --//

  /// creates & populates INL splines from m_calAsym
  void loadA2PSplines(); 
  /// uses asym2pos splines to convert asymmetry value to xtal position for energy centroid
  /// \note uses calibGenCAL internal xtal-pitch units w/ origin at negative face.
  float asym2pos(tXtalIdx xtalIdx, float asym) const; 

  /// return longitudinal position (in mm) of a crystal center along the length of an 
  /// orthogonal crystal given a cystal column numnber
  float xtalCenterPos(short col) const {
    return (m_cfg.csiLength/ColNum::N_VALS)*
      ((float)col + 0.5)  // calc for middle of segment
      - 0.5*m_cfg.csiLength;     // put 0 in middle of xtal
  }

  

  /// contains all application config data.
  const McCfg &m_cfg;   

};

#endif
