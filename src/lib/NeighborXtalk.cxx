// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/NeighborXtalk.cxx,v 1.1 2007/02/26 23:20:30 fewtrell Exp $

/** @file
    @author fewtrell
*/

// LOCAL INCLUDES
#include "NeighborXtalk.h"
#include "SplineUtil.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalVec.h"

// EXTLIB INCLUDES

// STD INCLUDES
#include <fstream>
#include <sstream>

using namespace std;
using namespace CalUtil;
using namespace SplineUtil;

const SplineUtil::Polyline *NeighborXtalk::getPts(CalUtil::RngIdx dest,
                                                  CalUtil::RngIdx source
                                                  ) const
{
  ChannelSplineMap destMap = m_splinePts[dest];

  ChannelSplineMap::const_iterator it = 
    destMap.find(source);
      
  if (it == m_splinePts[dest].end())
    return 0;

  return &(it->second);
}
  
SplineUtil::Polyline *NeighborXtalk::getPts(CalUtil::RngIdx dest,
                                            CalUtil::RngIdx source) 
{
  ChannelSplineMap destMap = m_splinePts[dest];

  ChannelSplineMap::iterator it = 
    destMap.find(source);
      
  if (it == m_splinePts[dest].end())
    return 0;

  return &(it->second);
}

void NeighborXtalk::writeTXT(const std::string &filename) const {
  ofstream outfile(filename.c_str());

  if (!outfile.is_open()) {
    ostringstream tmp;
    tmp << __FILE__ << ":" << __LINE__ << " "
        << "ERROR! unable to open txtFile='" << filename << "'";
    throw runtime_error(tmp.str());
  }

  // output header info as comment
  outfile << "; destRngIdx srcRngIdx dac adc" << endl;

  outfile.precision(2);
  outfile.setf(ios_base::fixed);

  // loop through each destination channel
  for (RngIdx destIdx; destIdx.isValid(); destIdx++) {
    const ChannelSplineMap &destMap = m_splinePts[destIdx];
        
    // loop through each registered source channel
    for (ChannelSplineMap::const_iterator it = destMap.begin();
         it != destMap.end();
         it++) {
                
      RngIdx srcIdx = it->first;
      const Polyline &pLine = it->second;
                
      // loop through each point in polyline
      for (Polyline::const_iterator it = pLine.begin();
           it != pLine.end();
           it++) {
        const float &dac = it->first;
        const float &adc = it->second;

        outfile << destIdx.val() << " "
                << srcIdx.val() << " "
                << dac << " "
                << adc << " "
                << endl;
      }
    }
  }
}

