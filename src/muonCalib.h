#ifndef muonCalib_h
#define muonCalib_h 1

/** @class muonCalib muonCalib handles muon calibration algorithms and calib file i/o

    muonCalib contains the following features:

    - event loop.
    - muon calibration constant generation
    - related constant storage
    - related ASCII/XML/ROOT FILE IO
    - related ROOT histograms

    \b Pedestals 


    In the first phase, pedestals are computed by accumulating pulse height histograms from crystals not hit by muons (104 events).  The resulting pedestal peaks are fit with Gaussians.  (The program is also capable of using the correlated Gaussian model for pedestal peak fitting, but this is not currently part of routine processing.)  Pedestal results, including peak positions and widths, are stored in both a text file and an XML file.  
    A sample pedestal XML file for the first two crystals is shown in Figure 5 (pedestals.xml).  The pedestal distribution peak and width (sigma) are in adc units.  They are supplied for each range of each crystal end.

    \verbatim
    <?xml version="1.0" ?>
    <!DOCTYPE calCalib SYSTEM "$(CALIBUTILROOT)/xml/calCalib_v2r1.dtd" [] >
    <calCalib>
    <generic instrument="EM" timestamp="2003-10-1-12:56" calibType="CAL_Ped" fmtVersion="v3r3p2">
    </generic>
    <dimension nRow="1" nCol="1" nLayer="8" nXtal="12" nFace="2" />
    <tower iRow="0" iCol="0">
    <layer iLayer="0">
    <xtal iXtal="0">
    <face end="NEG">
    <calPed avg="419.112" sig="5.71859" range="LEX8" />
    <calPed avg="225.457" sig="0.73131" range="LEX1" />
    <calPed avg="514.758" sig="4.32912" range="HEX8" />
    <calPed avg="257.9" sig="0.597832" range="HEX1" />
    </face>
    <face end="POS">
    <calPed avg="634.602" sig="5.78778" range="LEX8" />
    <calPed avg="203.673" sig="0.733355" range="LEX1" />
    <calPed avg="618.355" sig="4.35142" range="HEX8" />
    <calPed avg="238.426" sig="0.589113" range="HEX1" />
    </face>
    </xtal>
    <xtal iXtal="1">
    <face end="NEG">
    <calPed avg="576.902" sig="5.67201" range="LEX8" />
    <calPed avg="223.758" sig="0.72788" range="LEX1" />
    <calPed avg="645.682" sig="4.34322" range="HEX8" />
    <calPed avg="224.903" sig="0.592412" range="HEX1" />
    </face>
    <face end="POS">
    <calPed avg="551.953" sig="5.784" range="LEX8" />
    <calPed avg="219.657" sig="0.733635" range="LEX1" />
    <calPed avg="610.173" sig="4.38797" range="HEX8" />
    <calPed avg="217.313" sig="0.604085" range="HEX1" />
    </face>
    </xtal>
    \endverbatim

    Figure 5:  Sample from Pedestals.xml file for first two crystals

    \b MuPeaks


    muPeaks are simply the most probable value of the Landau model used to fit muon peaks accumulated from selected data.  They are stored together with a fractional width of the peak (i.e. sigma/mean).  They are fit in Phase II from pulse height spectra collected from the central region of each crystal.  The pulse heights are corrected for possible differences in gain between the crystal ends, for longitudinal position of the event and for path length of the muon track through the crystal.  Fitting is done twice during Phase II.  The first fit gives the relative gains between the two ends (remember that the “muPeak” stored during Phase I assume the ends are the same).  The second fit then iterates to account for changes in event selection and pulse height due to the refined gains from the first iteration.  Results are written out in both text and XML files.
    The muPeak XML file is slightly different than the text file in that it is referred to as “gains.xml” and contains 11.2 MeV/muPeak for each range for each crystal end.  This quantity is not really a gain, but rather an adc bin width, actually the reciprocal of a gain.  The fit is done strictly for LEX8, with the other ranges computed from known gain ratios.  
    Figure 6 shows a sample from a gains.xml file.  The gain ratios in this sample are not necessarily flight values.  The “gains” are in MeV/adc bin and the sigmas are fractional Landau peak widths.

    \verbatim
    <?xml version="1.0" ?>
    <!DOCTYPE calCalib SYSTEM "$(CALIBUTILROOT)/xml/calCalib_v2r1.dtd" [] >
    <calCalib>
    <generic instrument="EM" timestamp="2003-10-1-12:56" calibType="CAL_ElecGain" fmtVersion="v3r3p2">
    </generic>
    <dimension nRow="1" nCol="1" nLayer="8" nXtal="12" nFace="2" />
    <tower iRow="0" iCol="0">
    <layer iLayer="0">
    <xtal iXtal="0">
    <face end="NEG">
    <calGain avg="0.0356871" sig="0.0685552" range="LEX8" />
    <calGain avg="0.321184" sig="0.0685552" range="LEX1" />
    <calGain avg="0.192711" sig="0.0685552" range="HEX8" />
    <calGain avg="1.73439" sig="0.0685552" range="HEX1" />
    </face>
    <face end="POS">
    <calGain avg="0.0349193" sig="0.066587" range="LEX8" />
    <calGain avg="0.314274" sig="0.066587" range="LEX1" />
    <calGain avg="0.188564" sig="0.066587" range="HEX8" />
    <calGain avg="1.69708" sig="0.066587" range="HEX1" />
    </face>
    </xtal>
    <xtal iXtal="1">
    <face end="NEG">
    <calGain avg="0.0341574" sig="0.0671152" range="LEX8" />
    <calGain avg="0.307417" sig="0.0671152" range="LEX1" />
    <calGain avg="0.18445" sig="0.0671152" range="HEX8" />
    <calGain avg="1.66005" sig="0.0671152" range="HEX1" />
    </face>
    <face end="POS">
    <calGain avg="0.0344676" sig="0.0672717" range="LEX8" />
    <calGain avg="0.310209" sig="0.0672717" range="LEX1" />
    <calGain avg="0.186125" sig="0.0672717" range="HEX8" />
    <calGain avg="1.67513" sig="0.0672717" range="HEX1" />
    </face>
    </xtal>
    \endverbatim

    Figure 6:  Sample from gains.xml file for first two crystals

    \b MuSlopes


    The muSlopes, as written to the XML file, are proportional to the inverse of the slope of the best fit line to the asymmetry vs position profile histogram.  Since they are the inverse of the slope, they are a conversion factor from asymmetry to position, under the assumption that asymmetry for the central part of the crystal can be represented by a linear function.  By convention, the muSlopes values are normalized to the (P-M)/(P+M) style of asymmetry.  Since this has been supplanted by log(P/M) in the calculation, a factor of 2 has been introduced when the values are written out to XML (since old style asymmetry is normalized to the sum of the ends rather than the average).
    In addition to their use for computing event position, and given the assumption that the light taper (variation with light with distance from the diode) is equal for each end (not strictly true), the muSlopes can be used to compute light taper and hence normalize events to the center of the crystal.
    MuSlopes are stored for each crystal and range.  Their units are mm/unit asymmetry.  Figure 7 shows a sample from a muslopes.xml file.

    \verbatim
    <?xml version="1.0" ?>
    <!DOCTYPE calCalib SYSTEM "$(CALIBUTILROOT)/xml/calCalib_v2r1.dtd" [] >
    <calCalib>
    <generic instrument="EM" timestamp="2003-10-1-12:56" calibType="CAL_MuSlope" fmtVersion="v3r3p2">
    </generic>
    <dimension nRow="1" nCol="1" nLayer="8" nXtal="12" nFace="1" />
    <tower iRow="0" iCol="0">
    <layer iLayer="0">
    <xtal iXtal="0">
    <face end="NA">
    <muSlope slope="749.548" range="LEX8" />
    <muSlope slope="749.548" range="LEX1" />
    <muSlope slope="749.548" range="HEX8" />
    <muSlope slope="749.548" range="HEX1" />
    </face>
    </xtal>
    <xtal iXtal="1">
    <face end="NA">
    <muSlope slope="618.747" range="LEX8" />
    <muSlope slope="618.747" range="LEX1" />
    <muSlope slope="618.747" range="HEX8" />
    <muSlope slope="618.747" range="HEX1" />
    </face>
    </xtal>
    \endverbatim

    Figure 7:  Sample from file muslopes.xml for first two crystals

    \b Asymmetry


    The asymmetry product, stored in light_asym.xml, is just the histogram of asymmetry (log(P/M)) vs position for LE and HE diodes for each crystal.  The current product has identical values for the two diodes, but this will change in v3.  Each table contains ten numbers representing ten positions at the centers of the orthogonal crystals, excluding the end crystals.  Currently, the read routine for the table linearly extrapolates from the last two positions to get values at the ends.  Figure 8 shows a sample of a light_asym.xml file for the first two crystals.

    \verbatim
    <?xml version="1.0" ?>
    <!DOCTYPE calCalib SYSTEM "$(CALIBUTILROOT)/xml/calCalib_v2r1.dtd" [] >
    <calCalib>
    <generic instrument="EM" timestamp="2003-10-1-12:56" calibType="CAL_LightAsym" fmtVersion="v3r3p2">
    </generic>
    <dimension nRow="1" nCol="1" nLayer="8" nXtal="12" nFace="1" nRange="2" />
    <tower iRow="0" iCol="0">
    <layer iLayer="0">
    <xtal iXtal="0">
    <face end="NA">
    <lightAsym diode="LE" error="0.03"
    values=" -0.351856 -0.268391 -0.186048 -0.109585 -0.0366382 0.0349797 0.110767 0.187158 0.261037 0.338278" />
    <lightAsym diode="HE" error="0.03"
    values=" -0.351856 -0.268391 -0.186048 -0.109585 -0.0366382 0.0349797 0.110767 0.187158 0.261037 0.338278" />
    </face>
    </xtal>
    <xtal iXtal="1">
    <face end="NA">
    <lightAsym diode="LE" error="0.03"
    values=" -0.320841 -0.273547 -0.213502 -0.144366 -0.0601645 0.0450107 0.143692 0.226079 0.292944 0.349141" />
    <lightAsym diode="HE" error="0.03"
    values=" -0.320841 -0.273547 -0.213502 -0.144366 -0.0601645 0.0450107 0.143692 0.226079 0.292944 0.349141" />
    </face>
    </xtal>
    \endverbatim

    Figure 8: Sample from a light_asym.xml file showing values for the first two crystals

    \b Data Selection


    The following track related data selections are applied to all muons:
    - no more than 2 hits on any layer
    - For pedestal, muPeak calibration of x layer crystals:
    <ul>
    <li> 4 x-layers containing exactly 1 hit each
    <li> 2 y-layers containing 1-2 hits each
    <li> Reverse for y-layer crystals
    </ul>
    - For each projection (x and y), fit line using hit crystal center coordinates
    - Line defines angles from vertical, qx and qy.  For x-crystals, require:
    <ul>
    <li> |tan qy| < 0.01
    <li> |tan qx| < 0.5
    <li> Reverse for y-layer crystals
    </ul>
    

    These selections result in muons that are close to vertical when viewed in the plane perpendicular to the crystals to be calibrated and within 45 degrees of vertical when viewed in the plane parallel to the crystals to be calibrated.  This scheme results in better statistics than requiring muons to be vertical in both planes, and the varying path lengths along the crystal longitudinal direction are known and can be corrected using the angles from vertical defined above.

    For asymmetry, we want just the opposite:  We want the direction of the muons to be tightly constrained in the longitudinal direction, so we define a narrow range of longitudinal positions, but we don’t care too much about constraining the muon track in the transverse direction.  So we use the selection of muon described above but reversed i.e. the x-layer selection for muPeaks is used for the y-layer crystals for asymmetry and vice versa.
*/


#include "RootFileAnalysis.h"

//ROOT INCLUDES
#include "TFile.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TF1.h"
#include "TChain.h"

#include <string>
#include <vector>

#if !defined(__CINT__)
// Need these includes if we wish to compile this code
#else  // for interactive use
class DigiEvent;
class ReconEvent;
class McEvent;
#endif

class muonCalib : public RootFileAnalysis {
 public :

  enum GO_TYPE { FILLPEDHIST, FILLMUHIST, FILLRATHIST, FILLPEDHIST4RANGES,
					  FILLCORRPEDHIST, FILLCORRPEDHIST2RANGES };
  enum ASYM_CORR_TYPE { NONE, SLOPE, SPLINE };

  void SetAsymCorrNone() {asym_corr_type = NONE;}
  void SetAsymCorrSlope() {asym_corr_type = SLOPE;}
  void SetAsymCorrSpline() {asym_corr_type = SPLINE;}
  void SetFillPedHist() { go_type = FILLPEDHIST;}
  void SetFillPedHist4Ranges() { go_type = FILLPEDHIST4RANGES;}
  void SetFillCorrPedHist() { go_type = FILLCORRPEDHIST;}
  void SetFillCorrPedHist2Ranges() { go_type = FILLCORRPEDHIST2RANGES;}
  void SetFillMuHist() { go_type = FILLMUHIST;}
  void SetFillRatHist() { go_type = FILLRATHIST;}

  /// Default ctor, requires that that user calls muonCalib::Init
    /// to setup access to specific ROOT files.
    muonCalib();

    /// Special ctor which accepts TChains for input files
      muonCalib(std::vector<std::string> *digiFileNames,
                std::vector<std::string> *recFileNames,
                std::vector<std::string> *mcFileNames,
                const char *histFileName="muonCalib_histograms.root");

      ~muonCalib();

      void WriteHist(){ if (histFile) histFile->Write(); };
      /// Reset() all user histograms
        void HistClear();

        /// Fit calorimeter pedestal histograms
          void FitPedHist();
          /// Fit calorimeter pedestal histograms for both ranges of a same diode
            void FitCorrPedHist();
            /// Fit calorimeter log ends ratio vs position histogram
              void FitRatHist();
              ///Fit log ends signal histograms with Gaus convoluted Landau functions
                /// and log normal functions
                void FitMuHist();
  
                ///Write mu peak positions for all log ends into an ascii file
                  void WriteMuPeaks(const char* fileName);
                  int WriteMuPeaksXML(const char *fileName);
                  void ReadMuPeaks(const char* fileName);
 
                  ///Write mu slopes for all log ends into the file muslopes.txt
                    void WriteMuSlopes(const char* fileName);
                    int WriteMuSlopesXML(const char *fileName);
                    void ReadMuSlopes(const char* fileName);
                    /// Retrieve a pointer to an object stored in our output ROOT file

  
                      ///Write light asymmetry table from bin content of ratfull histograms to an ascii file
                      void WriteAsymTable(const char* fileName);
                      ///Read light asymmetry table from an ascii file and create a cubic spline for each crystal
                        void ReadAsymTable(const char* fileName);
                        int WriteAsymXML(const char *fileName);

                        void ReadCalPed(const char* fileName);
                        int WritePedXML(const char* fileName);
                        void PrintCalPed(const char* fileName);
  
                        int ReadCorrPed(const char *fileName);
                        void PrintCalCorrPed(const char* fileName);
                        int WriteCorrPedXML(const char *fileName);

                        /// process events
                          void Go(Int_t numEvents=100000);

 private:
                          void ZeroPeds();
                          TObject* GetObjectPtr(const char *tag) { return (m_histList->FindObject(tag)); };
                          /// define user histograms, ntuples and other output objects that will be saved to output
                            void HistDefine();
                            /// make list of user histograms and all objects created for output
                              void MakeHistList();
                              /// write the existing histograms and ntuples out to file
                                /// allow the user to specify their own file name for the output ROOT file
                                void SetHistFileName(const char *histFileName)
                                { m_histFileName = histFileName; }


                                /// populate m_asymTable from histograms
                                  int PopulateAsymArray();
                                  ASYM_CORR_TYPE asym_corr_type;

                                  GO_TYPE go_type;

                                  /// Histogram file
                                    TFile       *histFile;

                                    /// name of the output histogram ROOT file
                                      const char        *m_histFileName;

                                      TObjArray* pedhist;
                                      TObjArray* corrpedhist;
                                      TObjArray* pdahist;
                                      TObjArray* corrpdahist;

                                      // raw Histogram actually containing pedestal corrected histograms
                                      // it is sum of two faces
                                      TObjArray* rawhist;

                                      // raw adc histograms without any correction, for all 4 ranges
                                      TObjArray* rawAdcHist;

                                      // fully corrected muon adc. Correction include: pedestal, gain, cos(theta)
                                      // and light attenuation
                                      TObjArray* thrhist;

                                      TObjArray* adjhist;
                                      TObjArray* midhist;
                                      TObjArray* poshist;
                                      TObjArray* rathist;
                                      TObjArray* ratfull;
                                      TObjArray* asyhist;
                                      TObjArray* reshist;
                                      TObjArray* ratntup;
                                      TObjArray* asycalib;
                                      TObjArray* asymCorr;

                                      TCanvas* c1;

                                      // Graph of column(y axis) vs. layer(x axis) for a measure X layer
                                      TGraph* gx;

                                      // Graph of column(y axis) vs. layer(x axis) for a measure Y layer
                                      TGraph* gy;

                                      // functions used to fit gx and gy
                                      TF1* xline;
                                      TF1* yline;

                                      // not used
                                      TGraphErrors* glx;
                                      TGraphErrors* gly;

                                      TF1* xlongl;
                                      TF1* ylongl;
                                      TF1* land;
                                      int digi_select[8][12];

                                      // pedestal and gain corrected acd values
                                      float a[8][12][2];

                                      // pedestal corrected adc values
                                      float ar[8][12][2];

                                      /// list of user histograms
                                        THashList *m_histList;

                                        // array containing gain corrections, determined as the ration of 1000 to
                                        // mean muon signal from each crystal face; after application of these
                                        // corrections the average muon signal become 1000 at all crystal faces
                                        float m_calCorr[8][12][2];

                                        // sigma/mean in the landau fit to muon signal
                                        float m_muRelSigma[8][12][2];

                                        float m_calSlopes[8][12];
                                        float m_calPed[4][8][12][2];
                                        float m_calPedRms[4][8][12][2];
                                        float m_calCorrPed[4][8][12][2];
                                        float m_calCorrPedRms[4][8][12][2];
                                        float m_calCorrPedCos[2][8][12][2];
                                        float m_asymTable[8][12][10];

                                        /// Setup the Digitization output histograms
                                          void DigiHistDefine();

                                          /// event processing for digi CAL data
                                            void DigiCal();

                                            /// Zeros out all member vars, does NOT free memory,for use in constructor
                                              void ZeroMembers();

                                              static const std::string FACE_MNEM[];
                                              static const std::string RNG_MNEM[];
};

#endif
