// TODO:
// currently ignoring tower information & run information, perhaps i should put in a check?
// why are we returning calXTalID not as a pointer? cuz only data point is one int?
// put in capability for varying #'s of settings and/or trials.  probably can't use multi-dimensional 'c' arrays if i do this. yip, gonna have to switch to hard-core STL vectors.  not on this version though!
// does 'new' initialize all data to 0?
// some kind of config file 
// how to get instrument config ?
// merge openhist/histdefine/makehistlist file into one new public function, RTA::NewHistListForDummies()
// bjarne says not to use friends.  so that could change.  particularly when i move away from c multidimensional arrays.
// not sure exactly which values to fill in for some of the xml header stuff.

#include "muonCalib.h"
#include <cstring>
#include <strstream>
#include <iomanip>
#include <fstream>

////////////////////////////////////////////////////////////////////////////////
////////// GLOBAL CONFIGURATION PARAMETERS /////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

////////// TEST INFORMATION ////////////////////////////////////////////////////
const int N_TRIALS       = 50;
const int N_XTALS        = 12;
const int N_DACS         = 173;
const int N_LYRS         = 8;
const int N_RNGS         = 4;
const int N_FACES        = 2;
const int EVTS_PER_XTAL  = N_TRIALS*N_DACS;

////////// FIT PROPERTIES //////////////////////////////////////////////////////

const int SPL_GRPWID[]  = {3,4,3,4};     // # of points to group for quadratic spline fits.
const int SPL_SKPLO[]   = {3,1,3,1};     // # of points to copy directly from beg of array (no splin)
const int SPL_SKPHI[]   = {6,10,6,10};   // # of points to copy directly from end of array (no splin)
const int SPL_NPTSMIN[] = {23,45,23,48}; // total # of points in result splines.

////////////////////////////////////////////////////////////////////////////////
/// class ciFitData - represents all output data for ciFit appliation //////////
////////////////////////////////////////////////////////////////////////////////
class ciFitData {
  friend class RootCI;

public:

  ciFitData(void);

  int FitData(void);
  int WriteSplinesXML(const char *fileName);
  int WriteSplinesTXT(const char *fileName);
  int ReadSplinesTXT (const char *fileName);

private:
  TF1 splineFunc;

  float    m_DAC[N_DACS];  // has to be a float for the fit function, otherwise is int values
  float    m_ADCsum[N_XTALS][N_LYRS][N_FACES][N_RNGS][N_DACS];
  int      m_ADCn[N_XTALS][N_LYRS][N_FACES][N_RNGS][N_DACS];
  float    m_ADCmean[N_XTALS][N_LYRS][N_FACES][N_RNGS][N_DACS];

  float    m_SplineADC[N_XTALS][N_LYRS][N_FACES][N_RNGS][N_DACS];
  int      m_numSplineADC[N_XTALS][N_LYRS][N_FACES][N_RNGS];
  int      m_SplineDAC[N_RNGS][N_DACS];
  int      m_numSplineDAC[N_RNGS];

  int fill_DAC(void);
};

ciFitData::ciFitData(void) :
  splineFunc("spline_fitter","pol2",0,4095) {

  // Initialize DAC vals
  fill_DAC();
  memset(m_ADCsum,       0, sizeof(m_ADCsum));
  memset(m_ADCn,         0, sizeof(m_ADCn));
  memset(m_ADCmean,      0, sizeof(m_ADCmean));
  memset(m_SplineADC,    0, sizeof(m_SplineADC));
  memset(m_numSplineADC, 0, sizeof(m_numSplineADC));
  memset(m_SplineDAC,    0, sizeof(m_SplineDAC));
  memset(m_numSplineDAC, 0, sizeof(m_numSplineDAC));

  int FitData(void);
}

// smooth test lines & print output.
int ciFitData::FitData(void) {
  // 2 dimensional poly line f() to use for spline fitting.

  for (int rng = 0; rng < N_RNGS; rng++) {
	 // following vals only change w/ range, so i'm getting them outside the other loops.
	 int grpWid  = SPL_GRPWID[rng];
	 int splLen  = grpWid*2 + 1;
	 int skpLo   = SPL_SKPLO[rng];
	 int skpHi   = SPL_SKPHI[rng];
	 int nPtsMin = SPL_NPTSMIN[rng];

	 // configure output stream format for rest of function
	 std::cout.setf(std::ios_base::fixed);
	 std::cout.precision(2);

	 for (int xtal = 0; xtal < N_XTALS; xtal++)
		for (int lyr = 0; lyr < N_LYRS; lyr++)
		  for (int face = 0; face < N_FACES; face++) {
			 float *curADC = m_ADCmean[xtal][lyr][face][rng];
			 // get pedestal
			 float ped     = m_ADCsum[xtal][lyr][face][rng][0] /
				m_ADCn[xtal][lyr][face][rng][0];

			 //calculate ped-subtracted means.
			 for (int dac = 0; dac < N_DACS; dac++)
				curADC[dac] =
				  m_ADCsum[xtal][lyr][face][rng][dac] /
				  m_ADCn[xtal][lyr][face][rng][dac] - ped;

			 // get upper adc boundary
			 float adc_max = curADC[N_DACS-1];
			 int last_idx = 0; // last idx will be 1st index that is > .99*adc_max
			 while (curADC[last_idx] < .99*adc_max)
				last_idx++;
			 adc_max = curADC[last_idx];

			 // set up new graph object for fitting.
			 TGraph *myGraph = new TGraph(last_idx+1,
													m_DAC,
													curADC);

			 // copy SKPLO points directly from beginning of array.
			 int spl_idx = 0;
			 for (int i = 0; i < skpLo; i++,spl_idx++) {
				m_SplineDAC[rng][spl_idx] = m_DAC[i];
				m_SplineADC[xtal][lyr][face][rng][spl_idx] = curADC[i];
				//				std::cout << setw(2) << setfill('0') << xtal << setfill(' ')
				//					  << " " << lyr
				//					  << " " << face
				//					  << " " << rng
				//					  << " " << setw(4) << setfill('0') << (int)m_DAC[i] << setfill(' ')
				//					  << " " << setw(7) << curADC[i]
				//					  << std::endl;
			 }

			 //
			 // RUN SPLINE FITS
			 //
			 // start one grp above skiplo & go as high as you can w/out entering skpHi
			 for (int cp = skpLo + grpWid - 1; // cp = 'center point'
					cp < (nPtsMin-skpLo-skpHi)*grpWid + skpLo;
					cp += grpWid, spl_idx++) {
				int lp = cp - grpWid;
				int hp  = cp + grpWid;

				myGraph->Fit(&splineFunc,"QN","",m_DAC[lp],m_DAC[hp]);
				float myPar1 = splineFunc.GetParameter(0);
				float myPar2 = splineFunc.GetParameter(1);
				float myPar3 = splineFunc.GetParameter(2);

				int   fitDAC = m_DAC[cp];
            float fitADC = myPar1 + fitDAC*(myPar2 + fitDAC*myPar3);

				// output result
				m_SplineDAC[rng][spl_idx] = fitDAC;
				m_SplineADC[xtal][lyr][face][rng][spl_idx] = fitADC;

				//				std::cout << setw(2) << setfill('0') << xtal << setfill(' ')
				//					  << " " << lyr
				//					  << " " << face
				//					  << " " << rng
				//					  << " " << setw(4) << setfill('0') << (int)fitDAC << setfill(' ')
				//					  << " " << setw(7) << fitADC
				//					  << std::endl;
			 }

			 delete myGraph;

			 // copy SKPHI points directly from face of array.
			 for (int i = (nPtsMin-skpLo-skpHi)*grpWid + skpLo;
					i <= last_idx;
					i++,spl_idx++) {
				m_SplineDAC[rng][spl_idx] = m_DAC[i];
				m_SplineADC[xtal][lyr][face][rng][spl_idx] = curADC[i];
				//				std::cout << setw(2) << setfill('0') << xtal << setfill(' ')
				//					  << " " << lyr
				//					  << " " << face
				//					  << " " << rng
				//					  << " " << setw(4) << setfill('0') << (int)m_DAC[i] << setfill(' ')
				//					  << " " << setw(7) << curADC[i]
				//					  << std::endl;
			 }

			 //
			 // UPDATE COUNTS
			 //
			 m_numSplineADC[xtal][lyr][face][rng] = spl_idx;
			 m_numSplineDAC[rng] = TMath::Max(m_numSplineDAC[rng],spl_idx);  // ensure we have just enough DAC points for largest spline
		  }
  }

  return 0;
}

int ciFitData::WriteSplinesTXT(const char *fileName) {
  ofstream outFile(fileName);
  if (!outFile.is_open()) {
	 std::cout << "ERROR! unable to open txtFile='" << fileName << "'" << std::endl;
	 return -1;
  }
  outFile.precision(2);
  outFile.setf(std::ios_base::fixed);

  for (int xtal = 0; xtal < N_XTALS; xtal++) 
	 for (int lyr = 0; lyr < N_LYRS; lyr++)
		for (int face = 0; face < N_FACES; face++)
		  for (int rng = 0; rng < N_RNGS; rng++)
			 for (int n = 0; n < m_numSplineADC[xtal][lyr][face][rng]; n++)
				outFile << xtal << " "
						  << lyr  << " "
						  << face << " "
						  << rng  << " "
						  << m_SplineDAC[rng][n] << " "
						  << m_SplineADC[xtal][lyr][face][rng][n]
						  << std::endl;
  
  return 0;
}

int ciFitData::ReadSplinesTXT (const char *fileName) {
  ifstream inFile(fileName);
  if (!inFile.is_open()) {
	 std::cout << "ERROR! unable to open txtFile='" << fileName << "'" << std::endl;
	 return -1;
  }

  int xtal, lyr, face, rng;
  int tmpDAC;
  float tmpADC;
  while (inFile.good()) {
	 // load in one spline val w/ coords
	 inFile >> xtal 
			  >> lyr
			  >> face
			  >> rng
			  >> tmpDAC
			  >> tmpADC;
	 
	 int cur_idx = m_numSplineADC[xtal][lyr][face][rng];
	 m_SplineADC[xtal][lyr][face][rng][cur_idx] = tmpADC;
	 m_SplineDAC[rng][cur_idx]                  = tmpDAC;
	 
	 // update counters
	 m_numSplineADC[xtal][lyr][face][rng]++;
	 m_numSplineDAC[rng] = TMath::Max(m_numSplineDAC[rng],cur_idx+1);
  }

  return 0;
}

int ciFitData::WriteSplinesXML(const char *fileName) {
  // RANGE menmonic strings
  char *RNG_MNEM[N_RNGS];
  RNG_MNEM[CalXtalId::LEX8] = "LEX8";
  RNG_MNEM[CalXtalId::LEX1] = "LEX1";
  RNG_MNEM[CalXtalId::HEX8] = "HEX8";
  RNG_MNEM[CalXtalId::HEX1] = "HEX1";

  // setup output file
  ofstream xmlFile(fileName);
  if (!xmlFile.is_open()) {
	 std::cout << "ERROR! unable to open xmlFile='" << fileName << "'" << std::endl;
	 return -1;
  }

  //
  // XML file header
  //
  xmlFile << "<?xml version=\"1.0\" ?>" << std::endl;
  xmlFile << "<!-- $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/ciFit.cxx,v 1.2 2004/06/02 23:02:41 fewtrell Exp $  -->" << std::endl;
  xmlFile << "<!-- Made-up  intNonlin XML file for EM, according to calCalib_v2r1.dtd -->" << std::endl;
  xmlFile << std::endl;
  xmlFile << "<!DOCTYPE calCalib SYSTEM \"../calCalib_v2r1.dtd\" [] >" << std::endl;
  xmlFile << std::endl;
  xmlFile << "<calCalib>" << std::endl;
  xmlFile << "  <generic instrument=\"EM\" timestamp=\"2003-11-2-12:56\"" << std::endl;
  xmlFile << "           calibType=\"CAL_IntNonlin\" fmtVersion=\"v2r0\">" << std::endl;
  xmlFile << std::endl;
  xmlFile << "    <inputSample startTime=\"2003-2-21-05:49:12\" stopTime=\"2003-2-24-07:07:02\"" << std::endl;
  xmlFile << "		triggers=\"random\" mode=\"normal\" source=\"stuff\" >" << std::endl;
  xmlFile << std::endl;
  xmlFile << "		Times are start and stop time of calibration run." << std::endl;
  xmlFile << "		Other attributes are just made up for code testing." << std::endl;
  xmlFile << "    </inputSample>" << std::endl;
  xmlFile << "  </generic>" << std::endl;
  xmlFile << std::endl;
  xmlFile << "<!-- EM instrument: 8 layers, 12 columns -->" << std::endl;
  xmlFile << std::endl;
  xmlFile << "<!-- number of collections of dac settings should normally be" << std::endl;
  xmlFile << "     0 (the default), if dacs aren't used to acquire data, or " << std::endl;
  xmlFile << "     equal to nRange -->" << std::endl;
  xmlFile << " <dimension nRow=\"" << 1 << "\" nCol=\"" << 1 << "\" nLayer=\"" << N_LYRS << "\" nXtal=\"" 
			 << N_XTALS << "\" nFace=\"" << N_FACES << "\" nRange=\"" << N_RNGS << "\"" << std::endl;
  xmlFile << "           nDacCol=\"" << N_RNGS << "\" />" << std::endl;

  //
  // DAC values for rest of file.
  //
  xmlFile << std::endl;
  for (int rng = 0; rng < N_RNGS; rng++) {
	 xmlFile << " <dac range=\"" << RNG_MNEM[rng] << "\"" << std::endl;
	 xmlFile << "     values=\"";
	 for (int i = 0; i < m_numSplineDAC[rng]; i++) 
		xmlFile << m_SplineDAC[rng][i] << " ";
	 xmlFile << "\"" << std::endl;
	 xmlFile << "     error=\"" << 0.1 << "\" />" << std::endl;
  }
  
  //
  // main data loop
  //
  
  xmlFile.setf(std::ios_base::fixed);
  xmlFile.precision(2);
  // TOWER // currently only using 1 tower.
  xmlFile << std::endl;
  xmlFile << " <tower iRow=\"" << 0 << "\" iCol=\"" << 0 << "\">" << std::endl;
  // LAYER //
  for (int lyr = 0; lyr < N_LYRS; lyr++) {
	 xmlFile << "  <layer iLayer=\"" << lyr << "\">" << std::endl;
	 // XTAL //
	 for (int xtal = 0; xtal < N_XTALS; xtal++) {
		xmlFile << "   <xtal iXtal=\"" << xtal << "\">" << std::endl;
		// FACE //
		for (int face = 0; face < N_FACES; face++) {
		  const char *facestr = (face == CalXtalId::NEG) ? "NEG" : "POS";
		  xmlFile << "    <face end=\"" << facestr << "\">" << std::endl;
		  // RANGE //
		  for (int rng = 0; rng < N_RNGS; rng++) {
			 xmlFile << "     <intNonlin range=\"" << RNG_MNEM[rng] << "\"" << std::endl;
			 // ADC VALS //
			 xmlFile << "             values=\"";
			 for (int i = 0; i < m_numSplineADC[xtal][lyr][face][rng]; i++) {
				xmlFile << m_SplineADC[xtal][lyr][face][rng][i] << " ";
			 }
			 xmlFile << "\"" << std::endl;
		    xmlFile << "             error=\"" << 0.1 << "\" />" << std::endl;
		  }
		  xmlFile << "    </face>" << std::endl;
		}
		xmlFile << "   </xtal>" << std::endl;
	 }
	 xmlFile << "  </layer>" << std::endl;
  }
  xmlFile << " </tower>" << std::endl;
  xmlFile << "</calCalib>" << std::endl;
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
/// class RootCI - derived from muonCalib - represents all Root input data /////
////////////////////////////////////////////////////////////////////////////////

class RootCI : public muonCalib {
public:
  /// @enum Diode Specify LE, HE, BOTH_DIODES
  typedef enum Diode {
	 LE,
    HE,
    BOTH_DIODES};

  /// Standard ctor, where user provides the names of the input root files
  /// and optionally the name of the output ROOT histogram file
  RootCI(const char *digiFileName,
			ciFitData  *cfData);

  // standard dtor
  ~RootCI(void);

  // processes one 'event', as a collection of interactions.
  // loads up histograms.
  void DigiCal(void);

  // loops through all events in file
  void Go(Int_t numEvents);

  void SetDiode(RootCI::Diode d) {m_curDiode = d;}

private:
  bool   isRngEnabled(enum CalXtalId::AdcRange rng);          // checks range against m_curDiode setting
  Diode m_curDiode;

  ciFitData *m_cfData;

};

RootCI::RootCI(const char *digiFileName, ciFitData  *cfData) :
  muonCalib(digiFileName, "", "","")
{
  m_cfData = cfData;
  m_curDiode = BOTH_DIODES;
}

// default dstor
RootCI::~RootCI(void) {
}

// right now we're presuming we know the DAC settings
// eventually we will read them from input file.
int ciFitData::fill_DAC(void) {
  //dac=[findgen(33)*2,80+findgen(28)*16,543+findgen(112)*32] ; array of dac settings

  int n = 0;
  // 0 to 64 step 2
  for (int i = 0;   i <= 64;   i+=2,  n++) m_DAC[n] = i;
  // 80 to 512 step 16
  for (int i = 80;  i <= 512;  i+=16, n++) m_DAC[n] = i;
  // 543 to 4095 step 32
  for (int i = 543; i <= 4095; i+=32, n++) m_DAC[n] = i;

  if (n != N_DACS) {
	 std::cout << "ERROR! bad DAC array!" << n << " " << N_DACS << std::endl;
	 return -1;
  }
  return 0;
}

// checks range against m_curDiode setting
bool  RootCI::isRngEnabled(enum CalXtalId::AdcRange rng) {
  if (m_curDiode == BOTH_DIODES) return true;
  if (m_curDiode == LE && (rng == CalXtalId::LEX8 || rng == CalXtalId::LEX1)) return true;
  if (m_curDiode == HE && (rng == CalXtalId::HEX8 || rng == CalXtalId::HEX1)) return true;
  return false;
}

// compiles stats for each test type.
void RootCI::DigiCal(void) {
  // Determine test config for this event
  int testXtal   = digiEventId/EVTS_PER_XTAL;
  int testDAC   = (digiEventId%EVTS_PER_XTAL)/N_TRIALS;

  const TObjArray* calDigiCol = evt->getCalDigiCol();
  if (!calDigiCol) return;
  TIter calDigiIter(calDigiCol);

  // Loop through each xtal interaction
  CalDigi *cdig = 0;
  while (cdig = (CalDigi *)calDigiIter.Next()) {  //loop through each 'hit' in one event
	 CalXtalId id = cdig->getPackedId();  // get interaction information
    int xtal = id.getColumn();
	 if (xtal != testXtal) continue;

    int lyr = id.getLayer();

	 // Loop through each readout on current xtal
	 int numRo = cdig->getNumReadouts();
	 for (int iRo=0; iRo<numRo; iRo++){
		const CalXtalReadout* acRo=cdig->getXtalReadout(iRo);

      // POS FACE
      CalXtalId::XtalFace face  = CalXtalId::POS;
      CalXtalId::AdcRange rng = (CalXtalId::AdcRange)acRo->getRange(face);
		int adc                 = acRo->getAdc(face);
		// only interested in current diode!
		if (!isRngEnabled(rng)) continue;
		// assign to table
      m_cfData->m_ADCsum[xtal][lyr][face][rng][testDAC]   += adc;
      m_cfData->m_ADCn[xtal][lyr][face][rng][testDAC]++;

		// NEG FACE
		face = CalXtalId::NEG;
		rng = (CalXtalId::AdcRange)acRo->getRange(face);
		adc = acRo->getAdc(face);
		// insanity check
		// assign to table
      m_cfData->m_ADCsum[xtal][lyr][face][rng][testDAC]   += adc;
      m_cfData->m_ADCn[xtal][lyr][face][rng][testDAC]++;

	 } // foreach readout
  } // foreach xtal
}

void RootCI::Go(Int_t numEvents)
{
  // Purpose and Method:  Event Loop

  //
  //  COMMENT OUT ANY BRANCHES YOU ARE NOT INTERESTED IN.
  //
  if (mcTree) mcTree->SetBranchStatus("*", 0);    // disable all branches

  if (digiTree) {
	 digiTree->SetBranchStatus("*",0);  // disable all branches
	 // activate desired brances
	 digiTree->SetBranchStatus("m_cal*",1);
	 digiTree->SetBranchStatus("m_eventId", 1);
	 //digiTree->SetBranchStatus("m_runId", 1);
	 //digiTree->SetBranchStatus("m_timeStamp", 1);
  }

  if (reconTree) reconTree->SetBranchStatus("*",0);  // disable all branches

  //
  // DO WE HAVE ENOUGH EVENTS IN FILE?
  //
  Int_t nentries = GetEntries();
  std::cout << "\nNum Events in File is: " << nentries << std::endl;
  Int_t curI;
  Int_t nMax = TMath::Min(numEvents+m_StartEvent,nentries);

  if (numEvents+m_StartEvent >  nentries) {
	 std::cout << " not enough entries in file to proceed, we need " << nentries << std::endl;
	 return;
  }

  // BEGINNING OF EVENT LOOP
  for (Int_t ievent=m_StartEvent; ievent<nMax; ievent++, curI=ievent) {
	 if (evt) evt->Clear();
	 if (rec) rec->Clear();

	 GetEvent(ievent);
	 // Digi ONLY analysis
	 if (evt) {
		digiEventId = evt->getEventId();
		//digiRunNum = evt->getRunId();
      if(digiEventId%1000 == 0)
		  std::cout << " event " << digiEventId << std::endl;

		DigiCal();
	 }
  }  // end analysis code in event loop

  m_StartEvent = curI;
}

int main(int argc, char **argv) {
  char inputPath1[] = "D:\\GLAST_DATA\\040408110806_calu_collect_ci_singlex16.root";
  char inputPath2[] = "D:\\GLAST_DATA\\040408105812_calu_collect_ci_singlex16.root";
  char xmlPath[]    = "../xml/ciFit.xml";
  int numEvents     = N_TRIALS*N_DACS*N_XTALS;

  ciFitData *cfData = new ciFitData;

  // Hello/Usage/config message.
  std::cout << "RootCI.exe: GLAST CAL Charge injection fitting routine." << std::endl;
  std::cout << " NOTE    : Assuming SINGLEx16 test mode,  " << N_TRIALS << " trials at " << N_DACS << " DAC settings." << std::endl;
  std::cout << " NOTE    : Assuming each diode tested separately." << std::endl;
  std::cout << " LE_FILE : " << inputPath1 << std::endl;
  std::cout << " HE_FILE : " << inputPath2 << std::endl;
  std::cout << " XML_FILE: " << xmlPath    << std::endl;
  std::cout << std::endl;

  // LE PASS
  {
	 RootCI *rd = new RootCI(inputPath1,cfData);  // too big for stack
	 // set HE/LE range
	 rd->SetDiode(RootCI::LE);
	 rd->Go(numEvents);

	 delete rd;
  }

  // HE PASS
  {
	 RootCI *rd = new RootCI(inputPath2, cfData);
    rd->SetDiode(RootCI::HE);
	 rd->Go(numEvents);

	 delete rd;
  }

  cfData->FitData();
  //cfData->ReadSplinesTXT("../output/ciSplines.txt");
  cfData->WriteSplinesTXT("../output/ciSplines.txt");
  cfData->WriteSplinesXML(xmlPath);

  delete cfData;

  return 0;
}
