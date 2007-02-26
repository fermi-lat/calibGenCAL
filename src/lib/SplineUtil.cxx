// LOCAL INCLUDES
#include "SplineUtil.h"

// GLAST INCLUDES

// EXTLIB INCLUDES
#include "TSpline.h"

// STD INCLUDES
#include <vector>
#include <algorithm>

using namespace std;

namespace SplineUtil {

  TSpline3 *polyline2spline(const Polyline &pLine, 
                            const string &name) {
    unsigned size = pLine.size();

    if (size == 0) 
      return 0;

    double *x = new double[size];
    double *y = new double[size];

    for (unsigned i = 0;
         i < size;
         i++) {
      x[i] = pLine[i].first;
      y[i] = pLine[i].second;
    }

    TSpline3 *retVal = new TSpline3(name.c_str(),
                                    x,
                                    y,
                                    size);

    delete [] x;
    delete [] y;

    return retVal;
  }

  TSpline3 *vector2spline(const std::vector<float> &x,
                          const std::vector<float> &y,
                          const std::string &name) {
    unsigned size = min(x.size(), y.size());

    if (size == 0) 
      return 0;

    double *ax = new double[size];
    double *ay = new double[size];

    copy(x.begin(), 
         x.end(),
         ax);
    copy(y.begin(),
         y.end(),
         ay);

    TSpline3 *retVal = new TSpline3(name.c_str(),
                                    ax,
                                    ay,
                                    size);

    delete [] ax;
    delete [] ay;

    return retVal;
  }


};
