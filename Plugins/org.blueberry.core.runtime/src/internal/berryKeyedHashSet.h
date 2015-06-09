/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef BERRYKEYEDHASHSET_H
#define BERRYKEYEDHASHSET_H

#include <berrySmartPointer.h>
#include "berryKeyedElement.h"

#include <QSet>

namespace berry {

class KeyedHashSet
{

private:

  class LocalElement;

  QSet<KeyedElement::Pointer> elements;
  bool replace;

public:

  KeyedHashSet(bool replace = true);

  KeyedHashSet(int capacity, bool replace = true);

  /**
   * Adds an element to this set. If an element with the same key already exists,
   * replaces it depending on the replace flag.
   * @return true if the element was added/stored, false otherwise
   */
  bool Add(const KeyedElement::Pointer& element);

  void Clear();

  QList<KeyedElement::Pointer> Elements() const;

  /**
   * Returns the set element with the given id, or null
   * if not found.
   */
  KeyedElement::Pointer Get(const KeyedElement::Pointer& key) const;

  /**
   * Returns the set element with the given id, or null
   * if not found.
   */
  KeyedElement::Pointer GetByKey(const QString& key) const;

  bool IsEmpty() const;

  bool Remove(const KeyedElement::Pointer& toRemove);

  bool RemoveByKey(const QString& key);

  int Size() const;

};

}

#endif // BERRYKEYEDHASHSET_H
