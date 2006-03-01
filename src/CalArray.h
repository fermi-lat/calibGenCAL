#ifndef CalArray_h
#define CalArray_h

// LOCAL INLUDES
#include "CalDefs.h"

// GLAST INCLUDES

// EXTLIB INCLUDES

// STD INCLUDES
#include <algorithm>

namespace CalDefs {
  /** \brief Standard C array wrapper restricts array indexing to specified type.

  \author Zachary Fewtrell
      
  - intended for use w/ index classes from CalDefs.h.
  - allows for some STL algorithm operations


  \note initial size of array is idx_type::N_VALS
  \note initial array values are unitialized.

  

  \param idx_type array index type.
  \param val_type array value type.
  */
  template <typename idx_type, typename val_type >
    class CalArray {
    public:
    val_type& operator[](const idx_type &idx) {
      return m_dat[idx.val()];}

    const val_type& operator[](const idx_type &idx) const {
      return m_dat[idx.val()];
    }
      
    /// return size of array
    size_t size() const {return idx_type::N_VALS;}

    /// return pointer to 1st val in array.  good for use w/ STL algs
    val_type *begin() {return m_dat;}
    /// return const pointer to 1st val in array.  good for use w/ STL algs
    const val_type *begin() const {return m_dat;}

    /// return pointer to 1 val past end of array.  god for use w/ STL algs
    val_type *end() {return m_dat + size();}
    /// return const pointer to 1 val past end of array.  god for use w/ STL algs
    const val_type *end() const {return m_dat + size;}

    /// just like std::fill() from <algorithm>, fills entire array
    void fill(const val_type &val) {
      std::fill(begin(),
                end(),
                val);
    }

    /// just like std::find() from <algorithm>, searches entire array
    /// \return iterator to 1st matching val, or end() if no match
    val_type* find(const val_type &val) {
      return std::find(begin(),
                       end(),
                       val);
    }

    /// just like std::find() from <algorithm>, searches entire array
    /// \return iterator to 1st matching val, or end() if no match
    const val_type* find(const val_type &val) const {
      return std::find(begin(),
                       end(),
                       val);
    }

    private:
    /// internal array data
    val_type m_dat[idx_type::N_VALS];
      
  }; 

};

#endif
