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


#ifndef BERRYISERVICELOCATOR_H_
#define BERRYISERVICELOCATOR_H_

#include "../berryUiDll.h"

#include <berryObject.h>
#include <berryMacros.h>

namespace berry {

/**
 * <p>
 * A component with which one or more services are registered. The services can
 * be retrieved from this locator using some key -- typically the class
 * representing the interface the service must implement. For example:
 * </p>
 *
 * <pre>
 * IHandlerService service = (IHandlerService) workbenchWindow
 *    .getService(IHandlerService.class);
 * </pre>
 *
 * <p>
 * This interface is not to be implemented or extended by clients.
 * </p>
 *
 * @since 3.2
 */
struct BERRY_UI IServiceLocator : public virtual Object {

  berryInterfaceMacro(IServiceLocator, berry)

  /**
   * Retrieves the service corresponding to the given API.
   *
   * @param api
   *            This is the interface that the service implements. Must not be
   *            <code>null</code>.
   * @return The service, or <code>null</code> if no such service could be
   *         found.
   */
  virtual Object::Pointer GetService(const std::string& api) = 0;

  /**
   * Whether this service exists within the scope of this service locator.
   * This does not include looking for the service within the scope of the
   * parents. This method can be used to determine whether a particular
   * service supports nesting in this scope.
   *
   * @param api
   *            This is the interface that the service implements. Must not be
   *            <code>null</code>.
   * @return <code>true</code> iff the service locator can find a service
   *         for the given API; <code>false</code> otherwise.
   */
  virtual bool HasService(const std::string& api) const = 0;

};

}

#endif /* BERRYISERVICELOCATOR_H_ */
