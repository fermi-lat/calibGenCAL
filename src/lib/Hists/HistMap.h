#ifndef HistMap_h
#define HistMap_h

// LOCAL INCLUDES
#include "../Util/CGCUtil.h"

// GLAST INCLUDES

// EXTLIB INCLUDES
#include "TDirectory.h"
#include "TTree.h"
#include "TBranch.h"

// STD INCLUDES
#include <map>
#include <string>
#include <algorithm>
#include <functional>
#include <stdexcept>
#include <sstream>

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
    /// \param rootFile if ptr is non-zero, then all matching histograms will be loaded from given file.
    HistMap(const std::string &histBasename,
            const size_t nBins=1000,
            const double loBinLimit=0,
            const double hiBinLimit=0,
            const TDirectory *rootFile=0
            ) :
      m_histBasename(histBasename),
      m_histList(0),
      m_nBins(nBins),
      m_loBinLimit(loBinLimit),
      m_hiBinLimit(hiBinLimit),
      m_treeId(0)
    {
      // read in histlist & initial histograms if rootFile is set
      if (rootFile != 0)
        loadHists(*rootFile);
      else // else build new empty histlist 
        m_histList = &buildHistlist(histBasename);
    }


    /// retrieve histogram for given index, build it if it doesn't exist.
    HistType &checkNGet(const IdxType &idx) {
      typename MapType::iterator it(m_map.lower_bound(idx));

      /// this means that idx has not yet been inserted into the map
      if (it == m_map.end() || it->first != idx)       
        it = m_map.insert(it,ValType(idx, genHist(idx)));
      return *(it->second);
    }

    /// act like STL map
    typedef typename MapType::iterator iterator;

    /// act like STL map
    typedef typename MapType::const_iterator const_iterator;

    /// act like STL map
    iterator begin() {return m_map.begin();}
    /// act like STL map
    const_iterator begin() const {return m_map.begin();}

    /// act like STL map
    iterator end() {return m_map.end();}
    /// act like STL map
    const_iterator end() const {return m_map.end();}

    /// set ROOT directory for each histogram
    void setHistDir(TDirectory *const dir) {
      for_each(CGCUtil::map_val_iterator<MapType>(m_map.begin()),
               CGCUtil::map_val_iterator<MapType>(m_map.end()),
               std::bind2nd(std::mem_fun1_t<void, HistType, TDirectory*>(&HistType::SetDirectory),dir));
    }

  private:
    /// load all associated histogram from histogram file
    void loadHists(const TDirectory &rootFile) {
      // load new histList from file
      m_histList = CGCUtil::retrieveROOTObj<TTree>(rootFile, genHistlistName(m_histBasename));
      
      if (m_histList == 0)
        throw std::runtime_error(std::string("ROOT Obj ") + genHistlistName(m_histBasename) +
                                 "not found in " + rootFile.GetName());

      // loop through m_histList
      TBranch *const branch(m_histList->GetBranch(m_branchName.c_str()));
      if (branch == 0)
        throw std::runtime_error(std::string("Unable to find branch") + m_branchName);
      branch->SetAddress(&m_treeId);

      /// loop through each entry in tree
      const unsigned nHists(branch->GetEntries());
      for (unsigned i(0); i < nHists; i++) {
        branch->GetEvent(i);

        /// build index type from raw integer
        const IdxType idx(m_treeId);
        HistType *hist(CGCUtil::retrieveROOTObj<HistType>(rootFile, genHistName(idx, m_histBasename)));        
        if (hist == 0) {
          std::ostringstream msg;
          const unsigned val(idx.val());
          msg << "Unable to retrieve histogram " << val << " " << genHistName(idx, m_histBasename);
          throw std::runtime_error(msg.str());
        }
        m_map[idx] = hist;
      }
    }

    static std::string genHistName(const IdxType &idx, const std::string &basename) {
      return basename + "_" + idx.toStr();
    }

    HistType *genHist(const IdxType &idx) {
      const std::string name(genHistName(idx, m_histBasename));

      /// update tuple list of existing histograms
      fillHistList(idx);

      return new HistType(name.c_str(),
                          name.c_str(),
                          m_nBins,
                          m_loBinLimit,
                          m_hiBinLimit);
    }

    MapType m_map;

    /// build m_histList tuple for listing all present histograms in sparse array
    TTree &buildHistlist(const std::string &basename) {
      const std::string treename(genHistlistName(basename));
      
      TTree &tree(*(new TTree(treename.c_str(),
                              treename.c_str())));
      
      if (!tree.Branch(m_branchName.c_str(), &m_treeId, (m_branchName + "/i").c_str()))
        throw std::runtime_error("Couldn't create TTree branch");
      
      return tree;
    }
    
    static std::string genHistlistName(const std::string &basename) {
      return basename + "_histList";
    }

    void fillHistList(const IdxType &idx) {
      /// idx needs to be castable to unsigned integer.
      m_treeId = idx.val();
      m_histList->Fill();
    }

    const std::string m_histBasename;

    /// store list of id integers for histograms created so far.
    TTree *m_histList;

    const size_t m_nBins;
    const double m_loBinLimit;
    const double m_hiBinLimit;

    /// used for filling the m_histList tuple
    UInt_t m_treeId;

    static const std::string &m_branchName;
  };
  

  extern const std::string HISTMAP_BRANCHNAME;

  template <typename IdxType,
            typename HistType> 
  const std::string &HistMap<IdxType,HistType>::m_branchName(HISTMAP_BRANCHNAME);

}; // namespace calibGenCAL
#endif
