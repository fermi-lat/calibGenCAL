/** @file 

    @author Zachary Fewtrell

    \brief support HistIdx.h
*/

// LOCAL INCLUDES
#include "HistIdx.h"

// GLAST INCLUDES

// EXTLIB INCLUDES

// STD INCLUDES
#include <vector>
#include <stdexcept>

using namespace std;
using namespace CalUtil;

namespace calibGenCAL {

  /// construct from string repr
  MeanDacZId::MeanDacZId(const string &name) {
    /// break name up into component parts
    const vector<string> parts(CGCUtil::tokenize_str(name, string("_Z")));

    if (parts.size() != N_FIELDS)
      throw runtime_error("Invalid MeanDACZId string repr: " + name);

    // extract each component
    const XtalIdx xtalIdx(parts[0]);

    const DiodeNum diode(parts[1]);

    istringstream z_strm(parts[2]);
    unsigned short z;
    z_strm >> z;

    /// calculate index
    m_data = calc(z, xtalIdx, diode);
  }

  /// construct from string repr
  MeanDACId::MeanDACId(const string &name) {
    /// break name up into component parts
    const vector<string> parts(CGCUtil::tokenize_str(name, string("_")));

    if (parts.size() != N_FIELDS)
      throw runtime_error("Invalid MeanDACId string repr: " + name);

    const XtalIdx xtalIdx(parts[0]);

    const DiodeNum diode(parts[1]);
    
    m_data = calc(xtalIdx, diode);
    
  }

  /// construct from string repr
  ZDiodeId::ZDiodeId(const string &name) {
    /// break name up into component parts
    const vector<string> parts(CGCUtil::tokenize_str(name, string("_Z")));

    if (parts.size() != N_FIELDS)
      throw runtime_error("Invalid ZDiodeId string repr: " + name);

    // extract each component
    const DiodeNum diode(parts[0]);

    istringstream z_strm(parts[1]);
    unsigned short z;
    z_strm >> z;

    /// calculate index
    m_data = calc(z, diode);
  }
};
