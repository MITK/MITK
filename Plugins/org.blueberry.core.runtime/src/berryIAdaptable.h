/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _BERRY_IADAPTABLE_H_
#define _BERRY_IADAPTABLE_H_

#include <org_blueberry_core_runtime_Export.h>

#include <berryObject.h>

#include <berryLog.h>

#include <typeinfo>

namespace berry {

/**
 * An interface for an adaptable object.
 * <p>
 * Adaptable objects can be dynamically extended to provide different
 * interfaces (or "adapters").  Adapters are created by adapter
 * factories, which are in turn managed by type by adapter managers.
 * </p>
 * For example,
 * <pre>
 *     IAdaptable a = [some adaptable];
 *     IFoo x = (IFoo)a.getAdapter(IFoo.class);
 *     if (x != null)
 *         [do IFoo things with x]
 * </pre>
 * <p>
 * This interface can be used without OSGi running.
 * </p><p>
 * Clients may implement this interface, or obtain a default implementation
 * of this interface by subclassing <code>PlatformObject</code>.
 * </p>
 * @see IAdapterFactory
 * @see IAdapterManager
 * @see PlatformObject
 */
struct org_blueberry_core_runtime_EXPORT IAdaptable
{

public:

  /**
   * Returns an object which is an instance of the given class
   * associated with this object. Returns <code>null</code> if
   * no such object can be found.
   *
   * @param adapterType the adapter class to look up
   * @return a object castable to the given class,
   *    or <code>null</code> if this object does not
   *    have an adapter for the given class
   */
  virtual Object* GetAdapter(const QString& adapterType) const = 0;

  template<class A>
  A* GetAdapter()
  {
    const char* typeName = qobject_interface_iid<A*>();
    if (typeName == nullptr)
    {
      BERRY_WARN << "Error getting adapter for '" << Reflection::DemangleName(typeid(*this).name()) << "': "
                 << "Cannot get the interface id for type '" << Reflection::GetClassName<A>()
                 << "'. It is probably missing a Q_DECLARE_INTERFACE macro in its header.";
      return nullptr;
    }
    return dynamic_cast<A*>(this->GetAdapter(typeName));
  }

  virtual ~IAdaptable();

};

}  // namespace berry

#endif /*_BERRY_IADAPTABLE_H_*/
