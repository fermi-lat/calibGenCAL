#ifndef HistVec_h
#define HistVec_h

// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Hists/HistVec.h,v 1.8 2008/05/09 21:51:38 fewtrell Exp $

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "src/lib/Util/ROOTUtil.h"
#include "HistIdx.h"

// GLAST INCLUDES
#include "CalUtil/CalVec.h"

// EXTLIB INCLUDES
#include "TDirectory.h"
#include "TKey.h"
#include "TIterator.h"
#include "TList.h"
#include "TObject.h"

// STD INCLUDES
#include <string>
#include <algorithm>
#include <functional>
#include <map>
#include <vector>
#include <stdexcept>
#include <sstream>

namespace calibGenCAL {

  /**  template class represents a CalUtil::CalVec or array 
       type collection of 1D ROOT histograms

       \note histograms are created as needed.
     
       \param IdxType intended to be index data type following conventions set in CalUtil::CalDefs
  */
  template <typename IdxType,
            typename HistType> 
  class HistVec {
  public:
    typedef IdxType index_type;
    typedef HistType histogram_type;

    /// \param histBasename all histograms will be created w/ name histBasename+idx.toStr()
    /// \param writeDir (if non-zero) all new histograms will be written out to this directory upon class destruction.
    /// \param readDir (if non-zero) any associated histograms will be read from this directory upon construction
    HistVec(const std::string &histBasename,
            TDirectory *writeDir=0,
            TDirectory *readDir=0,
            const size_t nBins=1000,
            const double loBinLimit=0,
            const double hiBinLimit=0
            ) :
      m_histBasename(histBasename),
      m_nBins(nBins),
      m_loBinLimit(loBinLimit),
      m_hiBinLimit(hiBinLimit),
      m_writeDir(writeDir)
    {
      if (readDir != 0)
        loadHists(*readDir);

      setDirectory(writeDir);
    }

    /// act like STL vector
    typedef HistType& reference;
    /// act like STL vector
    typedef const HistType& const_reference;

    /// return pointer to histogram for given index, return 0 if it doesn't exit
    HistType *getHist(const IdxType &idx) {
      return m_vec[idx];
    }

    /// retrieve histogram for given index, build it if it doesn't exist.
    HistType &produceHist(const IdxType &idx) {
      // create new hist if needed
      if (m_vec[idx] == 0)
        m_vec[idx] = genHist(idx);

      return *m_vec[idx];
    }

    /// set directory for all contained & future histograms 
    void setDirectory(TDirectory *const dir) {
      /// loop through all possible histograms & search for each one in current root dir
      for (IdxType idx;
           idx.isValid();
           idx++) {
        
        HistType *const hist_ptr = m_vec[idx];

        // only update existing directories
        if (hist_ptr != 0) {
          // construct histogram (relative) path
          const std::string subdir(genHistPath(idx));

          /// retrieve proper directory for hist (create if needed)
          TDirectory *const histdir = deliverROOTDir(dir, subdir);
          if (histdir == 0)
            throw std::runtime_error(std::string("Unable to create directory: ") +
                                     dir->GetPath() + "/" + subdir);

          
          hist_ptr->SetDirectory(histdir);
        }
      }

      m_writeDir = dir;
    }

    unsigned getMinEntries() const {
      unsigned retVal = ULONG_MAX;

      for (IdxType idx; idx.isValid(); idx++) {
        const unsigned nEntries = (unsigned)m_vec[idx]->GetEntries();

        // only count histograms that have been filled
        // (some histograms will never be filled if we are
        // not using all 16 towers)
        if (nEntries != 0)
          retVal = min(retVal, nEntries);
      }
    
      // case where there are no fills at all
      if (retVal == ULONG_MAX)
        return 0;

      return retVal;
    }

  private:
    /// load all associated histogram from current ROOT directory 
    void loadHists(TDirectory &readDir) {
      /// loop through all possible histograms & search for each one in current root dir
      for (IdxType idx;
           idx.isValid();
           idx++) {
        // retrieve histogram name
        const std::string histname(genHistName(idx));
        // retrieve histogram (relative) path
        const std::string subdir(genHistPath(idx));
                
        TDirectory *const histdir(readDir.GetDirectory(subdir.c_str()));
        //  move on if that directory has not been created.
        if (histdir == 0)
          continue;
        
        /// try to retrieve obj from dir
        HistType *const hist_ptr = retrieveROOTObj<HistType>(*histdir, 
                                                             histname.c_str());
        /// skip if histogram doesn't exit
        if (hist_ptr == 0)
          continue;

        m_vec[idx] = hist_ptr;
      }
    }

    std::string genHistName(const IdxType &idx) const {
      return m_histBasename + "_" + idx.toStr();
    }


    /// generate histogram in  directory
    HistType *genHist(const IdxType &idx) {
      if (m_writeDir == 0)
        throw std::runtime_error("HistVec::genHist() : Write directory not set for HistVec class");

      const std::string histname(genHistName(idx));
      const std::string subdir(genHistPath(idx));
      
      std::ostringstream tmp;
      tmp <<  m_writeDir->GetPath() << subdir << "/";
      const std::string fullpath(tmp.str());

      HistType *newHist = new HistType(histname.c_str(),
                                       histname.c_str(),
                                       m_nBins,
                                       m_loBinLimit,
                                       m_hiBinLimit);
      if (newHist == 0) 
        throw std::runtime_error(std::string("Unable to create histogram: ") +
                                 histname);
      
      /// retrieve proper directory for hist (create if needed)
      TDirectory *const histdir = deliverROOTDir(m_writeDir, subdir);
      if (histdir == 0)
        throw std::runtime_error(std::string("Unable to create directory: ") +
                                 fullpath);

      newHist->SetDirectory(histdir);

      return newHist;
    }
  
    /// generate appropriate subdirectory for histogram
    std::string genHistPath(const IdxType &idx) const {
      ostringstream tmp;
      tmp << m_histBasename << "/"
          << toPath(idx);
      return tmp.str();
    }

    typedef CalUtil::CalVec<IdxType,HistType*> VecType;
    VecType m_vec;
 
    /// shared histogram name prefix
    const std::string m_histBasename;
    /// option for creating new histogram
    size_t m_nBins;
    /// option for creating new histogram
    const double m_loBinLimit;
    /// option for creating new histogram
    const double m_hiBinLimit;

    /// all new histograms (& modified) written to this directory
    TDirectory * m_writeDir;


  };

}; // namespace calibGenCAL
#endif
