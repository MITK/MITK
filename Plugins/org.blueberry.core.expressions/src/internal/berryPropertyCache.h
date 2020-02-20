/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
    return value.isNull() ? Property::Pointer(nullptr) : Property::Pointer(value->m_Property);
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
