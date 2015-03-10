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

#include "berryKeyedHashSet.h"

namespace berry {

class KeyedHashSet::LocalElement : public KeyedElement
{
public:

  LocalElement(const QString& k) : k(k) {}
  QString GetKey() const { return k; }

private:
  QString k;
  bool IsEqual(const KeyedElement& other) const
  {
    return k == static_cast<const LocalElement&>(other).k;
  }
};

KeyedHashSet::KeyedHashSet(bool replace)
  : replace(replace)
{
}

KeyedHashSet::KeyedHashSet(int capacity, bool replace)
  : replace(replace)
{
  elements.reserve(capacity);
}

bool KeyedHashSet::Add(const KeyedElement::Pointer& element)
{
  if (elements.contains(element))
  {
    if (replace)
    {
      elements.remove(element);
      elements.insert(element);
    }
    return replace;
  }
  else
  {
    elements.insert(element);
    return true;
  }
}

void KeyedHashSet::Clear()
{
  elements.clear();
}

QList<KeyedElement::Pointer> KeyedHashSet::Elements() const
{
  return elements.values();
}

KeyedElement::Pointer KeyedHashSet::Get(const KeyedElement::Pointer& key) const
{
  QSet<KeyedElement::Pointer>::const_iterator i = elements.find(key);
  if (i != elements.end()) return *i;
  return KeyedElement::Pointer();
}

KeyedElement::Pointer KeyedHashSet::GetByKey(const QString& key) const
{
  if (elements.empty()) return KeyedElement::Pointer();

  KeyedElement::Pointer x(new LocalElement(key));
  QSet<KeyedElement::Pointer>::const_iterator i = elements.find(x);
  if (i != elements.end()) return *i;
  return KeyedElement::Pointer();
}

bool KeyedHashSet::IsEmpty() const
{
  return elements.empty();
}

bool KeyedHashSet::Remove(const KeyedElement::Pointer& toRemove)
{
  return elements.remove(toRemove);
}

bool KeyedHashSet::RemoveByKey(const QString& key)
{
  if (elements.empty()) return false;

  KeyedElement::Pointer x(new LocalElement(key));
  return elements.remove(x);
}

int KeyedHashSet::Size() const
{
  return elements.size();
}

}
