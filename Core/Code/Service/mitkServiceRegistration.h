/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#ifndef MITKSERVICEREGISTRATION_H
#define MITKSERVICEREGISTRATION_H

#include "mitkServiceProperties.h"
#include "mitkServiceReference.h"

namespace mitk {

class ModulePrivate;

/**
 * \ingroup MicroServices
 *
 * A registered service.
 *
 * <p>
 * The framework returns a <code>ServiceRegistration</code> object when a
 * <code>ModuleContext#RegisterService()</code> method invocation is successful.
 * The <code>ServiceRegistration</code> object is for the private use of the
 * registering module and should not be shared with other modules.
 * <p>
 * The <code>ServiceRegistration</code> object may be used to update the
 * properties of the service or to unregister the service.
 *
 * @see ModuleContext#RegisterService()
 * @remarks This class is thread safe.
 */
class MITK_CORE_EXPORT ServiceRegistration {

public:

  /**
   * Creates an invalid ServiceRegistration object. You can use
   * this object in boolean expressions and it will evaluate to
   * <code>false</code>.
   */
  ServiceRegistration();

  ServiceRegistration(const ServiceRegistration& reg);

  operator bool() const;

  /**
   * Releases any resources held or locked by this
   * <code>ServiceRegistration</code> and renders it invalid.
   */
  ServiceRegistration& operator=(int null);

  ~ServiceRegistration();

  /**
   * Returns a <code>ServiceReference</code> object for a service being
   * registered.
   * <p>
   * The <code>ServiceReference</code> object may be shared with other
   * modules.
   *
   * @throws std::logic_error If this
   *         <code>ServiceRegistration</code> object has already been
   *         unregistered or if it is invalid.
   * @return <code>ServiceReference</code> object.
   */
  ServiceReference GetReference() const;

  /**
   * Updates the properties associated with a service.
   *
   * <p>
   * The {@link ServiceProperties#OBJECTCLASS} and {@link ServiceProperties#SERVICE_ID} keys
   * cannot be modified by this method. These values are set by the framework
   * when the service is registered in the environment.
   *
   * <p>
   * The following steps are taken to modify service properties:
   * <ol>
   * <li>The service's properties are replaced with the provided properties.
   * <li>A service event of type {@link ServiceEvent#MODIFIED} is fired.
   * </ol>
   *
   * @param properties The properties for this service. See {@link ServiceProperties}
   *        for a list of standard service property keys. Changes should not
   *        be made to this object after calling this method. To update the
   *        service's properties this method should be called again.
   *
   * @throws std::logic_error If this <code>ServiceRegistration</code>
   *         object has already been unregistered or if it is invalid.
   * @throws std::invalid_argument If <code>properties</code> contains
   *         case variants of the same key name.
   */
  void SetProperties(const ServiceProperties& properties);

  /**
   * Unregisters a service. Remove a <code>ServiceRegistration</code> object
   * from the framework service registry. All <code>ServiceRegistration</code>
   * objects associated with this <code>ServiceRegistration</code> object
   * can no longer be used to interact with the service once unregistration is
   * complete.
   *
   * <p>
   * The following steps are taken to unregister a service:
   * <ol>
   * <li>The service is removed from the framework service registry so that
   * it can no longer be obtained.
   * <li>A service event of type {@link ServiceEvent#UNREGISTERING} is fired
   * so that modules using this service can release their use of the service.
   * Once delivery of the service event is complete, the
   * <code>ServiceRegistration</code> objects for the service may no longer be
   * used to get a service object for the service.
   * <li>For each module whose use count for this service is greater than
   * zero: <br>
   * The module's use count for this service is set to zero. <br>
   * If the service was registered with a {@link ServiceFactory} object, the
   * <code>ServiceFactory#UngetService</code> method is called to release
   * the service object for the module.
   * </ol>
   *
   * @throws std::logic_error If this
   *         <code>ServiceRegistration</code> object has already been
   *         unregistered or if it is invalid.
   * @see ModuleContext#UngetService
   * @see ServiceFactory#UngetService
   */
  virtual void Unregister();

  bool operator<(const ServiceRegistration& o) const;

  bool operator==(const ServiceRegistration& registration) const;

  ServiceRegistration& operator=(const ServiceRegistration& registration);


protected:

  friend class ServiceRegistry;
  friend class ServiceReferencePrivate;
  friend struct HashServiceRegistration;

  ServiceRegistration(ServiceRegistrationPrivate* registrationPrivate);

  ServiceRegistration(ModulePrivate* module, itk::LightObject* service,
                      const ServiceProperties& props);

  ServiceRegistrationPrivate* d;

};

}

inline std::ostream& operator<<(std::ostream& os, const mitk::ServiceRegistration& /*reg*/)
{
  return os << "mitk::ServiceRegistration object";
}

#endif // MITKSERVICEREGISTRATION_H
