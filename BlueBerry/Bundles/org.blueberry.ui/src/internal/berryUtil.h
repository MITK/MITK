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

#ifndef BERRYUTIL_H_
#define BERRYUTIL_H_

namespace berry
{

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
      return 0;
    }
    if (A* adapter = dynamic_cast<A*>(sourceObject.GetPointer()))
    {
      return adapter;
    }

    if (IAdaptable* adaptable = dynamic_cast<IAdaptable*>(sourceObject.GetPointer()))
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
      return 0;
    }

    //        if (!(sourceObject instanceof PlatformObject)) {
    //            Object result = Platform.getAdapterManager().getAdapter(sourceObject, adapterType);
    //            if (result != null) {
    //                return result;
    //            }
    //        }

    return 0;
  }
};

}

#endif /* BERRYUTIL_H_ */
