// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/RoughPed.cxx,v 1.4 2006/06/27 15:36:25 fewtrell Exp $
/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "RoughPed.h"

// GLAST INCLUDES

// EXTLIB INCLUDES

// STD INCLUDES
#include <sstream>
#include <ostream>
#include <fstream>

RoughPed::RoughPed() :
  m_peds(FaceIdx::N_VALS, INVALID_PED),
  m_pedSig(FaceIdx::N_VALS, 0)
{
}

void RoughPed::writeTXT(const string &filename) const {
  ofstream outfile(filename.c_str());
  if (!outfile.is_open())
    throw runtime_error(string("Unable to open " + filename));

  for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++)
    if (m_peds[faceIdx] != INVALID_PED)
      outfile << faceIdx.getTwr().val()
              << " " << faceIdx.getLyr().val()
              << " " << faceIdx.getCol().val()
              << " " << faceIdx.getFace().val()
              << " " << m_peds[faceIdx]
              << " " << m_pedSig[faceIdx]
              << endl;
}

  
void RoughPed::readTXT(const string &filename) {
  ifstream infile(filename.c_str());
  if (!infile.is_open())
    throw runtime_error(string("Unable to open " + filename));

  while(infile.good()) {
    float ped, sig;
    unsigned short twr;
    unsigned short lyr;
    unsigned short col;
    unsigned short face;
    
    infile >> twr
           >> lyr
           >> col
           >> face
           >> ped
           >> sig;
    // quit once we can't read any more values
    if (infile.fail()) break; 

    FaceIdx faceIdx(twr, lyr, col, face);
    m_peds[faceIdx]= ped;
    m_pedSig[faceIdx]= sig;
  }
}

