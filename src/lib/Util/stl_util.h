#ifndef stl_util_h
#define stl_util_h

// STD INCLUDES
#include <vector>
#include <functional>
#include <streambuf>
#include <ostream>
#include <ios>

//$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/src/lib/Util/CGCUtil.h,v 1.5 2007/06/07 17:45:43 fewtrell Exp $

/** @file
    @author  Zachary Fewtrell
    
    @brief collection of utilities for working w/ various C++ STL elements
*/

namespace calibGenCAL {
  /// append elements of RH collection to LH collection
  template <class LHContainer, class RHContainer>
  void append(LHContainer &lhs, const RHContainer &rhs) {
    std::copy(rhs.begin(),
              rhs.end(),
              std::back_inserter(lhs));
  }

  /// Template function fills any STL type container with zero values
  template <class T> void fill_zero(T &container) {
    fill(container.begin(), container.end(), 0);
  }


  /// return minimum value from an STL vector
  template<typename T> const T & max_val(const std::vector<T> &vec) {
    return *(max_element(vec.begin(), vec.end()));
  }
                     
  /// return minimum value from an STL vector
  template<typename T> const T & min_val(const std::vector<T> &vec) {
    return *(min_element(vec.begin(), vec.end()));
  }
                     
  /// iterator helper class for stl map allows you to index through std::map::mapped_type instead of std::map::value_type (which is sometimes annoying std::pair))
  template <typename MapType>
  class map_val_iterator {
  public:
    explicit map_val_iterator(const typename MapType::iterator &it) :
      it(it) {
    }
                                             
    typename MapType::mapped_type &operator*() {
      return it->second;
    }
                                             
    const typename MapType::mapped_type &operator*() const {
      return it->second;
    }
                                             
    /// prefix ++ operator
    map_val_iterator operator++() {
      map_val_iterator tmp(*this);
      it++;
      return tmp;
    }
                                             
    /// postfix ++ operator
    map_val_iterator &operator++(int) {
      it++;
      return *this;
    }
                                             
    bool operator!=(const map_val_iterator &that) const {
      return it != that.it;
    }
                                             
  private:
    typename MapType::iterator it;
  };

  /// execute functor(ptr) only if ptr != 0                                                                                                                                                                  
  template <class _Functor>
  struct ifptr_fun_t : public std::unary_function<typename _Functor::argument_type,
                                                  typename _Functor::result_type>
  {
  private:
    _Functor op;

  public:
    typedef typename _Functor::result_type result_type;
    typedef typename _Functor::argument_type argument_type;

    explicit ifptr_fun_t(const _Functor &_op) : op(_op) {}

    result_type operator() (const argument_type &x) const {
      if (x != 0)
        return op(x);
    }
  };

  /// return ifptr_fun_t obj
  template <class _Functor>
  ifptr_fun_t<_Functor> ifptr_fun( const _Functor &op) {
    return ifptr_fun_t<_Functor>(op);
  }

  /// like STL back_insert_iterator instead it uses push() instead of push_back()
  /// \note 'inspired' by g++ 3.4 STL implementation of similar iterators
  template<typename ContainerType> 
  class push_insert_iterator {
  private:
    ContainerType *container;
  public:
    typedef ContainerType container_type;

    explicit push_insert_iterator(ContainerType &c) : container(&c) {}

    push_insert_iterator& operator=(const typename container_type::value_type &val) {
      container->push(val);
      return *this;
    }
      
    /// Simply returns *this.                                                                    
    push_insert_iterator&
    operator*()
    { return *this; }

    /// Simply returns *this.  (This iterator does not "move".)                                 
    push_insert_iterator&
    operator++()
    { return *this; }

    /// Simply returns *this.  (This iterator does not "move".)                                 
    push_insert_iterator
    operator++(int)
    { return *this; }

  };


  /** wrapper class for treating multiple streambuf objects as one
   *
   * used by multiplexor_ostream
   */
  class multiplexor_streambuf :
      public std::streambuf {
  public:
    multiplexor_streambuf() :
        std::streambuf() {
    }

    virtual int overflow(int c) {
      // write the incoming character into each stream
      streamvector::iterator _b = _streams.begin(), _e = _streams.end();


      for (; _b != _e; _b++)
        (*_b)->put(c);

      return c;
    }

  public:
    typedef std::vector<std::ostream *> streamvector;
    streamvector _streams;
  };

  /** ostream class will write to any number of streambuffers simultaneously
   *
   * taken from here:
   * http://www.gamedev.net/community/forums/topic.asp?topic_id=286078
   *
   * CoffeeMug  GDNet+  Member since: 3/25/2003  From: NY, USA
   * Posted - 12/1/2004 9:41:18 PM
   *
   *
   */
  class multiplexor_ostream :
      public std::ostream
  {
  public:
    multiplexor_ostream() :
        std::ios(0),
      std::ostream(new multiplexor_streambuf()) {
    }

    virtual ~multiplexor_ostream() {
      delete rdbuf();
    }

    multiplexor_streambuf::streamvector & getostreams() {
      return (dynamic_cast<multiplexor_streambuf *>(rdbuf())->_streams);
    }
  };



}

#endif
