#ifndef muonCalib_h
#define muonCalib_h 1

#include <string>
#include <vector>

//ROOT INCLUDES
#include "TFile.h"
#include "TSpline.h"
#include "TProfile.h"
#include "TH2F.h"

#include "RootFileAnalysis.h"

#include "CalDefs.h"

using namespace std;

class muonCalib : public RootFileAnalysis, protected CalDefs {
public:

  /// only one constructor, but it has defaults for many values
  muonCalib(const vector<string> &digiFilenames     = vector<string>(0),
            const string &instrument                = "n/a",
            const vector<int> &towerList            = vector<int>(1,1),
            ostream &ostr                           = cout,
            const string &timestamp                 = "n/a",
            double adcThresh                        = 100,
            double cellHorPitch                     = 27.84,
            double cellVertPitch                    = 21.35,
            double maxAsymLL                        = 0.5,
            double maxAsymLS                        = 2.5,
            double maxAsymSL                        = -1.0,
            double maxAsymSS                        = 1.0,
            double minAsymLL                        = -0.5,
            double minAsymLS                        = 1.0,
            double minAsymSL                        = -2.5,
            double minAsymSS                        = -1.0);
  
  ~muonCalib() {freeChildren();}

  void fillRoughPedHists(int nEvents); ///< Fill roughpedhist histograms w/ nEvents event data
  void fitRoughPedHists(); ///< Fit roughpedhist[]'s, assign means to m_calRoughPed
  void writeRoughPedsTXT(const string &filename); ///< write rough LEX8 pedestals to simple columnar .txt file
  
  void fillPedHists(int nEvents); ///< Fill pedhist histograms w/ nEvents event data
  void fitPedHists(); ///< Fit 4-range pedestals pedhist[], assign means to m_calPed
  void readCalPeds(const string &filename); ///< read 4-range pedestals in from .txt file created in previous run w/ WritePedsTXT
  void writePedsXML(const string &filename, const string &dtdFilename); ///< write 4-range pedestals out to .xml file, using official .dtd format
  void writePedsTXT(const string &filename); ///< write 4-range pedestals to simple columnar .txt file 

  void readIntNonlin(const string &filename); ///< read in TXT table of integral nonlinearity values (adc2dac) from ciFit.exe

  void fillAsymHists(int nEvents, bool genOptHists = false); ///< populate asymmetry profiles w/ nEvents worth of data.
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

  void fillMPDHists(int nEvents); ///< Fill MevPerDac histograms w/ nEvents worth of event data.
  void fitMPDHists(); ///< Fit MevPerDac calibration group
  void writeMPDTXT(const string &filenameL, const string &filenameS); ///< write out both MPD calibrations to text file
  void writeMPDXML(const string &filename, const string &dtdFilename); ///< write asymmetry file to official XML file

  void flushHists(); ///< writes histograms to file & closes file if m_histFile is open.  deletes all open histograms
  void openHistFile(const string &filename); ///< opens new histogram file.  closes current m_histFile if it is open

  /////////////////////////////////////
  // DEBUG / SUMMARY PRINT FUNCTIONS //
  /////////////////////////////////////
  void printAsciiHit(int nXtal); ///< prints ASCII art summary of data for one xtal-hit
  void printAsciiEvent(int nEvent); ///< prints ASCII art summary of data for one full event

private:
  void initRoughPedHists(); ///< allocate & create rough pedestal histograms & pointer array
  void initPedHists(); ///< allocate & create final pedestal histograms & pointer array
  void initAsymHists(bool genOptHists);///< allocate & create asymmetry histograms & pointer arrays
  void initMPDHists(); ///< allocate & create MevPerDac histograms & pointer arrays



  // HISTOGRAM/PROFILE PARAMETERS - num bins, limits, etc

  /// # of points per xtal for asymmetry type data.  
  /// 1 point for the center of each longitudinal xtal excluding the two outermost xtals
  static const short N_ASYM_PTS = 10;
  static const short N_L2S_PTS = 20; ///< # of bins in dacL2S profiles

  ///////////////////////////////////////////
  //          HIT SUMMARY                  //
  //                                       //
  //   Summarize all hits in one event     //
  ///////////////////////////////////////////

  /// Summarizes hit information for one cal DigiEvent.
  class hitSummary {
  public:
    hitSummary() : ///< default ctor
      adc_ped(MAX_DIODE_IDX),
      perLyrX(4),
      perLyrY(4),
      perColX(12),
      perColY(12)
    {
      clear();
    }

    void clear(); ///< 'zero-out' all members

    vector<float> adc_ped; ///< pedestal subtracted adc values 1 per diode

    // Hit summary histograms
    vector<short> perLyrX; ///< number of hits per layer
    vector<short> perLyrY; ///< number of hits per layer
    vector<short> perColX; ///< # of hits per X column
    vector<short> perColY; ///< # of hits per Y column

    // Hit lists
    vector<int> hitListX; ///< list of X direction xtalId's which were hit 
    vector<int> hitListY; ///< list of Y direction xtalId's which were hit

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

  This function is not a member the eventSumamry class b/c it needs to access data members from muonCalib as
  well as hitSummary.
  */
  void summarizeHits(hitSummary &hs);  

  // CONSTRUCTOR/DESTRUCTOR HELPERS //

  /// Free any dynamic memory associated with class & other associeated resources that 
  /// are not automatically cleared (such as file *s)
  void freeChildren(); 

  // EVENT RETRIEVAL //

  /// check to see if there are nEvents following the current event.  
  /// return # of events left to process (up to requested amount)
  int checkForEvents(int nEvents);
  /// retrieve new event from digi root file
  UInt_t getEvent(UInt_t ievt);

  auto_ptr<TFile> m_histFile;  ///< Current histogram file
  string m_histFilename;  ///< name of the current output histogram ROOT file

  ///////////////////////////////////////////////////////////
  // ROOT OBJECT COLLECTIONS: HISTOGRAMS/SPLINES/PROFILES  //
  //                                                       //
  // NOTE: ALL COLLECTIONS BEGIN SIZE=0.  THEY ARE EXPANDED// 
  // JUST BEFORE POPULATION                                //
  ///////////////////////////////////////////////////////////

  vector<TH1F*> m_roughPedHists; ///< list of histograms for 'rough' pedestals
  vector<TH1F*> m_pedHists; ///< list of histograms for 'final' 4-range pedestals

  vector<TProfile*> m_asymProfsLL; ///< list of profiles for logratio values Large diode vs Large diode over position 1 per xtal
  vector<TProfile*> m_asymProfsLS; ///< list of profiles for logratio values Large diode vs Small diode over position 1 per xtal
  vector<TProfile*> m_asymProfsSL; ///< list of profiles for logratio values Small diode vs Large diode over position 1 per xtal
  vector<TProfile*> m_asymProfsSS; ///< list of profiles for logratio values Small diode vs Small diode over position 1 per xtal
  
  vector<TProfile*> m_dacL2SProfs;  ///< profile X=bigdioedac Y=smalldioedac 1 per xtal

  vector<TSpline3*> m_inlSplines; ///< Collection of integral non-linearity splines, 1 per diode
  /// collection of spline functions based on LEX8 vs LEX8 asymmetry for calculating hit position in gain calibration (1 per xtal)
  vector<TSpline3*> m_asym2PosSplines; 

  vector<TH1F*> m_dacLLHists; ///< list of histograms of geometric mean(large diode dacs) for both ends on each xtal.

  vector<TH1F*> m_asymDacHists; ///< optional histograms of all dac values used in asymmetry calculations
  vector<TH2F*> m_logratHistsLL; ///< optional histograms of all LL loratios used in asymmetry calculations
  vector<TH2F*> m_logratHistsLS; ///< optional histograms of all LS loratios used in asymmetry calculations
  vector<TH2F*> m_logratHistsSL; ///< optional histograms of all SL loratios used in asymmetry calculations
  vector<TH2F*> m_logratHistsSS; ///< optional histograms of all SS loratios used in asymmetry calculations
  
  ///////////////////////////////////////////////////////////
  //            CALIBRATION RESULT VECTORS                 //
  //                                                       //
  // NOTE: ALL COLLECTIONS BEGIN SIZE=0.  THEY ARE EXPANDED// 
  // JUST BEFORE POPULATION                                //
  ///////////////////////////////////////////////////////////

  vector<float> m_calRoughPed; ///< 'rough' first pass cal pedestals, all signals used (i.e. 'hits' and 'misses'), indexed by getNFace()
  vector<float> m_calRoughPedErr; ///< corresponding err values for m_calRoughPed
  
  vector<float> m_calPed; ///< final pedestal values, all 4 ranges, indexed by ADC getNRng()
  vector<float> m_calPedErr; ///< corresponding err values for m_calPed

  vector<vector<float> > m_calAsymLL; ///< 2d vector N_ASYM_PTS lograt asymvals per xtal. large pos vs large neg: idx is [xtal][pos(1-N_ASYM_PTS)]
  vector<vector<float> > m_calAsymLS; ///< 2d vector N_ASYM_PTS lograt asymvals per xtal. large pos vs small neg: idx is [xtal][pos(1-N_ASYM_PTS)]
  vector<vector<float> > m_calAsymSL; ///< 2d vector N_ASYM_PTS lograt asymvals per xtal. small pos vs large neg: idx is [xtal][pos(1-N_ASYM_PTS)]
  vector<vector<float> > m_calAsymSS; ///< 2d vector N_ASYM_PTS lograt asymvals per xtal. small pos vs small neg: idx is [xtal][pos(1-N_ASYM_PTS)]
  
  vector<vector<float> > m_calAsymLLErr; ///< corresponding error value
  vector<vector<float> > m_calAsymLSErr; ///< corresponding error value
  vector<vector<float> > m_calAsymSLErr; ///< corresponding error value
  vector<vector<float> > m_calAsymSSErr; ///< corresponding error value

  vector<float> m_calMPDLarge; ///< final mevPerDac values for Large Diodes (1 per xtal)
  vector<float> m_calMPDSmall; ///< final mevPerDac values for Small Diodes (1 per xtal)

  vector<float> m_calMPDLargeErr; ///< corresponding error value
  vector<float> m_calMPDSmallErr; ///< corresponding error value

  /// Integral Nonlinearity spline values (adc2dac), 2d vector necessary b/c each range has different # of elements.  
  /// indexed by [getNDiode()][n].  since we have 4 range output we only need to use the X8 adc values for each diode
  /// 2nd dimension is initialized empty, so use push_back() to add values.
  vector<vector<float> > m_calInlADC;
  /// Corresponsding ADC values to go with intNonlin spline vals.  2d vector necessary b/c each range has different # of elements
  /// indexed by [diode][n]
  vector<vector<float> > m_calInlDAC;

  // CONFIG VALUES
  string m_timestamp; ///< time of muon measurements
  string m_instrument; ///< current instrument name
  vector<int> m_towerList; ///< list of mounted LAT towers
  vector<string> m_digiFilenames; ///< list of input digi-names

  // NUMERIC CONSTANTS
  double m_adcThresh;
  
  double m_cellHorPitch;
  double m_cellVertPitch;

  double m_maxAsymLL;
  double m_maxAsymLS;
  double m_maxAsymSL;
  double m_maxAsymSS;
  double m_minAsymLL;
  double m_minAsymLS;
  double m_minAsymSL;
  double m_minAsymSS;

  // Integral Nonlinearity
  void loadInlSplines(); ///< creates & populates INL splines from m_calIntNonlin;
  double adc2dac(int nDiode, double adc); ///< uses intNonlin to convert adc 2 dac for specified xtal/adc range

  // Asymmetry 2 Pos conversion
  void loadA2PSplines(); ///< creates & populates INL splines from m_calAsym
  double asym2pos(int nXtal, double asym); ///< uses asym2pos splines to convert asymmetry value to xtal position for energy centroid
};

#endif
