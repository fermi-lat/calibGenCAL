#ifndef HistMap_h
#define HistMap_h

// $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Hists/GCRHists.h,v 1.5 2007/05/25 21:06:47 fewtrell Exp $

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "../Util/CGCUtil.h"

// GLAST INCLUDES

// EXTLIB INCLUDES
#include "TDirectory.h"

// STD INCLUDES
#include <map>
#include <string>
#include <algorithm>
#include <functional>
#include <stdexcept>
#include <sstream>
#include <vector>

/** @file template class represents a collection  of 1D ROOT histograms 
    mapped to some index class
    
    \note class is intended for use w/ indexes that are spare arrays.  
    fully populated arrays should use HistVec

    \note this class also creates a TTree tuple which is intended to be saved w/
    the histograms & which can be used as a list of the sparesely populated histograms
    in retrieval

    \note IdxType needs a unsigned IdxType::val() method like the CalUtil::CalDefs idx classes.
    \note IdxType must support a constructor from 'unsigned'

*/

namespace calibGenCAL {

  template <typename IdxType,
            typename HistType> 
  class HistMap {
  private:
    typedef std::map<IdxType,HistType*> MapType;
    typedef std::pair<IdxType,HistType*> ValType;

  public:
    /// \param m_histBasename all histograms will be created _as_ needed w/ name m_histBasename+idx.toStr()
    /// \param writeDir (if non-zero) all histograms will be written out to this directory opun class destruction.
    /// \param readDir (if non-zero) any associated histograms will be read from this directory upon construction 
    HistMap(const std::string &histBasename,
            TDirectory *const writeDir=0,
            TDirectory *const readDir=0,
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
      /// load data from file
      if (readDir != 0)
        loadHists(*readDir);

      setDirectory(writeDir);
    }


    /// retrieve histogram for given index, build it if it doesn't exist.
    HistType &produceHist(const IdxType &idx) {
      typename MapType::iterator it(m_map.lower_bound(idx));

      /// this means that idx has not yet been inserted into the map
      if (it == m_map.end() || it->first != idx)       
        it = m_map.insert(it,ValType(idx, genHist(idx)));
      return *(it->second);
    }

    /// set directory for all contained histograms
    void setDirectory(TDirectory *const dir) {
      for (MapType::iterator it(m_map.begin());
           it != m_map.end();
           it++) {
        IdxType idx(it->first);
        HistType *const hist_ptr(it->second);

        // construct histogram (relative) path
        const std::string subdir(genHistPath(idx));

        /// retrieve proper directory for hist (create if needed)
        TDirectory *const histdir = deliverROOTDir(dir, subdir);
        if (histdir == 0)
          throw std::runtime_error(std::string("Unable to create directory: ") +
                                   dir->GetPath() + "/" + subdir);

        hist_ptr->SetDirectory(histdir);
        
      }

      m_writeDir = dir;
    }

    typedef typename MapType::iterator iterator;
    typedef typename MapType::const_iterator const_iterator;
    
    iterator begin() {return m_map.begin();}
    const_iterator begin() const {return m_map.begin();}

    iterator end() {return m_map.end();}
    const_iterator end() const {return m_map.end();}


  private:


    /// insert histogram into proper slot
    void insertHist(HistType *hist_ptr) {
      assert(hist_ptr != 0);
         
      std::string name(hist_ptr->GetName());

      IdxType idx(name2Idx(name));

      m_map[idx] = hist_ptr;
    }

    /// load all associated histograms from histogram file
    void loadHists(TDirectory &readDir) {
      std::vector<HistType*> histList(CGCUtil::harvestROOTObjs<HistType>(readDir, m_histBasename));
      
      std::for_each(histList.begin(),
                    histList.end(),
                    std::bind1st(std::mem_fun(&HistMap<IdxType,HistType>::insertHist),this));
               
    }

    std::string genHistName(const IdxType &idx) {
      static std::string prefix(m_histBasename + "_");
      return m_histBasename + "_" + idx.toStr();
    }

    /// convert histogram name back to index obj
    IdxType name2Idx(const std::string &name) {
      static std::string prefix(m_histBasename + "_");

      // first check that histogram name matches pattern
      if (name.find(prefix) != 0)
        throw std::runtime_error(string("Histogram : ") + name + "does not belong to collection: " + m_histBasename);

      // trim collection string from histogram name
      return IdxType(name.substr(prefix.size()));
    }

    HistType *genHist(const IdxType &idx) {
      if (m_writeDir == 0)
        throw std::runtime_error("HistVec::genHist() : Write directory not set for HistVec class");

      const std::string histname(genHistName(idx));

      const std::string subdir(genHistPath(idx));

      HistType *const hist(new HistType(histname.c_str(),
                                        histname.c_str(),
                                        m_nBins,
                                        m_loBinLimit,
                                        m_hiBinLimit));

      TDirectory *const histdir(CGCUtil::deliverROOTDir(m_writeDir, subdir));
      hist->SetDirectory(histdir);

      return hist;
    }

    MapType m_map;

    
    /// generate appropriate subdirectory for histogram
    std::string genHistPath(const IdxType &idx) {
      std::ostringstream tmp;
      tmp << m_histBasename << "/"
          << toPath(idx);
      return tmp.str();
    }

    const std::string m_histBasename;

    /// option for creating new histogram
    const size_t m_nBins;
    /// option for creating new histogram
    const double m_loBinLimit;
    /// option for creating new histogram
    const double m_hiBinLimit;

    /// all new (& modified) histograms written to this directory
    TDirectory * m_writeDir;

  };
}; // namespace calibGenCAL
#endif
