// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/NeighborXtalk.h,v 1.9 2007/01/29 19:28:00 fewtrell Exp $

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
  
}

void NeighborXtalk::readTXT(const std::string &filename) {
  
}
