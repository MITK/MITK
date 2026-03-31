/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef USSERVICEREGISTRY_H
#define USSERVICEREGISTRY_H

#include <usServiceInterface.h>
#include <usServiceRegistration.h>
#include <unordered_map>
#include <unordered_set>

#include <usThreads_p.h>

namespace us {

class CoreModuleContext;
class ModulePrivate;
class ServicePropertiesImpl;


/**
 * \brief Registry for all CppMicroServices services in the framework.
 *
 * \sa CoreModuleContext ServiceRegistrationBase
 */
class ServiceRegistry
{

public:

  typedef Mutex MutexType;

  /** \brief Mutex protecting registry state. */
  mutable MutexType mutex;

  /**
   * \brief Create a ServicePropertiesImpl with keys converted to lower case.
   *
   * \param[in] in The source service properties.
   * \param[in] classes A list of class names added under the key
   *            ModuleConstants::OBJECTCLASS.
   * \param[in] isFactory Whether the service is a ServiceFactory.
   * \param[in] isPrototypeFactory Whether the service is a PrototypeServiceFactory.
   * \param[in] sid A service id to use instead of a default one.
   * \return The created ServicePropertiesImpl object.
   */
  static ServicePropertiesImpl CreateServiceProperties(const ServiceProperties& in,
                                                       const std::vector<std::string>& classes = std::vector<std::string>(),
                                                       bool isFactory = false, bool isPrototypeFactory = false, long sid = -1);

  typedef std::unordered_map<ServiceRegistrationBase, std::vector<std::string> > MapServiceClasses;
  typedef std::unordered_map<std::string, std::vector<ServiceRegistrationBase> > MapClassServices;

  /**
   * \brief All registered services in the current framework.
   *
   * Mapping of registered service to class names under which
   * the service is registered.
   */
  MapServiceClasses services;

  /** \brief Ordered list of all service registrations. */
  std::vector<ServiceRegistrationBase> serviceRegistrations;

  /**
   * \brief Mapping of class name to registered services.
   *
   * The list of registered services is ordered with the highest
   * ranked service first.
   */
  MapClassServices classServices;

  /** \brief The core module context that owns this registry. */
  CoreModuleContext* core;

  /** \brief Construct a ServiceRegistry.
   *  \param[in] coreCtx The core module context owning this registry.
   */
  ServiceRegistry(CoreModuleContext* coreCtx);

  /** \brief Destructor. */
  ~ServiceRegistry();

  /** \brief Remove all registered services. */
  void Clear();

  /**
   * \brief Register a service in the framework-wide register.
   *
   * \param[in] module The module registering the service.
   * \param[in] service The service object.
   * \param[in] properties The properties for this service.
   * \return A ServiceRegistrationBase object.
   * \throws std::invalid_argument If the service object is null or is not
   *         a ServiceFactory or an instance of all named classes.
   */
  ServiceRegistrationBase RegisterService(ModulePrivate* module,
                                          const InterfaceMap& service,
                                          const ServiceProperties& properties);

  /**
   * \brief Reorder registered services after a ranking change.
   *
   * \param[in] sr The service registration whose ranking changed.
   * \param[in] classes The class names under which the service is registered.
   */
  void UpdateServiceRegistrationOrder(const ServiceRegistrationBase& sr,
                                      const std::vector<std::string>& classes);

  /**
   * \brief Get all services implementing a certain class.
   *
   * Only used internally by the framework.
   *
   * \param[in] clazz The class name of the requested service.
   * \param[out] serviceRegs A sorted list of matching service registrations.
   */
  void Get(const std::string& clazz, std::vector<ServiceRegistrationBase>& serviceRegs) const;

  /**
   * \brief Get a service implementing a certain class.
   *
   * \param[in] module The module requesting the reference.
   * \param[in] clazz The class name of the requested service.
   * \return A ServiceReferenceBase object.
   */
  ServiceReferenceBase Get(ModulePrivate* module, const std::string& clazz) const;

  /**
   * \brief Get all services implementing a class, filtered by properties.
   *
   * \param[in] clazz The class name of the requested service.
   * \param[in] filter The LDAP property filter.
   * \param[in] module The module requesting the references.
   * \param[out] serviceRefs A list of matching ServiceReferenceBase objects.
   */
  void Get(const std::string& clazz, const std::string& filter,
           ModulePrivate* module, std::vector<ServiceReferenceBase>& serviceRefs) const;

  /**
   * \brief Remove a registered service.
   *
   * \param[in] sr The ServiceRegistrationBase object to remove.
   */
  void RemoveServiceRegistration(const ServiceRegistrationBase& sr) ;

  /**
   * \brief Get all services registered by a module.
   *
   * \param[in] m The module whose registrations to retrieve.
   * \param[out] serviceRegs The matching service registrations.
   */
  void GetRegisteredByModule(ModulePrivate* m, std::vector<ServiceRegistrationBase>& serviceRegs) const;

  /**
   * \brief Get all services used by a module.
   *
   * \param[in] m The module to check.
   * \param[out] serviceRegs The service registrations used by the module.
   */
  void GetUsedByModule(Module* m, std::vector<ServiceRegistrationBase>& serviceRegs) const;

private:

  friend class ServiceHooks;

  /** \brief Get all services implementing a class without acquiring the lock.
   *  \param[in] clazz The class name of the requested service.
   *  \param[out] serviceRegs A sorted list of matching service registrations.
   */
  void Get_unlocked(const std::string& clazz, std::vector<ServiceRegistrationBase>& serviceRegs) const;

  /** \brief Get filtered services without acquiring the lock.
   *  \param[in] clazz The class name of the requested service.
   *  \param[in] filter The LDAP property filter.
   *  \param[in] module The module requesting the references.
   *  \param[out] serviceRefs A list of matching ServiceReferenceBase objects.
   */
  void Get_unlocked(const std::string& clazz, const std::string& filter,
                    ModulePrivate* module, std::vector<ServiceReferenceBase>& serviceRefs) const;

  // purposely not implemented
  ServiceRegistry(const ServiceRegistry&);
  ServiceRegistry& operator=(const ServiceRegistry&);

};

}

#endif // USSERVICEREGISTRY_H
