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

#ifndef _CHERRY_IADAPTABLE_H_
#define _CHERRY_IADAPTABLE_H_

#include "cherryRuntimeDll.h"

#include <cherryMacros.h>
#include <cherryObject.h>

#include <Poco/Any.h>

namespace cherry {

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
struct CHERRY_RUNTIME IAdaptable {

public:

  cherryNameMacro(cherry::IAdaptable)

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
  virtual Poco::Any GetAdapter(const std::string& adapterType) = 0;

  virtual ~IAdaptable();

};

}  // namespace cherry

#endif /*_CHERRY_IADAPTABLE_H_*/
