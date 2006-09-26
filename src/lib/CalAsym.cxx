// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/CalAsym.cxx,v 1.2 2006/09/19 18:44:15 fewtrell Exp $
/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "CalAsym.h"

// GLAST INCLUDES

// EXTLIB INCLUDES


// STD INCLUDES
#include <fstream>
#include <sstream>

using namespace std;
using namespace CalUtil;

CalAsym::CalAsym() :
  m_asym(AsymType::N_VALS),
  m_asymErr(AsymType::N_VALS)
{
}

void CalAsym::writeTXT(const string &filename) const{
  ofstream outfile(filename.c_str());
  
  if (!outfile.is_open())
    throw runtime_error(string("Unable to open " + filename));

  // PER XTAL LOOP
  for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
    TwrNum twr = xtalIdx.getTwr();
    LyrNum lyr = xtalIdx.getLyr();
    ColNum col = xtalIdx.getCol();
    for (AsymType asymType; asymType.isValid(); asymType++) {
      // per point along curve
      for (unsigned short i = 0; i < N_ASYM_PTS; i++) {
        // skip empty channels
        if (m_asym[asymType][xtalIdx].size() != N_ASYM_PTS)
          continue;
        
        outfile << twr
                << " " << lyr 
                << " " << col
                << " " << asymType.getDiode(POS_FACE)
                << " " << asymType.getDiode(NEG_FACE)
                << " " << m_asym[asymType][xtalIdx][i]
                << " " << m_asymErr[asymType][xtalIdx][i]
                << endl;
      }
    }
  }
}

void CalAsym::readTXT(const string &filename){
  unsigned short twr, lyr, col, pdiode, ndiode;
  float asym, sig;

  // open file
  ifstream infile(filename.c_str());
  if (!infile.is_open())
    throw runtime_error(string("Unable to open " + filename));

  // loop through each line in file
  while (infile.good()) {
    // get lyr, col (xtalId)
    infile >> twr >> lyr >> col >> pdiode >> ndiode >> asym >> sig;
    
    if (infile.fail()) break; // bad get()

    XtalIdx xtalIdx(twr, lyr,col);
    AsymType asymType(pdiode, ndiode);

    m_asym[asymType][xtalIdx].push_back(asym);
    m_asymErr[asymType][xtalIdx].push_back(sig);
  }
}


void CalAsym::buildSplines(){
  m_a2pSplines.resize(DiodeNum::N_VALS);
  m_p2aSplines.resize(DiodeNum::N_VALS);

  for (DiodeNum diode; diode.isValid(); diode++) {
    m_a2pSplines[diode].fill(0);
    m_p2aSplines[diode].fill(0);
  }

  // create position (Y-axis) array
  // linearly extrapolate for 1st and last points (+2 points)
  double pos[N_ASYM_PTS+2];
  for (unsigned short i = 0; i < N_ASYM_PTS+2; i++) 
    pos[i] = i + 0.5; // (center of the column)
  double asym[N_ASYM_PTS+2];

  // PER XTAL LOOP
  for (DiodeNum diode; diode.isValid(); diode++) {
    AsymType asymType(diode,diode);
    for (XtalIdx xtalIdx; xtalIdx.isValid(); xtalIdx++) {
      // skip empty channels
      if (m_asym[asymType][xtalIdx].size() != N_ASYM_PTS)
        continue;
    
      // copy asym vector into middle of array
      vector<float> &asymVec = m_asym[asymType][xtalIdx];
      copy(asymVec.begin(), asymVec.end(), asym+1);

      // extrapolate 1st & last points
      asym[0] = 2*asym[1] - asym[2];
      asym[N_ASYM_PTS+1] = 2*asym[N_ASYM_PTS]-asym[N_ASYM_PTS-1];

      { 
        //generate splinename
        ostringstream name;
        name << "asym2pos_" << xtalIdx.val() << "_" << diode.val();

        // create spline object
        TSpline3 *mySpline= new TSpline3(name.str().c_str(), 
                                         asym, pos, N_ASYM_PTS+2);
        mySpline->SetName(name.str().c_str());

        
        
        m_a2pSplines[diode][xtalIdx] = mySpline;
      }

      // create inverse spline object
      {
        //generate splinename
        ostringstream name;
        name << "pos2asym_" << xtalIdx.val() << "_" << diode.val();

        // create spline object
        TSpline3 *mySpline= new TSpline3(name.str().c_str(), 
                                         pos, asym, N_ASYM_PTS+2);
        mySpline->SetName(name.str().c_str());

        //         cout << mySpline->GetName() << " ";
        //         for (unsigned short i = 0; i < N_ASYM_PTS+2; i++)
        //           cout << pos[i] << " " << asym[i] << " ";
        //         cout << endl;

        m_p2aSplines[diode][xtalIdx] = mySpline;
      }
    }
  }
}
