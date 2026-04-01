/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef USPROTOTYPESERVICEFACTORY_H
#define USPROTOTYPESERVICEFACTORY_H

#include <usServiceFactory.h>

namespace us {

/**
 * \ingroup MicroServices
 *
 * \brief A factory for \link ServiceConstants::SCOPE_PROTOTYPE prototype scope\endlink services.
 * The factory can provide multiple, unique service objects.
 *
 * When registering a service, a PrototypeServiceFactory object can be used
 * instead of a service object, so that the module developer can create a
 * unique service object for each caller that is using the service.
 * When a caller uses a ServiceObjects to request a service instance, the
 * framework calls the GetService method to return a service object specifically
 * for the requesting caller. The caller can release the returned service object
 * and the framework will call the UngetService method with the service object.
 * When a module uses the ModuleContext::GetService(const ServiceReferenceBase&)
 * method to obtain a service object, the framework acts as if the service
 * has module scope. That is, the framework will call the GetService method to
 * obtain a module-scoped instance which will be cached and have a use count.
 * See ServiceFactory.
 *
 * A module can use both ServiceObjects and ModuleContext::GetService(const ServiceReferenceBase&)
 * to obtain a service object for a service. ServiceObjects::GetService() will always
 * return an instance provided by a call to GetService(Module*, const ServiceRegistrationBase&)
 * and ModuleContext::GetService(const ServiceReferenceBase&) will always
 * return the module-scoped instance.
 * PrototypeServiceFactory objects are only used by the framework and are not made
 * available to other modules. The framework may concurrently call a PrototypeServiceFactory.
 *
 * \sa ModuleContext::GetServiceObjects()
 * \sa ServiceObjects
 */
struct PrototypeServiceFactory : public ServiceFactory
{

  /**
   * \brief Returns a service object for a caller.
   *
   * The framework invokes this method for each caller requesting a service object using
   * ServiceObjects::GetService(). The factory can then return a specific service object for the caller.
   * The framework checks that the returned service object is valid. If the returned service
   * object is empty or does not contain entries for all the interfaces named when the service
   * was registered, a warning is issued and nullptr is returned to the caller. If this
   * method throws an exception, a warning is issued and nullptr is returned to the caller.
   *
   * \param[in] module The module requesting the service.
   * \param[in] registration The ServiceRegistrationBase object for the requested service.
   * \return A service object that must contain entries for all the interfaces named when
   *         the service was registered.
   *
   * \sa ServiceObjects#GetService()
   * \sa InterfaceMap
   */
  InterfaceMap GetService(Module* module, const ServiceRegistrationBase& registration) override = 0;

  /**
   * \brief Releases a service object created for a caller.
   *
   * The framework invokes this method when a service has been released by a modules such as
   * by calling ServiceObjects::UngetService(). The service object may then be destroyed.
   * If this method throws an exception, a warning is issued.
   *
   * \param[in] module The module releasing the service.
   * \param[in] registration The ServiceRegistrationBase object for the service being released.
   * \param[in] service The service object returned by a previous call to the GetService method.
   *
   * \sa ServiceObjects::UngetService()
   */
  void UngetService(Module* module, const ServiceRegistrationBase& registration,
                            const InterfaceMap& service) override = 0;

};

}

#endif // USPROTOTYPESERVICEFACTORY_H
