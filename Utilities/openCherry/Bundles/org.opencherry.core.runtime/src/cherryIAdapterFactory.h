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

#ifndef CHERRYIADAPTERFACTORY_H_
#define CHERRYIADAPTERFACTORY_H_

#include "cherryRuntimeDll.h"

#include <vector>
#include <typeinfo>

namespace cherry {

/**
 * An adapter factory defines behavioral extensions for
 * one or more classes that implements the <code>IAdaptable</code>
 * interface. Adapter factories are registered with an
 * adapter manager.
 * <p>
 * This interface can be used without OSGi running.
 * </p><p>
 * Clients may implement this interface.
 * </p>
 * @see IAdapterManager
 * @see IAdaptable
 */
struct CHERRY_RUNTIME IAdapterFactory {

  virtual ~IAdapterFactory() {};
  
  /**
   * Returns an object which is an instance of the given class
   * associated with the given object. Returns <code>null</code> if
   * no such object can be found.
   *
   * @param adaptableObject the adaptable object being queried
   *   (usually an instance of <code>IAdaptable</code>)
   * @param adapterType the type of adapter to look up
   * @return a object castable to the given adapter type, 
   *    or <code>null</code> if this adapter factory 
   *    does not have an adapter of the given type for the
   *    given object
   */
  virtual void* GetAdapter(void* adaptableObject, const std::type_info& adapterType) = 0;

  /**
   * Returns the collection of adapter types handled by this
   * factory.
   * <p>
   * This method is generally used by an adapter manager
   * to discover which adapter types are supported, in advance
   * of dispatching any actual <code>getAdapter</code> requests.
   * </p>
   *
   * @return the collection of adapter types
   */
  virtual void GetAdapterList(std::vector<const std::type_info&>& adapters) = 0;
};

}
#endif /*CHERRYIADAPTERFACTORY_H_*/
