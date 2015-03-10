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

#include "berryRegistryObjectReferenceMap.h"

#include "berryRegistryObject.h"

namespace berry {


struct RegistryObjectReferenceMap::IEntry
{
  virtual ~IEntry() {}

  /**
   * Returns the value of this entry.
   * @return The entry value.
   */
  virtual SmartPointer<RegistryObject> GetValue() const = 0;
};

class RegistryObjectReferenceMap::SmartRef : public RegistryObjectReferenceMap::IEntry
{

private:

  /**
   * Reference value.  Note this can never be null.
   */
  RegistryObject::Pointer value;

public:

  SmartRef(const RegistryObject::Pointer value)
    : value(value)
  {
  }

  RegistryObject::Pointer GetValue() const
  {
    return value;
  }

};

class RegistryObjectReferenceMap::WeakRef : public RegistryObjectReferenceMap::IEntry
{

private:

  /**
   * Reference value.  Note this can never be null.
   */
  RegistryObject::WeakPtr value;

public:

  WeakRef(const RegistryObject::Pointer value)
    : value(value)
  {
  }

  RegistryObject::Pointer GetValue() const
  {
    return value.Lock();
  }

};


RegistryObjectReferenceMap::RegistryObjectReferenceMap(ReferenceType referenceType, int capacity)
  : valueType(referenceType)
{
  references.reserve(capacity);
}

RegistryObjectReferenceMap::~RegistryObjectReferenceMap()
{
  for(ReferenceMapType::Iterator i = references.begin(); i != references.end(); ++i)
  {
    delete i.value();
  }
}

SmartPointer<RegistryObject> RegistryObjectReferenceMap::Get(int key) const
{
  Purge();
  ReferenceMapType::const_iterator entry = references.find(key);
  if (entry != references.end()) return entry.value()->GetValue();
  return RegistryObject::Pointer();
}

void RegistryObjectReferenceMap::Put(int key, const SmartPointer<RegistryObject>& value)
{
  if (value.IsNull())
    throw ctkInvalidArgumentException("null values not allowed");

  Purge();

  ReferenceMapType::Iterator i = references.find(key);
  if (i != references.end())
  {
    delete *i;
  }
  references.insert(key, NewEntry(value));
}

SmartPointer<RegistryObject> RegistryObjectReferenceMap::Remove(int key)
{
  Purge();
  IEntry* entry = references.take(key);
  RegistryObject::Pointer obj = entry->GetValue();
  delete entry;
  return obj;
}

RegistryObjectReferenceMap::IEntry*
RegistryObjectReferenceMap::NewEntry(const SmartPointer<RegistryObject>& value) const
{
  switch (valueType)
  {
  case HARD :
    return new SmartRef(value);
  case SOFT :
    return new WeakRef(value);
  default: return NULL;
  }
}

void RegistryObjectReferenceMap::Purge() const
{
  for (ReferenceMapType::Iterator i = references.begin(); i != references.end();)
  {
    if (i.value()->GetValue().IsNull())
    {
      i = references.erase(i);
    }
    else
    {
      ++i;
    }
  }
}

}
