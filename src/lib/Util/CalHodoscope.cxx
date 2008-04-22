// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Util/CalHodoscope.cxx,v 1.6 2008/01/22 19:41:00 fewtrell Exp $

/** @file
    @author fewtrell
*/

// LOCAL INCLUDES
#include "CalHodoscope.h"
#include "../Util/stl_util.h"

// GLAST INCLUDES
#include "digiRootData/CalDigi.h"
#include "CalUtil/SimpleCalCalib/CalPed.h"
#include "CalUtil/SimpleCalCalib/CIDAC2ADC.h"

// EXTLIB INCLUDES

// STD INCLUDES
#include <functional>
#include <ostream>

namespace calibGenCAL {

  using namespace CalUtil;

  void CalHodoscope::clear() {
    // zero out all vectors
    fill(adc_ped.begin(), adc_ped.end(), CIDAC2ADC::INVALID_ADC());
    fill(dac.begin(), dac.end(), CIDAC2ADC::INVALID_ADC());
    fill_zero(perLyr);
    fill(bestRng.begin(), bestRng.end(), LEX8);

    // empty hit lists
    hitList.clear();

    // zero out primitives
    count      = 0;
    nLyrs      = 0;
    maxPerLyr  = 0;
  }

  void CalHodoscope::addHit(const CalDigi &calDigi) {
    //-- XtalId --//
    idents::CalXtalId id(calDigi.getPackedId());  // get interaction information

    // skip hits not for current tower.
    LyrNum lyr(id.getLayer());

    XtalIdx xtalIdx(id);

    // load up all adc values for each xtal diode
    // also ped subtraced adc values.
    for (XtalDiode xDiode; xDiode.isValid(); xDiode++) {
      DiodeNum diode  = xDiode.getDiode();
      RngNum   rng    = diode.getX8Rng();   // we are only interested in x8 range adc vals for muon calib
      FaceNum  face   = xDiode.getFace();
      RngIdx    rngIdx  (xtalIdx,
                         face,
                         rng);
      DiodeIdx  diodeIdx(xtalIdx,
                         xDiode);

      float adc = calDigi.getAdcSelectedRange(rng.val(), (CalXtalId::XtalFace)face.val());   // raw adc
      if (adc < 0)
        continue;

      float ped = m_peds.getPed(rngIdx);
      adc_ped[diodeIdx] = adc - ped;

      // fill best range info
      FaceIdx faceIdx(xtalIdx,
                      face);

      bestRng[faceIdx]  = RngNum(calDigi.getRange(0, (CalXtalId::XtalFace)face.val()));

      dac[diodeIdx]     = m_cidac2adc.adc2dac(rngIdx, adc_ped[diodeIdx]);
      // double check that all dac signals are > 0
      // otherwise we have noise hit or diode deposit
      if (dac[diodeIdx] <= 0)
        return;
    }

    // DO WE HAVE A HIT? (sum up both ends LEX8 i.e LRG_DIODE)
    float &adcPedPL = adc_ped[DiodeIdx(xtalIdx, POS_FACE, LRG_DIODE)];
    float &adcPedNL = adc_ped[DiodeIdx(xtalIdx, NEG_FACE, LRG_DIODE)];

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

    count++;  // increment total # hits

    perLyr[lyr]++;
    hitList.push_back(xtalIdx);
  }

  void CalHodoscope::summarizeEvent() {
    /// currently all values are updated in the 'addHit()' method
    /// so no code is needed in this method
  }

}; // namespace calibGenCAL
