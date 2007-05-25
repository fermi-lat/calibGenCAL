#ifndef HistVec_h
#define HistVec_h

// LOCAL INCLUDES
#include "../Util/CGCUtil.h"


// GLAST INCLUDES
#include "CalUtil/CalVec.h"

// EXTLIB INCLUDES
#include "TDirectory.h"

// STD INCLUDES
#include <string>
#include <algorithm>
#include <functional>

namespace calibGenCAL {
  /**  template class represents a CalUtil::CalVec or array 
       type collection of 1D ROOT histograms
     
       \param IdxType intended to be CalUtil::CalDefs index data type.
  */


  template <typename IdxType,
            typename HistType> 
  class HistVec {
  private:
    typedef CalUtil::CalVec<IdxType,HistType*> VecType;
  public:
    /// \param histBasename all histograms will be created w/ name histBasename+idx.toStr()
    /// \param rootFile if ptr is non-zero, then all matching histograms will be loaded from given file.

    HistVec(const std::string &histBasename,
            const size_t nBins=1000,
            const double loBinLimit=0,
            const double hiBinLimit=0,
            const TDirectory *rootFile=0
            ) :
      m_vec(IdxType::N_VALS),
      histBasename(histBasename),
      nBins(nBins),
      loBinLimit(loBinLimit),
      hiBinLimit(hiBinLimit)
    {
      // read in histlist & initial histograms if rootFile is set
      if (rootFile != 0)
        loadHists(*rootFile);
    }

    /// act like STL vector
    typedef HistType& reference;
    /// act like STL vector
    typedef const HistType& const_reference;

    /// retrieve histogram for given index, build it if it doesn't exist.
    HistType &checkNGet(const IdxType &idx) {
      // create new hist if needed
      if (m_vec[idx] == 0)
        m_vec[idx] = genHist(idx);

      return *m_vec[idx];
    }

    /// set ROOT directory for each histogram
    void setHistDir(TDirectory *const dir) {
      for (typename VecType::iterator it(m_vec.begin());
             it != m_vec.end();
             it++) {
                 if (*it != 0)
                     (*it)->SetDirectory(dir);
             }
    }

    

  private:
    /// load all associated histogram from histogram file
    void loadHists(const TDirectory &rootFile) {
      for (IdxType idx; idx.isValid(); idx++) {
        const string histname(genHistName(idx));

        HistType * const hp(CGCUtil::retrieveROOTObj<HistType>(rootFile, histname));

        if (hp == 0)
          continue;

        m_vec[idx] = hp;
      }
    }
  

    std::string genHistName(const IdxType &idx) {
      return histBasename + "_" + idx.toStr();
    }


    HistType *genHist(const IdxType &idx) {
      const std::string name(genHistName(idx));

      return new HistType(name.c_str(),
                          name.c_str(),
                          nBins,
                          loBinLimit,
                          hiBinLimit);
    }
  
    VecType m_vec;
  
    const std::string histBasename;
    const size_t nBins;
    const double loBinLimit;
    const double hiBinLimit;
  };

}; // namespace calibGenCAL
#endif
