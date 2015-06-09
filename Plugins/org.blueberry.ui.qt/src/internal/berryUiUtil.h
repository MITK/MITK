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

#ifndef BERRYUIUTIL_H
#define BERRYUIUTIL_H

#include "berryIAdaptable.h"
#include "berryIAdapterManager.h"
#include "berryPlatformObject.h"
#include "berryPlatform.h"

namespace berry {

class UiUtil
{

public:

  /**
   * If it is possible to adapt the given object to the given type, this
   * returns the adapter. Performs the following checks:
   *
   * <ol>
   * <li>Returns <code>sourceObject</code> if it is an instance of the
   * adapter type.</li>
   * <li>If sourceObject implements IAdaptable, it is queried for adapters.</li>
   * <li>If sourceObject is not an instance of PlatformObject (which would have
   * already done so), the adapter manager is queried for adapters</li>
   * </ol>
   *
   * Otherwise returns null.
   *
   * @param sourceObject
   *            object to adapt, or null
   * @param adapterType
   *            type to adapt to
   * @return a representation of sourceObject that is assignable to the
   *         adapter type, or null if no such representation exists
   */
  template<class A>
  static A* GetAdapter(Object* sourceObject)
  {
    if (sourceObject == nullptr)
    {
      return nullptr;
    }
    if (A* adapter = dynamic_cast<A*>(sourceObject))
    {
      return adapter;
    }

    if (IAdaptable* adaptable = dynamic_cast<IAdaptable*>(sourceObject))
    {
      A* result = adaptable->GetAdapter<A>();
      if (result != nullptr)
      {
        return result;
      }
    }

    if (dynamic_cast<PlatformObject*>(sourceObject) == nullptr)
    {
      A* result = nullptr;
      IAdapterManager* adapterManager = Platform::GetAdapterManager();
      if (adapterManager)
      {
        result = adapterManager->GetAdapter<A>(sourceObject);
      }
      if (result != nullptr)
      {
        return result;
      }
    }

    return nullptr;
  }

};

}

#endif // BERRYUIUTIL_H
