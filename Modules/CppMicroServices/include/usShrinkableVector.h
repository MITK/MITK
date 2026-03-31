/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef USSHRINKABLEVECTOR_H
#define USSHRINKABLEVECTOR_H

#include <usGlobalConfig.h>

#include <vector>

namespace us {

/**
 * \ingroup MicroServicesUtils
 *
 * A std::vector style container allowing query and removal
 * operations only.
 */
template<class E>
class ShrinkableVector
{
private:
  static std::vector<E> emptyVector;

public:

  typedef std::vector<E> container_type;
  typedef typename container_type::iterator iterator;
  typedef typename container_type::const_iterator const_iterator;
  typedef typename container_type::size_type size_type;
  typedef typename container_type::reference reference;
  typedef typename container_type::const_reference const_reference;

  ShrinkableVector()
  : container(emptyVector)
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

  reference front()
  {
    return container.front();
  }

  const_reference front() const
  {
    return container.front();
  }

  reference back()
  {
    return container.back();
  }

  const_reference back() const
  {
    return container.back();
  }

  iterator erase(iterator pos)
  {
    return container.erase(pos);
  }

  iterator erase(iterator first, iterator last)
  {
    return container.erase(first, last);
  }

  void pop_back()
  {
    container.pop_back();
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

  reference at(size_type pos)
  {
    return container.at(pos);
  }

  const_reference at(size_type pos) const
  {
    return container.at(pos);
  }

  const_reference operator[](size_type i) const
  {
    return container[i];
  }

  reference operator[](size_type i)
  {
    return container[i];
  }

private:

  friend class ModuleHooks;
  friend class ServiceHooks;

  ShrinkableVector(container_type& container)
    : container(container)
  {}

  container_type& container;
};

template<class E>
std::vector<E> ShrinkableVector<E>::emptyVector;

}

#endif // USSHRINKABLEVECTOR_H
