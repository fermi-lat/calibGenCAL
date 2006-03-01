#ifndef CalVec_h
#define CalVec_h

// LOCAL INLUDES
#include "CalDefs.h"

// GLAST INCLUDES

// EXTLIB INCLUDES

// STD INCLUDES
#include <vector>
#include <algorithm>

namespace CalDefs {
  
  /** \brief STL vector wrapper restricts indexing to specified type.
      
  \author Zachary Fewtrell
      
  intended for use w/ index classes from CalDefs.h.

  \note default initial size of vector is 0
  \note default initial vector values are all set to 0.

  \param idx_type vector index type.
  \param val_type vector value type.
  */


  template <typename idx_type, typename val_type >
    class CalVec : protected vector<val_type > {
    protected:
    typedef vector<val_type> parent_type;
    typedef size_t size_type;
    typedef typename parent_type::reference reference;
    typedef typename parent_type::const_reference const_reference;
    typedef typename parent_type::iterator iterator;
    typedef typename parent_type::const_iterator const_iterator;

    public:
    CalVec() : parent_type() {};
    CalVec(size_type sz) : parent_type(sz) {}
    CalVec(size_type sz, const val_type &val) : parent_type(sz,val) {}

    reference operator[] (const idx_type &idx) {
      return parent_type::operator[](idx.val());
    }
    const_reference operator[] (const idx_type &idx) const {
      return parent_type::operator[](idx.val());
    }

    reference at(const idx_type &idx) {
      return parent_type::at(idx.val());
    }
    const_reference at(const idx_type &idx) const {
      return parent_type::at(idx.val());
    }

    void resize(size_type sz) {
      parent_type::resize(sz);
    }

    void resize(size_type sz, const val_type &val) {
      parent_type::resize(sz, val);
    }
    
    void clear() {
      parent_type::clear();
    }
    
    size_type size() const {
      return parent_type::size();
    }
 
    const_iterator begin() const {return parent_type::begin();}
    iterator begin() {return parent_type::begin();}

    const_iterator end() const {return parent_type::end();}
    iterator end() {return parent_type::end();}

    /// just like std::fill() from <algorithm>, fills entire vector
    void fill(const val_type &val) {
      std::fill(parent_type::begin(),
                parent_type::end(),
                val);
    }

    /// just like std::find() from <algorithm>, searches entire vector
    /// \return iterator to 1st matching val, or end() if no match
    iterator find(const val_type &val) {
      return std::find(parent_type::begin(),
                       parent_type::end(),
                       val);
    }

    /// just like std::find() from <algorithm>, searches entire vector
    /// \return iterator to 1st matching val, or end() if no match
    const_iterator find(const val_type &val) const {
      return std::find(parent_type::begin(),
                       parent_type::end(),
                       val);
    }

    void push_back(const val_type &val) {
      parent_type::push_back(val);}
    
  };
};

#endif
