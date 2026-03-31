/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef USSHRINKABLEMAP_H
#define USSHRINKABLEMAP_H

#include <usGlobalConfig.h>

#include <map>

namespace us {

/**
 * \ingroup MicroServicesUtils
 *
 * \brief A std::map style associative container allowing query and removal
 * operations only.
 */
template<class Key, class T>
class ShrinkableMap
{
private:
  static std::map<Key,T> emptyContainer;

public:

  typedef std::map<Key,T> container_type;
  typedef typename container_type::iterator iterator;
  typedef typename container_type::const_iterator const_iterator;
  typedef typename container_type::size_type size_type;
  typedef typename container_type::key_type key_type;
  typedef typename container_type::mapped_type mapped_type;
  typedef typename container_type::value_type value_type;
  typedef typename container_type::reference reference;
  typedef typename container_type::const_reference const_reference;

  ShrinkableMap()
    : container(emptyContainer)
  {
  }

  iterator begin()
  {
    return container.begin();
  }

  const_iterator begin() const
  {
    return container.begin();
  }

  iterator end()
  {
    return container.end();
  }

  const_iterator end() const
  {
    return container.end();
  }

  void erase(iterator pos)
  {
    return container.erase(pos);
  }

  void erase(iterator first, iterator last)
  {
    return container.erase(first, last);
  }

  size_type erase(const Key& key)
  {
    return container.erase(key);
  }

  bool empty() const
  {
    return container.empty();
  }

  void clear()
  {
    container.clear();
  }

  size_type size() const
  {
    return container.size();
  }

  size_type max_size() const
  {
    return container.max_size();
  }

  T& operator[](const Key& key)
  {
    return container[key];
  }

  size_type count(const Key& key) const
  {
    return container.count(key);
  }

  iterator find(const Key& key)
  {
    return container.find(key);
  }

  const_iterator find(const Key& key) const
  {
    return container.find(key);
  }

  std::pair<iterator,iterator> equal_range(const Key& key)
  {
    return container.equal_range(key);
  }

  std::pair<const_iterator,const_iterator> equal_range(const Key& key) const
  {
    return container.equal_range(key);
  }

  iterator lower_bound(const Key& key)
  {
    return container.lower_bound(key);
  }

  const_iterator lower_bound(const Key& key) const
  {
    return container.lower_bound(key);
  }

  iterator upper_bound(const Key& key)
  {
    return container.upper_bound(key);
  }

  const_iterator upper_bound(const Key& key) const
  {
    return container.upper_bound(key);
  }

private:

  friend class ServiceHooks;

  ShrinkableMap(container_type& container)
    : container(container)
  {}

  container_type& container;
};

template<class Key, class T>
std::map<Key,T> ShrinkableMap<Key,T>::emptyContainer;

}

#endif // USSHRINKABLEMAP_H
