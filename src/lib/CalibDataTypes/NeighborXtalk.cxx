// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/CalibDataTypes/NeighborXtalk.cxx,v 1.1 2007/03/27 18:50:50 fewtrell Exp $

/** @file
    @author fewtrell
*/

// LOCAL INCLUDES
#include "NeighborXtalk.h"
#include "../Util/SplineUtil.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalVec.h"

// EXTLIB INCLUDES
#include "TFile.h"
#include "TNtuple.h"
#include "TGraph.h"
#include "TCanvas.h"

// STD INCLUDES
#include <fstream>
#include <sstream>

using namespace std;
using namespace CalUtil;
using namespace SplineUtil;

const SplineUtil::Polyline *NeighborXtalk::getPts(CalUtil::DiodeIdx dest,
                                                  CalUtil::DiodeIdx source
                                                  ) const
{
  XtalkMap::const_iterator xtalkIt = m_xtalkMap.find(dest);


  if (xtalkIt == m_xtalkMap.end())
    return 0;

  ChannelSplineMap::const_iterator it =
    xtalkIt->second.find(source);

  if (it == xtalkIt->second.end())
    return 0;

  return &(it->second);
}

SplineUtil::Polyline *NeighborXtalk::getPts(CalUtil::DiodeIdx dest,
                                            CalUtil::DiodeIdx source)
{
  XtalkMap::iterator xtalkIt = m_xtalkMap.find(dest);


  if (xtalkIt == m_xtalkMap.end())
    return 0;

  ChannelSplineMap::iterator it =
    xtalkIt->second.find(source);

  if (it == m_xtalkMap[dest].end())
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
  outfile << "; destDiode srcDiode dac adc" << endl;

  outfile.precision(2);
  outfile.setf(ios_base::fixed);

  // loop through each destination channel
  for (DiodeIdx destIdx; destIdx.isValid(); destIdx++) {
    XtalkMap::const_iterator xtalkIt = m_xtalkMap.find(destIdx);
    if (xtalkIt == m_xtalkMap.end())
      continue;

    const ChannelSplineMap &destMap = xtalkIt->second;

    // loop through each registered source channel
    for (ChannelSplineMap::const_iterator it = destMap.begin();
         it != destMap.end();
         it++) {
      DiodeIdx srcIdx = it->first;
      const  Polyline &pLine = it->second;

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

void NeighborXtalk::setPoint(CalUtil::DiodeIdx dest,
                             CalUtil::DiodeIdx source,
                             float dac,
                             float adc) {
  // find all cross talk entries for given 'destination' channel
  XtalkMap::iterator xtalkIt = m_xtalkMap.find(dest);


  // create new destination map if needed
  if (xtalkIt == m_xtalkMap.end())
    xtalkIt = m_xtalkMap.insert(XtalkMap::value_type(dest, ChannelSplineMap())).first;

  // find curve for given source, destination pair.
  ChannelSplineMap::iterator chanIt =
    xtalkIt->second.find(source);

  // create new spline curve if needed
  if (chanIt == xtalkIt->second.end())
    chanIt = xtalkIt->second.insert(ChannelSplineMap::value_type(source, Polyline())).first;

  chanIt->second.push_back(Point2D(dac, adc));
}


void NeighborXtalk::writeTuples(const std::string &filename) const {
  // open new root file
  TFile rootFile(filename.c_str(),
                 "RECREATE",
                 "GLAST Cal Neighboring Crystal Cross-talk splines");
  
  // loop through each destination channel
  for (DiodeIdx destIdx; destIdx.isValid(); destIdx++) {
    XtalkMap::const_iterator xtalkIt = m_xtalkMap.find(destIdx);
    if (xtalkIt == m_xtalkMap.end())
      continue;

    const ChannelSplineMap &destMap = xtalkIt->second;

    // loop through each registered source channel
    for (ChannelSplineMap::const_iterator it = destMap.begin();
         it != destMap.end();
         it++) {
      DiodeIdx srcIdx = it->first;
      const  Polyline &pLine = it->second;
      
      string tuple_name("neighbor_xtalk" + destIdx.toStr() + "_from_" +
                        srcIdx.toStr());

      TNtuple *tuple = new TNtuple(tuple_name.c_str(),
                                   tuple_name.c_str(),
                                   "dac:adc");


      // build graph
      TCanvas *c = new TCanvas(tuple_name.c_str(), 
                               tuple_name.c_str(),
                               -1);
      TGraph *gr = new TGraph(pLine.size());
      c->SetGridx();
      c->SetGridy();
      gr->SetTitle(tuple_name.c_str());
      gr->SetMarkerStyle(kMultiply);

      float tuple_data[2];

      

      // loop through each point in polyline
      for (unsigned i = 0; i < pLine.size(); i++) {
        tuple_data[0] = pLine[i].first;
        tuple_data[1] = pLine[i].second;

        tuple->Fill(tuple_data);

        gr->SetPoint(i,
                     tuple_data[0],
                     tuple_data[1]);
      }

      gr->Draw("AP");
      c->Write();

    }
  }

  rootFile.Write();
  rootFile.Close();
}

void NeighborXtalk::pedSubtractADC() {
  /// loop through each dest channel
  for (XtalkMap::iterator it = m_xtalkMap.begin();
       it != m_xtalkMap.end();
       it++) {
    ChannelSplineMap &splMap = it->second;
                
    /// select each polyline for source channel
    for (ChannelSplineMap::iterator chanIt = splMap.begin();
         chanIt != splMap.end();
         chanIt++) {

      Polyline &spline = chanIt->second;

      float adcPed = spline[0].second;

      for (Polyline::iterator splIt = spline.begin();
           splIt != spline.end();
           splIt++)
        splIt->second -= adcPed;
    }
  }

}
