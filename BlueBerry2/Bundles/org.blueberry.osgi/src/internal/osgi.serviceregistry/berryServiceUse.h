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


#ifndef BERRYSERVICEUSE_H_
#define BERRYSERVICEUSE_H_

#include <osgi/framework/Object.h>
#include <osgi/framework/Macros.h>
#include <osgi/framework/ServiceFactory.h>

namespace berry {
namespace osgi {
namespace internal {

using namespace ::osgi::framework;

class BundleContextImpl;
class ServiceRegistrationImpl;

/**
 * This class represents the use of a service by a bundle. One is created for each
 * service acquired by a bundle. This class manages the calls to ServiceFactory
 * and the bundle's use count.
 *
 * @ThreadSafe
 */

class ServiceUse : public Object
{

private:

  /** ServiceFactory object if the service instance represents a factory,
   null otherwise */
  SmartPointer<ServiceFactory> factory;
  /** BundleContext associated with this service use */
  const SmartPointer<BundleContextImpl> context;
  /** ServiceDescription of the registered service */
  const SmartPointer<ServiceRegistrationImpl> registration;

  /** Service object either registered or that returned by
   ServiceFactory#GetService() */
  /* @GuardedBy("this") */
  Object::Pointer cachedService;
  /** bundle's use count for this service */
  /* @GuardedBy("this") */
  int useCount;

  /** Internal framework object. */

public:

  osgiObjectMacro(berry::osgi::internal::ServiceUse)

  /**
   * Constructs a service use encapsulating the service object.
   * Objects of this class should be constructed while holding the
   * registrations lock.
   *
   * @param   context bundle getting the service
   * @param   registration ServiceRegistration of the service
   */
  ServiceUse(SmartPointer<BundleContextImpl> context,
      SmartPointer<ServiceRegistrationImpl> registration);

  /**
   * Get a service's service object.
   * Retrieves the service object for a service.
   * A bundle's use of a service is tracked by a
   * use count. Each time a service's service object is returned by
   * {@link #getService}, the context bundle's use count for the service
   * is incremented by one. Each time the service is release by
   * {@link #ungetService}, the context bundle's use count
   * for the service is decremented by one.
   * When a bundle's use count for a service
   * drops to zero, the bundle should no longer use the service.
   *
   * <p>The following steps are followed to get the service object:
   * <ol>
   * <li>The context bundle's use count for this service is incremented by one.
   * <li>If the context bundle's use count for the service is now one and
   * the service was registered with a {@link ServiceFactory},
   * the {@link ServiceFactory#getService ServiceFactory.getService} method
   * is called to create a service object for the context bundle.
   * This service object is cached by the framework.
   * While the context bundle's use count for the service is greater than zero,
   * subsequent calls to get the services's service object for the context bundle
   * will return the cached service object.
   * <br>If the service object returned by the {@link ServiceFactory}
   * is not an <code>instanceof</code>
   * all the classes named when the service was registered or
   * the {@link ServiceFactory} throws an exception,
   * <code>null</code> is returned and a
   * {@link FrameworkEvent} of type {@link FrameworkEvent#ERROR} is broadcast.
   * <li>The service object for the service is returned.
   * </ol>
   *
   * @return A service object for the service associated with this
   * reference.
   */
  /* @GuardedBy("this") */
  Object::Pointer GetService();

  /**
   * Unget a service's service object.
   * Releases the service object for a service.
   * If the context bundle's use count for the service is zero, this method
   * returns <code>false</code>. Otherwise, the context bundle's use count for the
   * service is decremented by one.
   *
   * <p>The service's service object
   * should no longer be used and all references to it should be destroyed
   * when a bundle's use count for the service
   * drops to zero.
   *
   * <p>The following steps are followed to unget the service object:
   * <ol>
   * <li>If the context bundle's use count for the service is zero or
   * the service has been unregistered,
   * <code>false</code> is returned.
   * <li>The context bundle's use count for this service is decremented by one.
   * <li>If the context bundle's use count for the service is now zero and
   * the service was registered with a {@link ServiceFactory},
   * the {@link ServiceFactory#ungetService ServiceFactory.ungetService} method
   * is called to release the service object for the context bundle.
   * <li><code>true</code> is returned.
   * </ol>
   *
   * @return <code>true</code> if the context bundle's use count for the service
   *         is zero otherwise <code>false</code>.
   */
  /* @GuardedBy("this") */
  bool UngetService();

  /**
   * Release a service's service object.
   * <ol>
   * <li>The bundle's use count for this service is set to zero.
   * <li>If the service was registered with a {@link ServiceFactory},
   * the {@link ServiceFactory#ungetService ServiceFactory.ungetService} method
   * is called to release the service object for the bundle.
   * </ol>
   */
  /* @GuardedBy("this") */
  void ReleaseService();
};

}
}
}

#endif /* BERRYSERVICEUSE_H_ */
