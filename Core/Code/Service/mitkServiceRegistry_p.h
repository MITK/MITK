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


#ifndef MITKSERVICEREGISTRY_H
#define MITKSERVICEREGISTRY_H

#include <list>

#include <itkSimpleFastMutexLock.h>

#include "mitkServiceRegistration.h"
#include "mitkServiceProperties.h"
#include "mitkServiceUtils.h"

namespace mitk {

class CoreModuleContext;
class ModulePrivate;

struct HashServiceRegistration
{
  std::size_t operator()(const ServiceRegistration& s) const;
};

/**
 * Here we handle all the MITK services that are registered.
 */
class ServiceRegistry
{

public:

  typedef itk::SimpleFastMutexLock MutexType;

  mutable MutexType mutex;

  /**
   * Creates a new ServiceProperties object containing <code>in</code>
   * with the keys converted to lower case.
   *
   * @param classes A list of class names which will be added to the
   *        created ServiceProperties object under the key
   *        ModuleConstants::OBJECTCLASS.
   * @param sid A service id which will be used instead of a default one.
   */
  static ServiceProperties CreateServiceProperties(const ServiceProperties& in,
                                                   const std::list<std::string>& classes = std::list<std::string>(),
                                                   long sid = -1);

#ifdef MITK_HAS_UNORDERED_MAP_H
  typedef std::unordered_map<ServiceRegistration, std::list<std::string>, HashServiceRegistration> MapServiceClasses;
  typedef std::unordered_map<std::string, std::list<ServiceRegistration> > MapClassServices;
#else
  typedef itk::hash_map<ServiceRegistration, std::list<std::string>, HashServiceRegistration> MapServiceClasses;
  typedef itk::hash_map<std::string, std::list<ServiceRegistration> > MapClassServices;
#endif

  /**
   * All registered services in the current framework.
   * Mapping of registered service to class names under which
   * the service is registerd.
   */
  MapServiceClasses services;

  std::list<ServiceRegistration> serviceRegistrations;

  /**
   * Mapping of classname to registered service.
   * The List of registered services are ordered with the highest
   * ranked service first.
   */
  MapClassServices classServices;

  CoreModuleContext* core;

  ServiceRegistry(CoreModuleContext* coreCtx);

  ~ServiceRegistry();

  void Clear();

  /**
   * Register a service in the framework wide register.
   *
   * @param module The module registering the service.
   * @param classes The class names under which the service can be located.
   * @param service The service object.
   * @param properties The properties for this service.
   * @return A ServiceRegistration object.
   * @exception std::invalid_argument If one of the following is true:
   * <ul>
   * <li>The service object is 0.</li>
   * <li>The service parameter is not a ServiceFactory or an
   * instance of all the named classes in the classes parameter.</li>
   * </ul>
   */
  ServiceRegistration RegisterService(ModulePrivate* module,
                                      const std::list<std::string>& clazzes,
                                      itk::LightObject* service,
                                      const ServiceProperties& properties);

  /**
   * Service ranking changed, reorder registered services
   * according to ranking.
   *
   * @param serviceRegistration The ServiceRegistrationPrivate object.
   * @param rank New rank of object.
   */
  void UpdateServiceRegistrationOrder(const ServiceRegistration& sr,
                                      const std::list<std::string>& classes);

  /**
   * Checks that a given service object is an instance of the given
   * class name.
   *
   * @param service The service object to check.
   * @param cls     The class name to check for.
   */
  bool CheckServiceClass(itk::LightObject* service, const std::string& cls) const;

  /**
   * Get all services implementing a certain class.
   * Only used internally by the framework.
   *
   * @param clazz The class name of the requested service.
   * @return A sorted list of {@link ServiceRegistrationPrivate} objects.
   */
  void Get(const std::string& clazz, std::list<ServiceRegistration>& serviceRegs) const;

  /**
   * Get a service implementing a certain class.
   *
   * @param module The module requesting reference
   * @param clazz The class name of the requested service.
   * @return A {@link ServiceReference} object.
   */
  ServiceReference Get(ModulePrivate* module, const std::string& clazz) const;

  /**
   * Get all services implementing a certain class and then
   * filter these with a property filter.
   *
   * @param clazz The class name of requested service.
   * @param filter The property filter.
   * @param module The module requesting reference.
   * @return A list of {@link ServiceReference} object.
   */
  void Get(const std::string& clazz, const std::string& filter,
           ModulePrivate* module, std::list<ServiceReference>& serviceRefs) const;

  /**
   * Remove a registered service.
   *
   * @param sr The ServiceRegistration object that is registered.
   */
  void RemoveServiceRegistration(const ServiceRegistration& sr) ;

  /**
   * Get all services that a module has registered.
   *
   * @param p The module
   * @return A set of {@link ServiceRegistration} objects
   */
  void GetRegisteredByModule(ModulePrivate* m, std::list<ServiceRegistration>& serviceRegs) const;

  /**
   * Get all services that a module uses.
   *
   * @param p The module
   * @return A set of {@link ServiceRegistration} objects
   */
  void GetUsedByModule(Module* m, std::list<ServiceRegistration>& serviceRegs) const;

private:

  void Get_unlocked(const std::string& clazz, const std::string& filter,
                    ModulePrivate* module, std::list<ServiceReference>& serviceRefs) const;

};

}

#endif // MITKSERVICEREGISTRY_H
