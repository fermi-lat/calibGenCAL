// $Header: //

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "ADC2NRG.h"
#include "CalMPD.h"
#include "CIDAC2ADC.h"
#include "CalAsym.h"
#include "MPDHists.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"

// EXTLIB INCLUDES

// STD INCLUDES
#include <string>
#include <fstream>

using namespace std;
using namespace CalUtil;

void ADC2NRG::writeTXT(const string &filename,
                       const CalAsym &asym,
                       const CIDAC2ADC &dac2adc,
                       const CalMPD &calMPD) {
  ofstream outfile(filename.c_str());

  if (!outfile.is_open())
    throw runtime_error(string("Unable to open " + filename));

  for (RngIdx rngIdx; rngIdx.isValid(); rngIdx++) {
    TwrNum   twr     = rngIdx.getTwr();
    LyrNum   lyr     = rngIdx.getLyr();
    ColNum   col     = rngIdx.getCol();
    FaceNum  face    = rngIdx.getFace();
    RngNum   rng     = rngIdx.getRng();
    DiodeNum diode   = rng.getDiode();
    XtalIdx  xtalIdx = rngIdx.getXtalIdx();
    float    mpd     = calMPD.getMPD(xtalIdx, diode);

    if (mpd == CalMPD::INVALID_MPD)
      continue;

    // get asymmetry measured at ctr of xtal (gives us relative gain between xtal faces)

    // 0.25 would normally be 0.5, but it is applied equally to both sides
    // so we split it in half.
    float asym_ctr = 0.25*asym.pos2asym(xtalIdx,
                                        diode,
                                        6.0);   // is center of xtal, coords range from 0-12

    asym_ctr = exp(asym_ctr);

    float dac = MPDHists::MUON_ENERGY/mpd;


    dac = (face == POS_FACE) ?
      dac * asym_ctr :
      dac / asym_ctr;



    float adc     = dac2adc.dac2adc(rngIdx, dac);

    float adc2nrg = MPDHists::MUON_ENERGY/adc;

    outfile << twr
            << " " << lyr
            << " " << col
            << " " << face.val()
            << " " << rng.val()
            << " " << adc2nrg
            << " " << 0  // error calc
            << endl;
  }
}

