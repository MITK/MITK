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

#ifndef BERRYPROPERTYCACHE_H_
#define BERRYPROPERTYCACHE_H_

#include "Poco/LRUCache.h"

#include "berryProperty.h"

namespace berry {

class PropertyCache {

private:

  struct PropertyHandle
  {
    Property* m_Property;
  };

  Poco::LRUCache<Property::Pointer, PropertyHandle> fCache;

public:

  PropertyCache(const int cacheSize) : fCache(cacheSize) {

  }

  Property::Pointer Get(Property::Pointer key) {
    Poco::SharedPtr<PropertyHandle> value = fCache.get(key);
    return value.isNull() ? Property::Pointer(0) : Property::Pointer(value->m_Property);
  }

  void Put(Property::Pointer method) {
    PropertyHandle handle = { method.GetPointer() };
    fCache.add(method, handle);
  }

  void Remove(Property::Pointer method) {
    fCache.remove(method);
  }
};

} // namespace berry

#endif /*BERRYPROPERTYCACHE_H_*/
