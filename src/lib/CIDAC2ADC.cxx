// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/CIDAC2ADC.cxx,v 1.7 2006/09/26 18:57:24 fewtrell Exp $

/** @file
    @author fewtrell
 */

// LOCAL INCLUDES
#include "CIDAC2ADC.h"

// GLAST INCLUDES

// EXTLIB INCLUDES

// STD INCLUDES
#include <fstream>
#include <sstream>

using namespace std;
using namespace CalUtil;

const short CIDAC2ADC::     INVALID_ADC       = -5000;

static const float          CIDAC_TEST_VALS[] =
{
  0,    2,     4,      6,      8,      10,      12,      14,      16,      18,     20,    22,    24,    26,   28,   30,
  32,
  34,   36,    38,     40,     42,     44,      46,      48,      50,      52,     54,    56,    58,    60,   62,   64,
  80,   96,    112,    128,    144,    160,     176,     192,     208,     224,    240,   256,   272,   288,
  304,  320,   336,    352,    368,    384,     400,     416,     432,     448,    464,   480,   496,   512,
  543,  575,   607,    639,    671,    703,     735,     767,     799,     831,    863,   895,   927,   959,
  991,  1023,  1055,   1087,   1119,   1151,    1183,    1215,    1247,    1279,
  1311, 1343,  1375,   1407,   1439,   1471,    1503,    1535,    1567,    1599,
  1631, 1663,  1695,   1727,   1759,   1791,    1823,    1855,    1887,    1919,
  1951, 1983,  2015,   2047,   2079,   2111,    2143,    2175,    2207,    2239,
  2271, 2303,  2335,   2367,   2399,   2431,    2463,    2495,    2527,    2559,
  2591, 2623,  2655,   2687,   2719,   2751,    2783,    2815,    2847,    2879,
  2911, 2943,  2975,   3007,   3039,   3071,    3103,    3135,    3167,    3199,
  3231, 3263,  3295,   3327,   3359,   3391,    3423,    3455,    3487,    3519,
  3551, 3583,  3615,   3647,   3679,   3711,    3743,    3775,    3807,    3839,
  3871, 3903,  3935,   3967,   3999,   4031,    4063,    4095
};

/// how many points for each smoothing 'group'?  (per adc range)
static const unsigned short SMOOTH_GRP_WIDTH[] = {
  3, 4, 3, 4
};
/// how many points at beginning of curve to extrapolate from following points
static const unsigned short EXTRAP_PTS_LO[]    = {
  2, 2, 2, 2
};
/// how many points to extrapolate beginning of curve _from_
static const unsigned short EXTRAP_PTS_LO_FROM[] = {
  5, 5, 5, 5
};
/// how many points at end of curve not to smooth (simply copy them over verbatim from raw data)
static const unsigned short SMOOTH_SKIP_HI[]   = {
  6, 10, 6, 10
};

/// number of CIDAC values tested
static const unsigned short N_CIDAC_VALS           = sizeof(CIDAC_TEST_VALS)/sizeof(*CIDAC_TEST_VALS);
/// n pulses (events) per CIDAC val
static const unsigned short N_PULSES_PER_DAC       = 50;
/// n total pulsees per xtal (or column)
static const unsigned       N_PULSES_PER_XTAL      = N_CIDAC_VALS * N_PULSES_PER_DAC;

CIDAC2ADC::CIDAC2ADC() :
  m_splinePtsADC(RngIdx::N_VALS),
  m_splinePtsDAC(RngIdx::N_VALS)
{
}

void CIDAC2ADC::writeTXT(const string &filename) const {
  ofstream outFile(filename.c_str());

  if (!outFile.is_open()) {
    ostringstream tmp;
    tmp << __FILE__ << ":" << __LINE__ << " "
        << "ERROR! unable to open txtFile='" << filename << "'";
    throw runtime_error(tmp.str());
  }
  outFile.precision(2);
  outFile.setf(ios_base::fixed);

  for (RngIdx rngIdx; rngIdx.isValid(); rngIdx++)
    for (unsigned n = 0; n < m_splinePtsADC[rngIdx].size(); n++) {
      RngNum rng = rngIdx.getRng();
      outFile << rngIdx.getTwr()  << " "
              << rngIdx.getLyr()  << " "
              << rngIdx.getCol()  << " "
              << rngIdx.getFace().val() << " "
              << rng.val() << " "
              << m_splinePtsDAC[rngIdx][n] << " "
              << m_splinePtsADC[rngIdx][n]
              << endl;
    }
}

void CIDAC2ADC::readTXT(const string &filename) {
  ifstream inFile(filename.c_str());

  if (!inFile.is_open()) {
    ostringstream tmp;
    tmp << __FILE__ << ":" << __LINE__ << " "
        << "ERROR! unable to open txtFile='" << filename << "'";
    throw runtime_error(tmp.str());
  }

  unsigned short twr;
  unsigned short lyr;
  unsigned short col;
  unsigned short face;
  unsigned short rng;
  float tmpDAC;
  float tmpADC;
  while (inFile.good()) {
    // load in one spline val w/ coords
    inFile >> twr
    >> lyr
    >> col
    >> face
    >> rng
    >> tmpDAC
    >> tmpADC;
    if (inFile.fail()) break; // quit once we can't read any more values

    RngIdx rngIdx(twr,
                  lyr,
                  col,
                  face,
                  rng);

    m_splinePtsADC[rngIdx].push_back(tmpADC);
    m_splinePtsDAC[rngIdx].push_back(tmpDAC);
  }
}

/// pedestal subtract spline point ADC by using value from first point
void CIDAC2ADC::pedSubtractADCSplines() {
  for (RngIdx rngIdx; rngIdx.isValid(); rngIdx++) {
    // skip empty splines
    if (m_splinePtsADC[rngIdx].size() == 0)
      continue;

    float ped = m_splinePtsADC[rngIdx][0];
    for (unsigned i = 0; i < m_splinePtsADC[rngIdx].size(); i++)
      m_splinePtsADC[rngIdx][i] -= ped;
  }
}

void CIDAC2ADC::genSplines() {
  m_splinesADC2DAC.resize(RngIdx::N_VALS, 0);
  m_splinesDAC2ADC.resize(RngIdx::N_VALS, 0);

  // ROOT functions take double arrays, not vectors
  // so we need to copy each vector into an array
  // before loading it into a ROOT spline
  unsigned short arraySize = 100;  // first guess for size, can resize later
  double        *dacs = new double[arraySize];
  double        *adcs = new double[arraySize];

  for (RngIdx rngIdx; rngIdx.isValid(); rngIdx++) {
    vector<float> &adcVec = m_splinePtsADC[rngIdx];
    unsigned short nADC = adcVec.size();

    // Load up
    vector<float> &dacVec = m_splinePtsDAC[rngIdx];
    unsigned short nDAC = dacVec.size();

    // skip channel if we have no data.
    if (nADC == 0)
      continue;

    if (nADC != nDAC) {
      ostringstream tmp;
      tmp << __FILE__  << ":"     << __LINE__ << " "
          << "nDAC != nADC for rngIdx = " << rngIdx.val();
      throw runtime_error(tmp.str());
    }

    // expand arrays if necessary
    if (nDAC > arraySize) {
      delete [] dacs;
      delete [] adcs;

      arraySize = nDAC;
      dacs = new double[arraySize];
      adcs = new double[arraySize];
    }

    // copy vector into array
    copy(dacVec.begin(), dacVec.end(), dacs);
    copy(adcVec.begin(), adcVec.end(), adcs);

    // generate splinename
    ostringstream name;
    name << "intNonlin_" << rngIdx.val();
    ostringstream nameInv;
    nameInv << "intNonlinInv_" << rngIdx.val();

    // create spline object.
    TSpline3     *mySpline    = new TSpline3(name.str().c_str(), adcs, dacs, nADC);
    TSpline3     *mySplineInv = new TSpline3(nameInv.str().c_str(), dacs, adcs, nADC);

    mySpline->SetName(name.str().c_str());
    m_splinesADC2DAC[rngIdx] = mySpline;

    mySplineInv->SetName(name.str().c_str());
    m_splinesDAC2ADC[rngIdx] = mySplineInv;
  }

  // cleanup
  delete [] dacs;
  delete [] adcs;
}

