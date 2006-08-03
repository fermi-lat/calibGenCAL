// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/TwrHodoscope.cxx,v 1.2 2006/06/22 21:50:23 fewtrell Exp $
/** @file
    @author fewtrell
 */

// LOCAL INCLUDES
#include "TwrHodoscope.h"
#include "CGCUtil.h"

// GLAST INCLUDES

// EXTLIB INCLUDES

// STD INCLUDES
#include <sstream>
#include <functional>

using namespace CGCUtil;
using namespace CalUtil;

void TwrHodoscope::clear() {
  // zero out all vectors
  fill_zero(adc_ped);
  fill_zero(dac);
  fill_zero(perLyrX);
  fill_zero(perLyrY);
  fill_zero(perColX);
  fill_zero(perColY);

  // empty hit lists
  hitListX.clear();
  hitListY.clear();

  // zero out primitives
  count      = 0;
  nLyrsX     = 0;
  nLyrsY     = 0;
  nColsX     = 0;
  nColsY     = 0;
  maxPerLyr  = 0;
  maxPerLyrX = 0;
  maxPerLyrY = 0;
  firstColX  = 0;
  firstColY  = 0;

  goodXTrack = false;
  goodYTrack = false;
}

void TwrHodoscope::addHit(const CalDigi &calDigi) {
  //-- XtalId --//
  idents::CalXtalId id(calDigi.getPackedId()); // get interaction information
  // skip hits not for current tower.
  ColNum col = id.getColumn();
  LyrNum lyr = id.getLayer();

  XtalIdx xtalIdx(id);

  // check that we are in 4-range readout mode
  int nRO = calDigi.getNumReadouts();
  if (nRO != 4)
    return;

  // load up all adc values for each xtal diode
  // also ped subtraced adc values.
  for (XtalDiode xDiode; xDiode.isValid(); xDiode++) {
    DiodeNum  diode  = xDiode.getDiode();
    RngNum    rng    = diode.getX8Rng();  // we are only interested in x8 range adc vals for muon calib
    FaceNum   face   = xDiode.getFace();
    RngIdx    rngIdx  (xtalIdx, face, rng);
    tDiodeIdx diodeIdx(xtalIdx.getTXtalIdx(), xDiode);

    float adc = calDigi.getAdcSelectedRange(rng.val(), (CalXtalId::XtalFace)face.val()); // raw adc
    if (adc < 0) {
      m_ostrm << "Couldn't get adc val for face=" << face.val()
              << " rng=" << rng.val() << endl;
      return;
    }

    float ped = m_peds.getPed(rngIdx);
    adc_ped[diodeIdx] = adc - ped;

    dac[diodeIdx] = m_cidac2adc.adc2dac(rngIdx, adc_ped[diodeIdx]);
    // double check that all dac signals are > 0
    if (dac[diodeIdx] <= 0) 
      return;
  }
    
  // DO WE HAVE A HIT? (sum up both ends LEX8 i.e LRG_DIODE)
  float &adcPedPL = adc_ped[tDiodeIdx(xtalIdx.getTXtalIdx(), POS_FACE, LRG_DIODE)];
  float &adcPedNL = adc_ped[tDiodeIdx(xtalIdx.getTXtalIdx(), NEG_FACE, LRG_DIODE)];

  float adcSigPL = m_peds.getPedSig(RngIdx(xtalIdx, POS_FACE, LEX8));
  float adcSigNL = m_peds.getPedSig(RngIdx(xtalIdx, NEG_FACE, LEX8));

  //-- HIT CUT --//
  // - total deposited energy should be > threshold 
  // - also, each face > 3 sigma pretty much guarantees that it is xtal deposit & not
  //   direct diode deposit.
  if (adcPedPL + adcPedNL < hitThresh ||
      adcPedPL < 3*adcSigPL ||
      adcPedNL < 3*adcSigNL) 
    return;

  count++; // increment total # hits
  
  // used to determine if xtal is x or y layer
  if (lyr.getDir() == X_DIR) { // X layer
    GCRCNum gcrc = lyr.getGCRC();
    perLyrX[gcrc]++;
    perColX[col]++;
    hitListX.push_back(xtalIdx);
  } else { // y layer
    GCRCNum gcrc = lyr.getGCRC();
    perLyrY[gcrc]++;
    perColY[col]++;
    hitListY.push_back(xtalIdx);
  }
}


void TwrHodoscope::summarizeEvent() {
  // we're done if there were no hits
  if (count) {
    // POST-PROCESS: after we have registered all hits, summarize them all
    // find highest hits per layer
    maxPerLyrX = *(max_element(perLyrX.begin(),
                               perLyrX.end()));
    maxPerLyrY = *(max_element(perLyrY.begin(),
                               perLyrY.end()));
    maxPerLyr = max(maxPerLyrX, maxPerLyrY);
    
    
    // count all layers w/ count > 0
    nLyrsX = count_if(perLyrX.begin(), perLyrX.end(),
                         bind2nd(greater<unsigned short>(), 0));
    nLyrsY = count_if(perLyrY.begin(), perLyrY.end(),
                         bind2nd(greater<unsigned short>(), 0));
    
    // count all cols w/ count > 0
    nColsX = count_if(perColX.begin(), perColX.end(),
                      bind2nd(greater<unsigned short>(), 0));
    nColsY = count_if(perColY.begin(), perColY.end(),
                      bind2nd(greater<unsigned short>(), 0));

    // find 1st col hit in each direction (will be only col hit if evt is good)
    // find_if returns iterator to 1st true element... 
    // ...subtract begin() iterator to get distance or index
    firstColX = find_if(perColX.begin(), perColX.end(), 
                        bind2nd(greater<unsigned short>(), 0)) - perColX.begin();
    firstColY = find_if(perColY.begin(), perColY.end(), 
                        bind2nd(greater<unsigned short>(), 0)) - perColY.begin();

    // EVENT SELECTION:
    // don't want ANY layer w/ >= 3 hits
    if (maxPerLyr > 2) return;
    
    // X has Vertical Connect-4 && >0 Y hits
    if (nLyrsX == 4 &&
        nColsX == 1 &&
        nLyrsY > 0)
      goodXTrack = true;

    // Y has Vertical Connect-4 && >0 X hits
    if (nLyrsY == 4 &&
        nColsY == 1 &&
        nLyrsX > 0)
      goodYTrack = true;
  }
}

