/*=========================================================================

 Program:   openCherry Platform
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


#ifndef CHERRYPLATFORMOBJECT_H_
#define CHERRYPLATFORMOBJECT_H_

#include "cherryRuntimeDll.h"

#include <cherryObject.h>

#include "cherryIAdaptable.h"

namespace cherry {

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
class CHERRY_RUNTIME PlatformObject : public Object, public IAdaptable {

public:

  cherryObjectMacro(cherry::PlatformObject)

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
  Poco::Any GetAdapter(const std::string& adapter);

};

}

#endif /* CHERRYPLATFORMOBJECT_H_ */
