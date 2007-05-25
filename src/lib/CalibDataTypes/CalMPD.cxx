// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/CalibDataTypes/CalMPD.cxx,v 1.2 2007/04/10 14:51:01 fewtrell Exp $

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "CalMPD.h"

// GLAST INCLUDES

// EXTLIB INCLUDES

// STD INCLUDES
#include <fstream>
#include <sstream>

namespace calibGenCAL {

  using namespace std;
  using namespace CalUtil;

  const short CalMPD::INVALID_MPD = -5000;

  CalMPD::CalMPD() :
    m_mpd(DiodeNum::N_VALS),
    m_mpdErr(DiodeNum::N_VALS)
  {
    for (DiodeNum diode; diode.isValid(); diode++)
      m_mpd[diode].fill(INVALID_MPD);
  }

  void CalMPD::writeTXT(const string &filename) const {
    ofstream outfile(filename.c_str());

    if (!outfile.is_open())
      throw runtime_error(string("Unable to open " + filename));

    // output header info as comment
    outfile << "; twr lyr col diode mpd error" << endl;

    // PER XTAL LOOP
    for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++)
      for (DiodeNum diode; diode.isValid(); diode++) {
        const TwrNum twr ( xtalIdx.getTwr());
        const LyrNum lyr ( xtalIdx.getLyr());
        const ColNum col ( xtalIdx.getCol());
        if (m_mpd[diode][xtalIdx] == INVALID_MPD)
          continue;

        // per point along curve
        outfile << twr.val()
                << " " << lyr.val()
                << " " << col.val()
                << " " << diode.val()
                << " " << m_mpd[diode][xtalIdx]
                << " " << m_mpdErr[diode][xtalIdx]
                << endl;
      }
  }

  void CalMPD::readTXT(const string &filename) {
    unsigned short twr, lyr, col, diode;
    float mpd, sig;


    // open file
    ifstream infile(filename.c_str());

    if (!infile.is_open())
      throw runtime_error(string("Unable to open " + filename));

    // loop through each line in file
    string line;
    while (infile.good()) {
      getline(infile, line);
      if (infile.fail()) break; // bad get

      // check for comments
      if (line[0] == ';')
        continue;

      istringstream istrm(line);

      // get lyr, col (xtalId)
      istrm >> twr >> lyr >> col >> diode >> mpd >> sig;

      const XtalIdx xtalIdx(twr,
                            LyrNum(lyr),
                            col);

      m_mpd[DiodeNum(diode)][xtalIdx]    = mpd;
      m_mpdErr[DiodeNum(diode)][xtalIdx] = sig;
    }
  }

}; // namespace calibGenCAL
