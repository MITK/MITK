/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYPLATFORMOBJECT_H_
#define BERRYPLATFORMOBJECT_H_

#include <org_blueberry_core_runtime_Export.h>

#include <berryObject.h>

#include "berryIAdaptable.h"

namespace berry {

/**
 * An abstract superclass implementing the <code>IAdaptable</code>
 * interface. <code>getAdapter</code> invocations are directed
 * to the platform's adapter manager.
 * <p>
 * Note: In situations where it would be awkward to subclass this
 * class, the same affect can be achieved simply by implementing
 * the <code>IAdaptable</code> interface and explicitly forwarding
 * the <code>getAdapter</code> request to the platform's
 * adapater manager. The method would look like:
 * <pre>
 *     public Object getAdapter(Class adapter) {
 *         return Platform.getAdapterManager().getAdapter(this, adapter);
 *     }
 * </pre>
 * </p>
 * <p>
 * Clients may subclass.
 * </p>
 *
 * @see Platform#getAdapterManager
 */
class org_blueberry_core_runtime_EXPORT PlatformObject : public virtual Object, public virtual IAdaptable
{

public:

  berryObjectMacro(berry::PlatformObject);

  /**
   * Constructs a new platform object.
   */
  PlatformObject();

  /**
   * Returns an object which is an instance of the given class
   * associated with this object. Returns <code>null</code> if
   * no such object can be found.
   * <p>
   * This implementation of the method declared by <code>IAdaptable</code>
   * passes the request along to the platform's adapter manager; roughly
   * <code>Platform.getAdapterManager().getAdapter(this, adapter)</code>.
   * Subclasses may override this method (however, if they do so, they
   * should invoke the method on their superclass to ensure that the
   * Platform's adapter manager is consulted).
   * </p>
   *
   * @see IAdaptable#getAdapter
   * @see Platform#getAdapterManager
   */
  Object* GetAdapter(const QString& adapter) const override;

};

}

#endif /* BERRYPLATFORMOBJECT_H_ */
