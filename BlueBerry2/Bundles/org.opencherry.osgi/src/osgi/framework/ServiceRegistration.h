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


#ifndef SERVICEREGISTRATION_H_
#define SERVICEREGISTRATION_H_

#include "Object.h"
#include "Macros.h"

#include "ServiceReference.h"

namespace osgi {
namespace util {
  struct Dictionary;
}
}

namespace osgi {
namespace framework {

using namespace ::osgi::util;

/**
 * A registered service.
 *
 * <p>
 * The Framework returns a <code>ServiceRegistration</code> object when a
 * <code>BundleContext.registerService</code> method invocation is successful.
 * The <code>ServiceRegistration</code> object is for the private use of the
 * registering bundle and should not be shared with other bundles.
 * <p>
 * The <code>ServiceRegistration</code> object may be used to update the
 * properties of the service or to unregister the service.
 *
 * @see BundleContext#registerService(String[],Object,Dictionary)
 * @ThreadSafe
 * @version $Revision$
 */

struct ServiceRegistration : public Object {

  osgiInterfaceMacro(osgi::framework::ServiceRegistration)

  /**
   * Returns a <code>ServiceReference</code> object for a service being
   * registered.
   * <p>
   * The <code>ServiceReference</code> object may be shared with other
   * bundles.
   *
   * @throws IllegalStateException If this
   *         <code>ServiceRegistration</code> object has already been
   *         unregistered.
   * @return <code>ServiceReference</code> object.
   */
  virtual ServiceReference::Pointer GetReference() const throw(IllegalStateException) = 0;

  /**
   * Updates the properties associated with a service.
   *
   * <p>
   * The {@link Constants#OBJECTCLASS} and {@link Constants#SERVICE_ID} keys
   * cannot be modified by this method. These values are set by the Framework
   * when the service is registered in the OSGi environment.
   *
   * <p>
   * The following steps are required to modify service properties:
   * <ol>
   * <li>The service's properties are replaced with the provided properties.
   * <li>A service event of type {@link ServiceEvent#MODIFIED} is fired.
   * </ol>
   *
   * @param properties The properties for this service. See {@link Constants}
   *        for a list of standard service property keys. Changes should not
   *        be made to this object after calling this method. To update the
   *        service's properties this method should be called again.
   *
   * @throws IllegalStateException If this <code>ServiceRegistration</code>
   *         object has already been unregistered.
   * @throws IllegalArgumentException If <code>properties</code> contains
   *         case variants of the same key name.
   */
  virtual void SetProperties(const Dictionary& properties) throw(IllegalStateException, IllegalArgumentException) = 0;

  /**
   * Unregisters a service. Remove a <code>ServiceRegistration</code> object
   * from the Framework service registry. All <code>ServiceReference</code>
   * objects associated with this <code>ServiceRegistration</code> object
   * can no longer be used to interact with the service once unregistration is
   * complete.
   *
   * <p>
   * The following steps are required to unregister a service:
   * <ol>
   * <li>The service is removed from the Framework service registry so that
   * it can no longer be obtained.
   * <li>A service event of type {@link ServiceEvent#UNREGISTERING} is fired
   * so that bundles using this service can release their use of the service.
   * Once delivery of the service event is complete, the
   * <code>ServiceReference</code> objects for the service may no longer be
   * used to get a service object for the service.
   * <li>For each bundle whose use count for this service is greater than
   * zero: <br>
   * The bundle's use count for this service is set to zero. <br>
   * If the service was registered with a {@link ServiceFactory} object, the
   * <code>ServiceFactory#UngetService</code> method is called to release
   * the service object for the bundle.
   * </ol>
   *
   * @throws IllegalStateException If this
   *         <code>ServiceRegistration</code> object has already been
   *         unregistered.
   * @see BundleContext#UngetService
   * @see ServiceFactory#UngetService
   */
  virtual void UnregisterService() throw(IllegalStateException) = 0;
};

}
}

#endif /* SERVICEREGISTRATION_H_ */
