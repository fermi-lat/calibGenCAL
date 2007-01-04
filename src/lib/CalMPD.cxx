// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/CalMPD.cxx,v 1.2 2006/09/26 18:57:24 fewtrell Exp $

/** @file
    @author Zachary Fewtrell
 */

// LOCAL INCLUDES
#include "CalMPD.h"

// GLAST INCLUDES

// EXTLIB INCLUDES

// STD INCLUDES
#include <fstream>

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

  // PER XTAL LOOP
  for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++)
    for (DiodeNum diode; diode.isValid(); diode++) {
      TwrNum twr = xtalIdx.getTwr();
      LyrNum lyr = xtalIdx.getLyr();
      ColNum col = xtalIdx.getCol();
      if (m_mpd[diode][xtalIdx] == INVALID_MPD)
        continue;

      // per point along curve
      outfile << twr
              << " " << lyr
              << " " << col
              << " " << diode
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
  while (infile.good()) {
    // get lyr, col (xtalId)
    infile >> twr >> lyr >> col >> diode >> mpd >> sig;

    if (infile.fail()) break; // bad get()

    XtalIdx xtalIdx(twr,
                    lyr,
                    col);

    m_mpd[diode][xtalIdx]    = mpd;
    m_mpdErr[diode][xtalIdx] = sig;
  }
}

