// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Util/FaceSignalArray.cxx,v 1.6 2008/01/22 19:41:00 fewtrell Exp $

/** @file
    @author fewtrell
*/

// LOCAL INCLUDES
#include "CalSignalArray.h"
#include "src/lib/Util/CGCUtil.h"

// GLAST INCLUDES
#include "digiRootData/CalDigi.h"
#include "digiRootData/DigiEvent.h"
#include "CalUtil/SimpleCalCalib/CalPed.h"
#include "CalUtil/SimpleCalCalib/ADC2NRG.h"

// EXTLIB INCLUDES

// STD INCLUDES

namespace calibGenCAL {

  using namespace CalUtil;

  void CalSignalArray::addHit(const CalDigi &calDigi) {
    // get interaction information
    const idents::CalXtalId id(calDigi.getPackedId());
    const XtalIdx xtalIdx(id);

    for (FaceNum face; face.isValid(); face++) {
      /// get best range
      const RngNum rng(calDigi.getRange(0, (CalXtalId::XtalFace)face.val()));
      const unsigned short adc(calDigi.getAdc(0, (CalXtalId::XtalFace)face.val()));
      const RngIdx  rngIdx(xtalIdx, face, rng);

      const float ped(m_peds.getPed(rngIdx));
      const float adcPed = (float)adc - ped;
      
      const float adc2nrg(m_adc2nrg.getADC2NRG(rngIdx));
      
      /// mev = adc*(mev/adc)
      const FaceIdx faceIdx(xtalIdx, face);
      m_faceSignal[faceIdx] = adcPed*adc2nrg;
      m_adcPed[faceIdx] = adcPed;
      m_adcRng[faceIdx] = rng;
    } // for (face)
  }

  void CalSignalArray::fillArray(const DigiEvent &digiEvent) {
    //-- loop through each 'hit' in one event --//
    const TClonesArray *calDigiCol = digiEvent.getCalDigiCol();
    if (!calDigiCol) {
      LogStrm::get() << "no calDigiCol found." << endl;
      return;
    }

    TIter calDigiIter(calDigiCol);

    const CalDigi      *pCalDigi = 0;

    while ((pCalDigi = dynamic_cast<CalDigi *>(calDigiIter.Next()))) {
      const     CalDigi &calDigi = *pCalDigi;    // use reference to avoid -> syntax

      addHit(calDigi);
    }

  }

}; // namespace calibGenCAL
