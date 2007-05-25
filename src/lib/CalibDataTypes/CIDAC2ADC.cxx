// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/CalibDataTypes/CIDAC2ADC.cxx,v 1.2 2007/04/10 14:51:01 fewtrell Exp $

/** @file
    @author fewtrell
*/

// LOCAL INCLUDES
#include "CIDAC2ADC.h"
#include "../Specs/singlex16.h"

// GLAST INCLUDES

// EXTLIB INCLUDES

// STD INCLUDES
#include <fstream>
#include <sstream>

namespace calibGenCAL {

  using namespace std;
  using namespace CalUtil;
  using namespace singlex16;

  namespace {
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
  };

  const short CIDAC2ADC::INVALID_ADC = -5000;

  CIDAC2ADC::CIDAC2ADC() :
    m_splinePtsADC(RngIdx::N_VALS),
    m_splinePtsDAC(RngIdx::N_VALS)
  {
  }

  void CIDAC2ADC::writeTXT(const string &filename) const {
    ofstream outfile(filename.c_str());

    if (!outfile.is_open()) {
      ostringstream tmp;
      tmp << __FILE__ << ":" << __LINE__ << " "
          << "ERROR! unable to open txtFile='" << filename << "'";
      throw runtime_error(tmp.str());
    }

    // output header info as comment
    outfile << "; twr lyr col face rng dac adc" << endl;

    outfile.precision(2);
    outfile.setf(ios_base::fixed);

    for (RngIdx rngIdx; rngIdx.isValid(); rngIdx++)
      for (unsigned n = 0; n < m_splinePtsADC[rngIdx].size(); n++) {
        const RngNum rng = rngIdx.getRng();
        outfile << rngIdx.getTwr().val()  << " "
                << rngIdx.getLyr().val()  << " "
                << rngIdx.getCol().val()  << " "
                << rngIdx.getFace().val() << " "
                << rng.val() << " "
                << m_splinePtsDAC[rngIdx][n] << " "
                << m_splinePtsADC[rngIdx][n]
                << endl;
      }
  }

  void CIDAC2ADC::readTXT(const string &filename) {
    ifstream infile(filename.c_str());

    if (!infile.is_open()) {
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
    string line;
    while (infile.good()) {
      getline(infile, line);
      if (infile.fail()) break; // bad get

      // check for comments
      if (line[0] == ';')
        continue;

      istringstream istrm(line);

      // load in one spline val w/ coords
      istrm >> twr
            >> lyr
            >> col
            >> face
            >> rng
            >> tmpDAC
            >> tmpADC;

      const RngIdx rngIdx(twr,
                          LyrNum(lyr),
                          col,
                          FaceNum((idents::CalXtalId::XtalFace)face),
                          RngNum(rng));

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

      const float ped(m_splinePtsADC[rngIdx][0]);
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
    double        * dacs = new double[arraySize];
    double        * adcs = new double[arraySize];

    for (RngIdx rngIdx; rngIdx.isValid(); rngIdx++) {
      vector<float> &adcVec(m_splinePtsADC[rngIdx]);
      const unsigned short nADC(adcVec.size());

      // Load up
      vector<float> &dacVec(m_splinePtsDAC[rngIdx]);
      const unsigned short nDAC = dacVec.size();

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
}; // namespace calibGenCAL

