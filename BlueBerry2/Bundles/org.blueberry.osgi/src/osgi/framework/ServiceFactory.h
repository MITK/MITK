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


#ifndef SERVICEFACTORY_H_
#define SERVICEFACTORY_H_

#include "Bundle.h"
#include "ServiceRegistration.h"

namespace osgi {
namespace framework {

/**
 * Allows services to provide customized service objects in the OSGi
 * environment.
 *
 * <p>
 * When registering a service, a <code>ServiceFactory</code> object can be
 * used instead of a service object, so that the bundle developer can gain
 * control of the specific service object granted to a bundle that is using the
 * service.
 *
 * <p>
 * When this happens, the
 * <code>BundleContext::GetService(ServiceReference::Pointer)</code> method calls the
 * <code>ServiceFactory::GetService</code> method to create a service object
 * specifically for the requesting bundle. The service object returned by the
 * <code>ServiceFactory</code> is cached by the Framework until the bundle
 * releases its use of the service.
 *
 * <p>
 * When the bundle's use count for the service equals zero (including the bundle
 * stopping or the service being unregistered), the
 * <code>ServiceFactory::UngetService</code> method is called.
 *
 * <p>
 * <code>ServiceFactory</code> objects are only used by the Framework and are
 * not made available to other bundles in the OSGi environment. The Framework
 * may concurrently call a <code>ServiceFactory</code>.
 *
 * @see BundleContext#GetService
 * @ThreadSafe
 */

struct ServiceFactory : public virtual Object {

  osgiObjectMacro(osgi::framework::ServiceFactory)

  /**
   * Creates a new service object.
   *
   * <p>
   * The Framework invokes this method the first time the specified
   * <code>bundle</code> requests a service object using the
   * <code>BundleContext.getService(ServiceReference)</code> method. The
   * service factory can then return a specific service object for each
   * bundle.
   *
   * <p>
   * The Framework caches the value returned (unless it is <code>null</code>),
   * and will return the same service object on any future call to
   * <code>BundleContext.getService</code> for the same bundle. This means the
   * Framework must not allow this method to be concurrently called for the
   * same bundle.
   *
   * <p>
   * The Framework will check if the returned service object is an instance of
   * all the classes named when the service was registered. If not, then
   * <code>null</code> is returned to the bundle.
   *
   * @param bundle The bundle using the service.
   * @param registration The <code>ServiceRegistration</code> object for the
   *        service.
   * @return A service object that <strong>must</strong> be an instance of all
   *         the classes named when the service was registered.
   * @see BundleContext#getService
   */
  virtual Object::Pointer GetService(Bundle::Pointer bundle, ServiceRegistration::Pointer registration) = 0;

  /**
   * Releases a service object.
   *
   * <p>
   * The Framework invokes this method when a service has been released by a
   * bundle. The service object may then be destroyed.
   *
   * @param bundle The bundle releasing the service.
   * @param registration The <code>ServiceRegistration</code> object for the
   *        service.
   * @param service The service object returned by a previous call to the
   *        <code>ServiceFactory.getService</code> method.
   * @see BundleContext#ungetService
   */
  virtual void UngetService(Bundle::Pointer bundle, ServiceRegistration::Pointer registration,
      Object::Pointer service) = 0;
};

}
}

#endif /* SERVICEFACTORY_H_ */
