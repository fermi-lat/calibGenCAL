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

class MuonCalib : public RootFileAnalysis {
 public:

  /// only one constructor, but it has defaults for many values
  MuonCalib(McCfg &cfg);
  
  ~MuonCalib() {freeChildren();}

  void fillRoughPedHists(int nEvt); ///< Fill roughpedhist histograms w/ nEvt event data
  void fitRoughPedHists(); ///< Fit roughpedhist[]'s, assign means to m_calRoughPed
  void writeRoughPedsTXT(const string &filename); ///< write rough LEX8 pedestals to simple columnar .txt file
  
  void fillPedHists(int nEvt); ///< Fill pedhist histograms w/ nEvt event data
  void fitPedHists(); ///< Fit 4-range pedestals pedhist[], assign means to m_calPed
  void readCalPeds(const string &filename); ///< read 4-range pedestals in from .txt file created in previous run w/ WritePedsTXT
  void writePedsXML(const string &filename, const string &dtdFilename); ///< write 4-range pedestals out to .xml file, using official .dtd format
  void writePedsTXT(const string &filename); ///< write 4-range pedestals to simple columnar .txt file 

  void readIntNonlin(const string &filename); ///< read in TXT table of integral nonlinearity values (adc2dac) from ciFit.exe

  void fillAsymHists(int nEvt, bool genOptHists = false); ///< populate asymmetry profiles w/ nEvt worth of data.
  void populateAsymArrays(); ///< load mean values from asymmetry profiles into m_calAsym*** arrays
  void writeAsymTXT(const string &filenameLL,
                    const string &filenameLS,
                    const string &filenameSL,
                    const string &filenameSS); ///< write asymmetry tables out to text file.
  void writeAsymXML(const string &filename, const string &dtdFilename); ///< write asymmetry data to official XML file
  void readAsymTXT(const string &filenameLL,
                   const string &filenameLS,
                   const string &filenameSL,
                   const string &filenameSS); ///< read asymmetry tables in from text file(s)

  void fillMPDHists(int nEvt); ///< Fill MevPerDAC histograms w/ nEvt worth of event data.
  void fitMPDHists(); ///< Fit MevPerDAC calibration group
  void writeMPDTXT(const string &filenameL, const string &filenameS); ///< write out both MPD calibrations to text file
  void writeMPDXML(const string &filename, const string &dtdFilename); ///< write asymmetry file to official XML file

  void flushHists(); ///< writes histograms to file & closes file if m_histFile is open.  deletes all open histograms
  void openHistFile(const string &filename); ///< opens new histogram file.  closes current m_histFile if it is open

  /////////////////////////////////////
  // DEBUG / SUMMARY PRINT FUNCTIONS //
  /////////////////////////////////////
  void printAsciiHit(XtalIdx xtalIdx); ///< prints ASCII art summary of data for one xtal-hit
  void printAsciiEvt(int nEvt); ///< prints ASCII art summary of data for one full event

 private:
  void initRoughPedHists(); ///< allocate & create rough pedestal histograms & pointer array
  void initPedHists(); ///< allocate & create final pedestal histograms & pointer array
  void initAsymHists(bool genOptHists);///< allocate & create asymmetry histograms & pointer arrays
  void initMPDHists(); ///< allocate & create MevPerDAC histograms & pointer arrays



  // HISTOGRAM/PROFILE PARAMETERS - num bins, limits, etc

  /// # of points per xtal for asymmetry type data.  
  /// 1 point for the center of each orthogonal xtal excluding the two outermost xtals
  static const short N_ASYM_PTS = 10;
  static const short N_L2S_PTS = 20; ///< # of bins in dacL2S profiles

  ///////////////////////////////////////////
  //          HIT SUMMARY                  //
  //                                       //
  //   Summarize all hits in one event     //
  ///////////////////////////////////////////

  /// Summarizes hit information for one cal DigiEvent.
  class HitSummary {
  public:
    HitSummary() : ///< default ctor
      adc_ped(DiodeIdx::N_VALS),
      perLyrX(4),
      perLyrY(4),
      perColX(12),
      perColY(12)
      {
        clear();
      }

    void clear(); ///< 'zero-out' all members

    CalVec<DiodeIdx, float> adc_ped; ///< pedestal subtracted adc values 1 per diode

    // Hit summary histograms
    vector<short> perLyrX; ///< number of hits per layer
    vector<short> perLyrY; ///< number of hits per layer
    CalVec<ColNum, short> perColX; ///< # of hits per X column
    CalVec<ColNum, short> perColY; ///< # of hits per Y column

    // Hit lists
    vector<XtalIdx> hitListX; ///< list of X direction xtalId's which were hit 
    vector<XtalIdx> hitListY; ///< list of Y direction xtalId's which were hit

    // His summary 
    int count;    ///< total # of hit xtals 
    short nLyrsX; ///< total # of x layers hit
    short nLyrsY; ///< total # of y layers hit
    short nColsX; ///< total # of x columns hit
    short nColsY; ///< total # of y columns hit
    short maxPerLyr;   ///< max # of hits in any layer
    short maxPerLyrX;  ///< max # of hits in any x layer
    short maxPerLyrY;  ///<  max # of hits in any y layer
    short firstColX; ///< first hit X col (will be only hit col in good X track)
    short firstColY; ///< fisrt hit Y col (will be only hit col in good Y track)

    bool status; ///< true if all event data has been loaded;

    bool goodXTrack; ///< true if track passed selection in x direction (clean vertical 4-in row)
    bool goodYTrack; ///< true if track passed selection in y direction (clean vertical 4-in row)
  };

  // CURRENT EVENT DATA
  int m_evtId; ///< current eventId #

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

  CalVec<FaceIdx, TH1F*> m_roughPedHists; ///< list of histograms for 'rough' pedestals
  CalVec<RngIdx,  TH1F*> m_pedHists; ///< list of histograms for 'final' 4-range pedestals

  CalVec<XtalIdx, TProfile*> m_asymProfsLL; ///< list of profiles for logratio values Large diode vs Large diode over position 1 per xtal
  CalVec<XtalIdx, TProfile*> m_asymProfsLS; ///< list of profiles for logratio values Large diode vs Small diode over position 1 per xtal
  CalVec<XtalIdx, TProfile*> m_asymProfsSL; ///< list of profiles for logratio values Small diode vs Large diode over position 1 per xtal
  CalVec<XtalIdx, TProfile*> m_asymProfsSS; ///< list of profiles for logratio values Small diode vs Small diode over position 1 per xtal
  
  CalVec<XtalIdx, TProfile*> m_dacL2SProfs;  ///< profile X=bigdioedac Y=smalldioedac 1 per xtal

  CalVec<DiodeIdx, TSpline3*> m_inlSplines; ///< Collection of integral non-linearity splines, 1 per diode
  /// collection of spline functions based on LEX8 vs LEX8 asymmetry for calculating hit position in muon gain calibration (1 per xtal)
  CalVec<XtalIdx, TSpline3*> m_asym2PosSplines; 

  CalVec<XtalIdx, TH1F*> m_dacLLHists; ///< list of histograms of geometric mean(large diode dacs) for both ends on each xtal.

  CalVec<DiodeIdx, TH1F*> m_asymDACHists; ///< optional histograms of all dac values used in asymmetry calculations
  CalVec<XtalIdx, TH2F*> m_logratHistsLL; ///< optional histograms of all LL loratios used in asymmetry calculations
  CalVec<XtalIdx, TH2F*> m_logratHistsLS; ///< optional histograms of all LS loratios used in asymmetry calculations
  CalVec<XtalIdx, TH2F*> m_logratHistsSL; ///< optional histograms of all SL loratios used in asymmetry calculations
  CalVec<XtalIdx, TH2F*> m_logratHistsSS; ///< optional histograms of all SS loratios used in asymmetry calculations
  
  ///////////////////////////////////////////////////////////
  //            CALIBRATION RESULT VECTORS                 //
  //                                                       //
  // NOTE: ALL COLLECTIONS BEGIN SIZE=0.  THEY ARE EXPANDED// 
  // JUST BEFORE POPULATION                                //
  ///////////////////////////////////////////////////////////

  CalVec<FaceIdx, float> m_calRoughPed; ///< 'rough' first pass cal pedestals, all signals used (i.e. 'hits' and 'misses'), indexed by FaceIdx()
  CalVec<FaceIdx, float> m_calRoughPedErr; ///< corresponding err values for m_calRoughPed
  
  CalVec<RngIdx, float> m_calPed; ///< final pedestal values, all 4 ranges, indexed by ADC getNRng()
  CalVec<RngIdx, float> m_calPedErr; ///< corresponding err values for m_calPed

  CalVec<XtalIdx, vector<float> > m_calAsymLL; ///< 2d vector N_ASYM_PTS lograt asymvals per xtal. large pos vs large neg: idx is [xtal][pos(1-N_ASYM_PTS)]
  CalVec<XtalIdx, vector<float> > m_calAsymLS; ///< 2d vector N_ASYM_PTS lograt asymvals per xtal. large pos vs small neg: idx is [xtal][pos(1-N_ASYM_PTS)]
  CalVec<XtalIdx, vector<float> > m_calAsymSL; ///< 2d vector N_ASYM_PTS lograt asymvals per xtal. small pos vs large neg: idx is [xtal][pos(1-N_ASYM_PTS)]
  CalVec<XtalIdx, vector<float> > m_calAsymSS; ///< 2d vector N_ASYM_PTS lograt asymvals per xtal. small pos vs small neg: idx is [xtal][pos(1-N_ASYM_PTS)]
  
  CalVec<XtalIdx, vector<float> > m_calAsymLLErr; ///< corresponding error value
  CalVec<XtalIdx, vector<float> > m_calAsymLSErr; ///< corresponding error value
  CalVec<XtalIdx, vector<float> > m_calAsymSLErr; ///< corresponding error value
  CalVec<XtalIdx, vector<float> > m_calAsymSSErr; ///< corresponding error value

  CalVec<XtalIdx, float> m_calMPDLarge; ///< final mevPerDAC values for Large Diodes (1 per xtal)
  CalVec<XtalIdx, float> m_calMPDSmall; ///< final mevPerDAC values for Small Diodes (1 per xtal)

  CalVec<XtalIdx, float> m_calMPDLargeErr; ///< corresponding error value
  CalVec<XtalIdx, float> m_calMPDSmallErr; ///< corresponding error value

  /// Integral Nonlinearity spline values (adc2dac), 2d vector necessary b/c each range has different # of elements.  
  /// indexed by [DiodeIdx()][n].  since we have 4 range output we only need to use the X8 adc values for each diode
  /// 2nd dimension is initialized empty, so use push_back() to add values.
  CalVec<DiodeIdx, vector<float> > m_calInlADC;
  /// Corresponsding DAC values to go with intNonlin spline vals.  2d vector necessary b/c each range has different # of elements
  /// indexed by [diode][n]
  CalVec<DiodeIdx, vector<float> > m_calInlDAC;
  
  // Integral Nonlinearity
  void loadInlSplines(); ///< creates & populates INL splines from m_calIntNonlin;
  double adc2dac(DiodeIdx diodeIdx, double adc); ///< uses intNonlin to convert adc 2 dac for specified xtal/adc range

  // Asymmetry 2 Pos conversion
  void loadA2PSplines(); ///< creates & populates INL splines from m_calAsym
  /// uses asym2pos splines to convert asymmetry value to xtal position for energy centroid
  /// \note uses calibGenCAL internal xtal-pitch units w/ origin at negative face.
  double asym2pos(XtalIdx xtalIdx, double asym); 

  /// return longitudinal position (in mm) of a crystal center along the length of an 
  /// orthogonal crystal given a cystal column numnber
  double xtalCenterPos(short col) {
     return (m_cfg.csiLength/ColNum::N_VALS)*
         ((float)col + 0.5)  // calc for middle of segment
         - 0.5*m_cfg.csiLength;     // put 0 in middle of xtal
  }

  const McCfg &m_cfg; ///< contains all application config data.
};

#endif
