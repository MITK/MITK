/*=========================================================================
 
 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$
 
 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 
 =========================================================================*/

#ifndef COPYONWRITEMAP_H_
#define COPYONWRITEMAP_H_

#include <osgi/framework/ObjectMap.h>
#include "berryPair.h"

#include <Poco/Mutex.h>

#include <set>

namespace berry
{
namespace osgi
{
namespace internal
{

template<typename _Tmap>
struct _cow_map_iterator
{
  typedef typename _Tmap::Pointer map_pointer;
  typedef typename _Tmap::value_type value_type;
  typedef typename _Tmap::value_type& reference;
  typedef typename _Tmap::value_type* pointer;

  typedef std::bidirectional_iterator_tag iterator_category;
  typedef std::ptrdiff_t difference_type;

  typedef _cow_map_iterator<_Tmap> _Self;

  typedef typename _Tmap::iterator map_iterator;
  typedef typename _Tmap::iterator::pointer map_iterator_pointer;
  typedef typename _Tmap::key_type key_type;
  typedef typename _Tmap::mapped_type mapped_type;
  typedef typename _Tmap::key_compare key_compare;
  typedef typename _Tmap::allocator_type allocator_type;

  typedef ::osgi::framework::ObjectMap<key_type, mapped_type, key_compare,
      allocator_type> object_map;

  _cow_map_iterator(const map_iterator& __i, map_pointer __m) :
    _M_iter(__i), _M_map(__m)
  {
  }

  _Tmap* map()
  {
    return _M_map.GetPointer;
  }

  _Self end()
  {
    return _Self(_M_map.end(), _M_map);
  }

  reference operator*() const
  {
    return _M_iter.operator*();
  }

  map_iterator_pointer operator->() const
  {
    return _M_iter.operator->();
  }

  _Self&
  operator++()
  {
    ++_M_iter;
    return *this;
  }

  _Self operator++(int)
  {
    _Self __tmp = *this;
    _M_iter++;
    return __tmp;
  }

  _Self&
  operator--()
  {
    ++_M_iter;
    return *this;
  }

  _Self operator--(int)
  {
    _Self __tmp = *this;
    _M_iter++;
    return __tmp;
  }

  bool operator==(const _Self& __x) const
  {
    return _M_map == __x._M_map && _M_iter == __x._M_iter;
  }

  bool operator!=(const _Self& __x) const
  {
    return _M_map != __x._M_map && _M_iter != __x._M_iter;
  }

  map_iterator _M_iter;
  map_pointer _M_map;

};

template<typename _Tp>
struct _cow_map_const_iterator
{
  typedef typename _Tp::ConstPointer const_pointer;
  typedef _Tp value_type;
  typedef const _Tp& reference;
  typedef const _Tp* pointer;

  typedef _cow_map_iterator<_Tp> iterator;

  typedef std::bidirectional_iterator_tag iterator_category;
  typedef std::ptrdiff_t difference_type;

  typedef _cow_map_const_iterator<_Tp> _Self;

  typedef typename _Tp::iterator map_iterator;
  typedef typename _Tp::const_iterator map_const_iterator;
  typedef typename _Tp::iterator::pointer map_iterator_pointer;
  typedef typename _Tp::const_iterator::pointer map_const_iterator_pointer;
  typedef typename _Tp::key_type key_type;
  typedef typename _Tp::mapped_type mapped_type;
  typedef typename _Tp::key_compare key_compare;
  typedef typename _Tp::allocator_type allocator_type;

  typedef ::osgi::framework::ObjectMap<key_type, mapped_type, key_compare,
      allocator_type> object_map;

  _cow_map_const_iterator(const map_const_iterator& __it, const_pointer __m) :
    _M_iter(__it), _M_map(__m)
  {
  }

  _cow_map_const_iterator(const iterator& __it) :
    _M_iter(__it._M_iter), _M_map(__it._M_map)
  {
  }

  _Self end()
  {
    return _Self(_M_map->end(), _M_map);
  }

  reference operator*() const
  {
    return _M_iter.operator*();
  }

  map_const_iterator_pointer operator->() const
  {
    return _M_iter.operator->();
  }

  _Self&
  operator++()
  {
    ++_M_iter;
    return *this;
  }

  _Self operator++(int)
  {
    _Self __tmp = *this;
    _M_iter++;
    return __tmp;
  }

  _Self&
  operator--()
  {
    ++_M_iter;
    return *this;
  }

  _Self operator--(int)
  {
    _Self __tmp = *this;
    _M_iter++;
    return __tmp;
  }

  bool operator==(const _Self& __x) const
  {
    return _M_iter == __x._M_iter;
  }

  bool operator!=(const _Self& __x) const
  {
    return _M_iter != __x._M_iter;
  }

  map_const_iterator _M_iter;
  const_pointer _M_map;

};

template<typename _Val>
inline bool operator==(const _cow_map_iterator<_Val>& __x,
    const _cow_map_const_iterator<_Val>& __y)
{
  return __x._M_map.GetPointer() == __y._M_map && __x._M_iter == __y._M_iter;
}

template<typename _Val>
inline bool operator!=(const _cow_map_iterator<_Val>& __x,
    const _cow_map_const_iterator<_Val>& __y)
{
  return __x._M_map.GetPointer() != __y._M_map || __x._M_iter != __y._M_iter;
}

/**
 * A copy-on-write identity map. Write operations result in copying the underlying data so that
 * simultaneous read operations are not affected.
 * This allows for safe, unsynchronized traversal.
 *
 * <p>
 * Note: This class uses identity for key and value comparison, not equals.
 *
 */
template<typename Key, typename T, class Cmp = std::less<Key>,
    class A = std::allocator<std::pair<const Key, T> > >
class CopyOnWriteMap
{
private:

  typedef ::osgi::framework::ObjectMap<Key, T, Cmp, A> StdMap;

  typename StdMap::Pointer map;

  Poco::FastMutex _M_mutex;

public:

  typedef typename StdMap::key_type key_type;
  typedef typename StdMap::mapped_type mapped_type;
  typedef typename StdMap::value_type value_type;
  typedef typename StdMap::key_compare key_compare;
  typedef typename StdMap::value_compare value_compare;
  typedef typename StdMap::allocator_type allocator_type;

  typedef typename StdMap::Pointer pointer;
  typedef typename StdMap::ConstPointer const_pointer;
  typedef typename StdMap::reference reference;
  typedef typename StdMap::const_reference const_reference;
  typedef _cow_map_iterator<StdMap> iterator;
  typedef _cow_map_const_iterator<StdMap> const_iterator;
  typedef typename StdMap::size_type size_type;
  typedef typename StdMap::difference_type difference_type;
  typedef _cow_map_iterator<StdMap> reverse_iterator;
  typedef _cow_map_const_iterator<StdMap> const_reverse_iterator;

  /**
   *  @brief  Default constructor creates no elements.
   */
  CopyOnWriteMap()
  {
  }

  /**
   *  @brief  Creates a CopyOnWriteMap with no elements.
   *  @param  comp  A comparison object.
   *  @param  a  An allocator object.
   */
  explicit CopyOnWriteMap(const Cmp& __comp, const allocator_type& __a =
      allocator_type())
  {
    map = new StdMap(__comp, __a);
  }

  /**
   *  @brief  CopyOnWriteMap copy constructor.
   *  @param  x  A CopyOnWriteMap of identical element and allocator types.
   *
   *  The newly-created CopyOnWriteMap uses a copy of the allocation object
   *  used by @a x.
   */
  CopyOnWriteMap(const CopyOnWriteMap& __x)
  {
    const pointer __tmp(__x.map);
    map = new StdMap(*(__tmp.GetPointer()));
  }

  /**
   *  @brief  Builds a CopyOnWriteMap from a range.
   *  @param  first  An input iterator.
   *  @param  last  An input iterator.
   *
   *  Create a CopyOnWriteMap consisting of copies of the elements from [first,last).
   *  This is linear in N if the range is already sorted, and NlogN
   *  otherwise (where N is distance(first,last)).
   */
  template<typename _InputIterator>
  CopyOnWriteMap(_InputIterator __first, _InputIterator __last) :
    map(new StdMap(__first, __last))
  {
  }

  /**
   *  @brief  Builds a CopyOnWriteMap from a range.
   *  @param  first  An input iterator.
   *  @param  last  An input iterator.
   *  @param  comp  A comparison functor.
   *  @param  a  An allocator object.
   *
   *  Create a CopyOnWriteMap consisting of copies of the elements from [first,last).
   *  This is linear in N if the range is already sorted, and NlogN
   *  otherwise (where N is distance(first,last)).
   */
  template<typename _InputIterator>
  CopyOnWriteMap(_InputIterator __first, _InputIterator __last,
      const key_compare& __comp, const allocator_type& __a = allocator_type()) :
    map(new StdMap(__first, __last, __comp, __a))
  {
  }

  /**
   *  @brief  %Map assignment operator.
   *  @param  x  A %map of identical element and allocator types.
   *
   *  All the elements of @a x are copied, but unlike the copy constructor,
   *  the allocator object is not copied.
   */
  CopyOnWriteMap&
  operator=(const CopyOnWriteMap& __x)
  {
    Poco::FastMutex::ScopedLock lock(_M_mutex);
    pointer __tmp(new StdMap(*(__x.map.GetPointer())));
    map = __tmp;
    return *this;
  }

  /// Get a copy of the memory allocation object.
  allocator_type get_allocator() const
  {
    return map->get_allocator();
  }

  /**
   * Returns a snapshot of the entries in this map.
   * The returned set will NOT be changed by future changes to this map.
   *
   */
  template<class T1, class T2>
  std::set<Pair<T1, T2> > entry_set() const
  {
    const_pointer __tmp(map);
    std::set<Pair<T1, T2> > __entries;
    for (typename StdMap::const_iterator __i = __tmp->begin(); __i != __tmp->end(); ++__i)
    {
      Pair<T1, T2> p(MakePair(__i->first, __i->second));
      __entries.insert(p);
    }
    return __entries;
  }

  /**
   * Returns a snapshot of the entries in this map.
   * The returned set will NOT be changed by future changes to this map.
   *
   */
  std::set<Pair<key_type, value_type> > entry_set() const
  {
    const_pointer __tmp(map);
    std::set<Pair<key_type, value_type> > __entries;
    for (typename StdMap::const_iterator __i = __tmp->begin(); __i != __tmp->end(); ++__i)
    {
      __entries.insert(MakePair(__i->first, __i->second));
    }
    return __entries;
  }

  // iterators
  /**
   *  Returns a read/write iterator that points to the first pair in the
   *  %map. Actually, the iterator points to the first pair in a snapshot of the map.
   *  The snapshot is not changed by future changes to this %map.
   *  Iteration is done in ascending order according to the keys.
   */
  iterator begin()
  {
    pointer __tmp(map);
    return iterator(__tmp->begin(), __tmp);
  }

  /**
   *  Returns a read-only (constant) iterator that points to the first pair
   *  in the %map.  Iteration is done in ascending order according to the
   *  keys.
   */
  const_iterator begin() const
  {
    const_pointer __tmp(map);
    return const_iterator(__tmp->begin(), __tmp);
  }

  /**
   *  Returns a read/write iterator that points one past the last
   *  pair in the %map.  Iteration is done in ascending order
   *  according to the keys.
   */
  iterator end()
  {
    pointer __tmp(map);
    return iterator(__tmp->end(), __tmp);
  }

  /**
   *  Returns a read-only (constant) iterator that points one past the last
   *  pair in the %map.  Iteration is done in ascending order according to
   *  the keys.
   */
  const_iterator end() const
  {
    const_pointer __tmp(map);
    return const_iterator(__tmp->end(), __tmp);
  }

  /**
   *  Returns a read/write reverse iterator that points to the last pair in
   *  the %map.  Iteration is done in descending order according to the
   *  keys.
   */
  reverse_iterator rbegin()
  {
    pointer __tmp(map);
    return reverse_iterator(__tmp, __tmp->rbegin());
  }

  /**
   *  Returns a read-only (constant) reverse iterator that points to the
   *  last pair in the %map.  Iteration is done in descending order
   *  according to the keys.
   */
  const_reverse_iterator rbegin() const
  {
    const_pointer __tmp(map);
    return const_reverse_iterator(__tmp->rbegin(), __tmp);
  }

  /**
   *  Returns a read/write reverse iterator that points to one before the
   *  first pair in the %map.  Iteration is done in descending order
   *  according to the keys.
   */
  reverse_iterator rend()
  {
    pointer __tmp(map);
    return reverse_iterator(__tmp, __tmp->rend());
  }

  /**
   *  Returns a read-only (constant) reverse iterator that points to one
   *  before the first pair in the %map.  Iteration is done in descending
   *  order according to the keys.
   */
  const_reverse_iterator rend() const
  {
    const_pointer __tmp(map);
    return const_reverse_iterator(__tmp->rend(), __tmp);
  }

  // capacity
  /** Returns true if the %map is empty.  (Thus begin() would equal
   *  end().)
   */
  bool empty() const
  {
    const_pointer __tmp(map);
    return __tmp->empty();
  }

  /** Returns the size of the %map.  */
  size_type size() const
  {
    const_pointer __tmp(map);
    return __tmp->size();
  }

  /** Returns the maximum size of the %map.  */
  size_type max_size() const
  {
    const_pointer __tmp(map);
    return __tmp->max_size();
  }

  // [23.3.1.2] element access
  /**
   *  @brief  Subscript ( @c [] ) access to %map data.
   *  @param  k  The key for which data should be retrieved.
   *  @return  A reference to the data of the (key,data) %pair.
   *
   *  Allows for easy lookup with the subscript ( @c [] )
   *  operator.  Returns data associated with the key specified in
   *  subscript.  If the key does not exist, a pair with that key
   *  is created using default values, which is then returned.
   *
   *  Lookup requires logarithmic time.
   */
  mapped_type&
  operator[](const key_type& __k)
  {
    pointer __tmp(map);
    return __tmp->operator[](__k);
  }

  /**
   *  @brief  Access to %map data.
   *  @param  k  The key for which data should be retrieved.
   *  @return  A reference to the data whose key is equivalent to @a k, if
   *           such a data is present in the %map.
   *  @throw  std::out_of_range  If no such data is present.
   */
  mapped_type&
  at(const key_type& __k)
  {
    pointer __tmp(map);
    return __tmp->at(__k);
  }

  const mapped_type&
  at(const key_type& __k) const
  {
    const_pointer __tmp(map);
    return __tmp->at(__k);
  }

  // modifiers
  /**
   *  @brief Attempts to insert a std::pair into the %map.

   *  @param  x  Pair to be inserted (see std::make_pair for easy creation
   *       of pairs).

   *  @return  A pair, of which the first element is an iterator that
   *           points to the possibly inserted pair, and the second is
   *           a bool that is true if the pair was actually inserted.
   *
   *  This function attempts to insert a (key, value) %pair into the %map.
   *  A %map relies on unique keys and thus a %pair is only inserted if its
   *  first element (the key) is not already present in the %map.
   *
   *  Insertion requires logarithmic time.
   */
  std::pair<iterator, bool> insert(const value_type& __x)
  {
    Poco::FastMutex::ScopedLock lock(_M_mutex);
    pointer __tmp(new StdMap(*map));
    std::pair<typename StdMap::iterator, bool> __tmp_result(__tmp->insert(__x));
    std::pair<iterator, bool> __result(iterator(__tmp_result.first, __tmp), __tmp_result.second);
    map = __tmp;
    return __result;
  }

  /**
   *  @brief Attempts to insert a std::pair into the %map.
   *  @param  position  An iterator that serves as a hint as to where the
   *                    pair should be inserted.
   *  @param  x  Pair to be inserted (see std::make_pair for easy creation
   *             of pairs).
   *  @return  An iterator that points to the element with key of @a x (may
   *           or may not be the %pair passed in).
   *

   *  This function is not concerned about whether the insertion
   *  took place, and thus does not return a boolean like the
   *  single-argument insert() does.  Note that the first
   *  parameter is only a hint and can potentially improve the
   *  performance of the insertion process.  A bad hint would
   *  cause no gains in efficiency.
   *
   *  See
   *  http://gcc.gnu.org/onlinedocs/libstdc++/manual/bk01pt07ch17.html
   *  for more on "hinting".
   *
   *  Insertion requires logarithmic time (if the hint is not taken).
   */
  iterator insert(iterator __position, const value_type& __x)
  {
    Poco::FastMutex::ScopedLock lock(_M_mutex);
    pointer __tmp(new StdMap(*map));
    typename StdMap::iterator __tmp_i(__tmp->insert(__position, __x));
    map = __tmp;
    return iterator(__tmp_i, __tmp);
  }

  /**
   *  @brief Template function that attempts to insert a range of elements.
   *  @param  first  Iterator pointing to the start of the range to be
   *                 inserted.
   *  @param  last  Iterator pointing to the end of the range.
   *
   *  Complexity similar to that of the range constructor.
   */
  template<typename _InputIterator>
  void insert(_InputIterator __first, _InputIterator __last)
  {
    Poco::FastMutex::ScopedLock lock(_M_mutex);
    pointer __tmp(new StdMap(*map));
    __tmp->insert(__first, __last);

    map = __tmp;
  }

  /**
   *  @brief Erases an element from a %map.
   *  @param  position  An iterator pointing to the element to be erased.
   *
   *  This function erases an element, pointed to by the given
   *  iterator, from a %map.  Note that this function only erases
   *  the element, and that if the element is itself a pointer,
   *  the pointed-to memory is not touched in any way.  Managing
   *  the pointer is the user's responsibility.
   */
  void erase(iterator __position)
  {
    Poco::FastMutex::ScopedLock lock(_M_mutex);
    pointer __tmp(new StdMap(*map));
    __tmp->erase(__position);
    map = __tmp;
  }

  /**
   *  @brief Erases elements according to the provided key.
   *  @param  x  Key of element to be erased.
   *  @return  The number of elements erased.
   *
   *  This function erases all the elements located by the given key from
   *  a %map.
   *  Note that this function only erases the element, and that if
   *  the element is itself a pointer, the pointed-to memory is not touched
   *  in any way.  Managing the pointer is the user's responsibility.
   */
  size_type erase(const key_type& __x)
  {
    Poco::FastMutex::ScopedLock lock(_M_mutex);
    pointer __tmp(new StdMap(*map));
    size_type s = __tmp->erase(__x);
    map = __tmp;
    return s;
  }

  /**
   *  @brief Erases a [first,last) range of elements from a %map.
   *  @param  first  Iterator pointing to the start of the range to be
   *                 erased.
   *  @param  last  Iterator pointing to the end of the range to be erased.
   *
   *  This function erases a sequence of elements from a %map.
   *  Note that this function only erases the element, and that if
   *  the element is itself a pointer, the pointed-to memory is not touched
   *  in any way.  Managing the pointer is the user's responsibility.
   */
  void erase(iterator __first, iterator __last)
  {
    Poco::FastMutex::ScopedLock lock(_M_mutex);
    pointer __tmp(new StdMap(*map));
    __tmp->erase(__first, __last);
    map = __tmp;
  }

  /**
   *  @brief  Swaps data with another %map.
   *  @param  x  A %map of the same element and allocator types.
   *
   *  This exchanges the elements between two maps in constant
   *  time.  (It is only swapping a pointer, an integer, and an
   *  instance of the @c Compare type (which itself is often
   *  stateless and empty), so it should be quite fast.)  Note
   *  that the global std::swap() function is specialized such
   *  that std::swap(m1,m2) will feed to this function.
   */
  //void
  //swap(map& __x)
  //{ _M_t.swap(__x._M_t); }

  /**
   *  Erases all elements in a %map.  Note that this function only
   *  erases the elements, and that if the elements themselves are
   *  pointers, the pointed-to memory is not touched in any way.
   *  Managing the pointer is the user's responsibility.
   */
  void clear()
  {
    Poco::FastMutex::ScopedLock lock(_M_mutex);
    pointer __tmp(new StdMap(*map));
    map->clear();
  }

  // observers
  /**
   *  Returns the key comparison object out of which the %map was
   *  constructed.
   */
  key_compare key_comp() const
  {
    return map->key_comp();
  }

  /**
   *  Returns a value comparison object, built from the key comparison
   *  object out of which the %map was constructed.
   */
  value_compare value_comp() const
  {
    return map->value_comp();
  }

  // [23.3.1.3] map operations
  /**
   *  @brief Tries to locate an element in a %map.
   *  @param  x  Key of (key, value) %pair to be located.
   *  @return  Iterator pointing to sought-after element, or end() if not
   *           found.
   *
   *  This function takes a key and tries to locate the element with which
   *  the key matches.  If successful the function returns an iterator
   *  pointing to the sought after %pair.  If unsuccessful it returns the
   *  past-the-end ( @c end() ) iterator.
   */
  iterator find(const key_type& __x)
  {
    pointer __tmp(map);
    return iterator(__tmp->find(__x), __tmp);
  }

  /**
   *  @brief Tries to locate an element in a %map.
   *  @param  x  Key of (key, value) %pair to be located.
   *  @return  Read-only (constant) iterator pointing to sought-after
   *           element, or end() if not found.
   *
   *  This function takes a key and tries to locate the element with which
   *  the key matches.  If successful the function returns a constant
   *  iterator pointing to the sought after %pair. If unsuccessful it
   *  returns the past-the-end ( @c end() ) iterator.
   */
  const_iterator find(const key_type& __x) const
  {
    pointer __tmp(map);
    return const_iterator(__tmp->find(__x), __tmp);
  }

  /**
   *  @brief  Finds the number of elements with given key.
   *  @param  x  Key of (key, value) pairs to be located.
   *  @return  Number of elements with specified key.
   *
   *  This function only makes sense for multimaps; for map the result will
   *  either be 0 (not present) or 1 (present).
   */
  size_type count(const key_type& __x) const
  {
    pointer __tmp(map);
    return __tmp->count(__x);
  }

  /**
   *  @brief Finds the beginning of a subsequence matching given key.
   *  @param  x  Key of (key, value) pair to be located.
   *  @return  Iterator pointing to first element equal to or greater
   *           than key, or end().
   *
   *  This function returns the first element of a subsequence of elements
   *  that matches the given key.  If unsuccessful it returns an iterator
   *  pointing to the first element that has a greater value than given key
   *  or end() if no such element exists.
   */
  iterator lower_bound(const key_type& __x)
  {
    pointer __tmp(map);
    return __tmp->lower_bound(__x);
  }

  /**
   *  @brief Finds the beginning of a subsequence matching given key.
   *  @param  x  Key of (key, value) pair to be located.
   *  @return  Read-only (constant) iterator pointing to first element
   *           equal to or greater than key, or end().
   *
   *  This function returns the first element of a subsequence of elements
   *  that matches the given key.  If unsuccessful it returns an iterator
   *  pointing to the first element that has a greater value than given key
   *  or end() if no such element exists.
   */
  const_iterator lower_bound(const key_type& __x) const
  {
    pointer __tmp(map);
    return __tmp->lower_bound(__x);
  }

  /**
   *  @brief Finds the end of a subsequence matching given key.
   *  @param  x  Key of (key, value) pair to be located.
   *  @return Iterator pointing to the first element
   *          greater than key, or end().
   */
  iterator upper_bound(const key_type& __x)
  {
    pointer __tmp(map);
    return __tmp->upper_bound(__x);
  }

  /**
   *  @brief Finds the end of a subsequence matching given key.
   *  @param  x  Key of (key, value) pair to be located.
   *  @return  Read-only (constant) iterator pointing to first iterator
   *           greater than key, or end().
   */
  const_iterator upper_bound(const key_type& __x) const
  {
    pointer __tmp(map);
    return __tmp->upper_bound(__x);
  }

  /**
   *  @brief Finds a subsequence matching given key.
   *  @param  x  Key of (key, value) pairs to be located.
   *  @return  Pair of iterators that possibly points to the subsequence
   *           matching given key.
   *
   *  This function is equivalent to
   *  @code
   *    std::make_pair(c.lower_bound(val),
   *                   c.upper_bound(val))
   *  @endcode
   *  (but is faster than making the calls separately).
   *
   *  This function probably only makes sense for multimaps.
   */
  std::pair<iterator, iterator> equal_range(const key_type& __x)
  {
    pointer __tmp(map);
    return __tmp->equal_range(__x);
  }

  /**
   *  @brief Finds a subsequence matching given key.
   *  @param  x  Key of (key, value) pairs to be located.
   *  @return  Pair of read-only (constant) iterators that possibly points
   *           to the subsequence matching given key.
   *
   *  This function is equivalent to
   *  @code
   *    std::make_pair(c.lower_bound(val),
   *                   c.upper_bound(val))
   *  @endcode
   *  (but is faster than making the calls separately).
   *
   *  This function probably only makes sense for multimaps.
   */
  std::pair<const_iterator, const_iterator> equal_range(const key_type& __x) const
  {
    pointer __tmp(map);
    return __tmp->equal_range(__x);
  }

  template<typename _K1, typename _T1, typename _C1, typename _A1>
  friend bool
  operator==(const CopyOnWriteMap<_K1, _T1, _C1, _A1>&, const CopyOnWriteMap<
      _K1, _T1, _C1, _A1>&);

  template<typename _K1, typename _T1, typename _C1, typename _A1>
  friend bool
  operator<(const CopyOnWriteMap<_K1, _T1, _C1, _A1>&, const CopyOnWriteMap<
      _K1, _T1, _C1, _A1>&);
};

/**
 *  @brief  Map equality comparison.
 *  @param  x  A %map.
 *  @param  y  A %map of the same type as @a x.
 *  @return  True iff the size and elements of the maps are equal.
 *
 *  This is an equivalence relation.  It is linear in the size of the
 *  maps.  Maps are considered equivalent if their sizes are equal,
 *  and if corresponding elements compare equal.
 */
template<typename _Key, typename _Tp, typename _Compare, typename _Alloc>
inline bool operator==(const CopyOnWriteMap<_Key, _Tp, _Compare, _Alloc>& __x,
    const CopyOnWriteMap<_Key, _Tp, _Compare, _Alloc>& __y)
{
  return *(__x._M_map) == *(__y._M_map);
}

/**
 *  @brief  Map ordering relation.
 *  @param  x  A %map.
 *  @param  y  A %map of the same type as @a x.
 *  @return  True iff @a x is lexicographically less than @a y.
 *
 *  This is a total ordering relation.  It is linear in the size of the
 *  maps.  The elements must be comparable with @c <.
 *
 *  See std::lexicographical_compare() for how the determination is made.
 */
template<typename _Key, typename _Tp, typename _Compare, typename _Alloc>
inline bool operator<(const CopyOnWriteMap<_Key, _Tp, _Compare, _Alloc>& __x,
    const CopyOnWriteMap<_Key, _Tp, _Compare, _Alloc>& __y)
{
  return *(__x._M_map) < *(__y._M_map);
}

/// Based on operator==
template<typename _Key, typename _Tp, typename _Compare, typename _Alloc>
inline bool operator!=(const CopyOnWriteMap<_Key, _Tp, _Compare, _Alloc>& __x,
    const CopyOnWriteMap<_Key, _Tp, _Compare, _Alloc>& __y)
{
  return !(__x == __y);
}

/// Based on operator<
template<typename _Key, typename _Tp, typename _Compare, typename _Alloc>
inline bool operator>(const CopyOnWriteMap<_Key, _Tp, _Compare, _Alloc>& __x,
    const CopyOnWriteMap<_Key, _Tp, _Compare, _Alloc>& __y)
{
  return __y < __x;
}

/// Based on operator<
template<typename _Key, typename _Tp, typename _Compare, typename _Alloc>
inline bool operator<=(const CopyOnWriteMap<_Key, _Tp, _Compare, _Alloc>& __x,
    const CopyOnWriteMap<_Key, _Tp, _Compare, _Alloc>& __y)
{
  return !(__y < __x);
}

/// Based on operator<
template<typename _Key, typename _Tp, typename _Compare, typename _Alloc>
inline bool operator>=(const CopyOnWriteMap<_Key, _Tp, _Compare, _Alloc>& __x,
    const CopyOnWriteMap<_Key, _Tp, _Compare, _Alloc>& __y)
{
  return !(__x < __y);
}

/// See std::map::swap().
//  template<typename _Key, typename _Tp, typename _Compare, typename _Alloc>
//    inline void
//    swap(map<_Key, _Tp, _Compare, _Alloc>& __x,
//   map<_Key, _Tp, _Compare, _Alloc>& __y)
//    { __x.swap(__y); }

}
}
}

#endif /* COPYONWRITEMAP_H_ */
