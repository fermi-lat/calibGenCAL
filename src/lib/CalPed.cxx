// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/CalPed.cxx,v 1.4 2006/06/27 15:36:25 fewtrell Exp $
/** @file
    @author Zachary Fewtrell
 */

// LOCAL INCLUDES
#include "CalPed.h"
#include "CGCUtil.h"

// GLAST INCLUDES

// EXTLIB INCLUDES
#include "TF1.h"
#include "TKey.h"

// STD INCLUDES
#include <sstream>
#include <ostream>
#include <fstream>

using namespace CGCUtil;

CalPed::CalPed() :
  m_peds(RngIdx::N_VALS, INVALID_PED),
  m_pedSig(RngIdx::N_VALS, 0)
{
}


void CalPed::writeTXT(const string &filename) const {
  ofstream outfile(filename.c_str());
  if (!outfile.is_open())
    throw runtime_error(string("Unable to open " + filename));

  for (RngIdx rngIdx; rngIdx.isValid(); rngIdx++)
    if (m_peds[rngIdx] != INVALID_PED)
      outfile << rngIdx.getTwr().val()
              << " " << rngIdx.getLyr().val()
              << " " << rngIdx.getCol().val()
              << " " << rngIdx.getFace().val()
              << " " << rngIdx.getRng().val()
              << " " << m_peds[rngIdx]
              << " " << m_pedSig[rngIdx]
              << endl;
}

  
void CalPed::readTXT(const string &filename) {
  ifstream infile(filename.c_str());
  if (!infile.is_open())
    throw runtime_error(string("Unable to open " + filename));

  while(infile.good()) {
    float ped, sig;
    unsigned short twr;
    unsigned short lyr;
    unsigned short col;
    unsigned short face;
    unsigned short rng;
    
    infile >> twr
           >> lyr
           >> col
           >> face
           >> rng
           >> ped
           >> sig;
    // quit once we can't read any more values
    if (infile.fail()) break; 

    RngIdx rngIdx(twr, lyr, col, face, rng);
    m_peds[rngIdx]= ped;
    m_pedSig[rngIdx]= sig;
  }
}
