/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYUTIL_H_
#define BERRYUTIL_H_

#include "berryObject.h"
#include "berryIAdaptable.h"

namespace berry {

class Shell;

class Util
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
  static A* GetAdapter(Object::Pointer sourceObject)
  {
    if (sourceObject == 0)
    {
      return nullptr;
    }
    if (A* adapter = dynamic_cast<A*>(sourceObject.GetPointer()))
    {
      return adapter;
    }

    if (/*IAdaptable* adaptable =*/ dynamic_cast<IAdaptable*>(sourceObject.GetPointer()))
    {
      // TODO IAdaptable
      //            IAdaptable adaptable = (IAdaptable) sourceObject;
      //
      //            Object result = adaptable.getAdapter(adapterType);
      //            if (result != null) {
      //                // Sanity-check
      //                Assert.isTrue(adapterType.isInstance(result));
      //                return result;
      //            }
      return nullptr;
    }

    //        if (!(sourceObject instanceof PlatformObject)) {
    //            Object result = Platform.getAdapterManager().getAdapter(sourceObject, adapterType);
    //            if (result != null) {
    //                return result;
    //            }
    //        }

    return nullptr;
  }

  /**
   * Return an appropriate shell to parent dialogs on. This will be one of the
   * workbench windows (the active one) should any exist. Otherwise
   * <code>null</code> is returned.
   *
   * @return the shell to parent on or <code>null</code> if there is no
   *         appropriate shell
   */
  static SmartPointer<Shell> GetShellToParentOn();

};

}

#endif /* BERRYUTIL_H_ */
