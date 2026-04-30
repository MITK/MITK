/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef USSERVICEREGISTRATIONBASEPRIVATE_H
#define USSERVICEREGISTRATIONBASEPRIVATE_H

#include <usServiceInterface.h>
#include <usServiceReference.h>
#include "usServicePropertiesImpl_p.h"
#include <usAtomicInt_p.h>
#include <unordered_map>
#include <unordered_set>

namespace us {

class ModulePrivate;
class ServiceRegistrationBase;

/**
 * \brief Private implementation data for ServiceRegistrationBase.
 *
 * \ingroup MicroServices
 * \sa ServiceRegistrationBase ServiceReferenceBasePrivate
 */
class ServiceRegistrationBasePrivate
{

protected:

  friend class ServiceRegistrationBase;

  // The ServiceReferenceBasePrivate class holds a pointer to a
  // ServiceRegistrationBasePrivate instance and needs to manipulate
  // its reference count. This way it can keep the ServiceRegistrationBasePrivate
  // instance alive and keep returning service properties for
  // unregistered service instances.
  friend class ServiceReferenceBasePrivate;

  /**
   * Reference count for implicitly shared private implementation.
   */
  AtomicInt ref;

  /**
   * Service or ServiceFactory object.
   */
  InterfaceMap service;

public:

  typedef std::unordered_map<Module*,int> ModuleToRefsMap;
  typedef std::unordered_map<Module*, InterfaceMap> ModuleToServiceMap;
  typedef std::unordered_map<Module*, std::list<InterfaceMap> > ModuleToServicesMap;

  /**
   * Modules dependent on this service. Integer is used as
   * reference counter, counting number of unbalanced getService().
   */
  ModuleToRefsMap dependents;

  /**
   * Object instances that a prototype factory has produced.
   */
  ModuleToServicesMap prototypeServiceInstances;

  /**
   * Object instance with module scope that a factory may have produced.
   */
  ModuleToServiceMap moduleServiceInstance;

  /**
   * Module registering this service.
   */
  ModulePrivate* module;

  /**
   * Reference object to this service registration.
   */
  ServiceReferenceBase reference;

  /**
   * Service properties.
   */
  ServicePropertiesImpl properties;

  /**
   * Is service available. I.e., if <code>true</code> then holders
   * of a ServiceReference for the service are allowed to get it.
   */
  volatile bool available;

  /**
   * Avoid recursive unregistrations. I.e., if <code>true</code> then
   * unregistration of this service has started but is not yet
   * finished.
   */
  volatile bool unregistering;

  /**
   * Lock object for synchronous event delivery.
   */
  Mutex eventLock;

  // needs to be recursive
  Mutex propsLock;

  /** \brief Construct a service registration private implementation.
   *  \param[in] module The module registering the service.
   *  \param[in] service The service object or factory interface map.
   *  \param[in] props The service properties.
   */
  ServiceRegistrationBasePrivate(ModulePrivate* module, const InterfaceMap& service,
                                 const ServicePropertiesImpl& props);

  /** \brief Destructor. */
  ~ServiceRegistrationBasePrivate();

  /**
   * \brief Check if a module uses this service.
   *
   * \param[in] m The module to check.
   * \return \c true if the module uses this service.
   */
  bool IsUsedByModule(Module* m) const;

  /** \brief Get the interface map of the registered service.
   *  \return The InterfaceMap for this registration.
   */
  const InterfaceMap& GetInterfaces() const;

  /** \brief Get the service pointer for a given interface.
   *  \param[in] interfaceId The interface identifier to look up.
   *  \return The service pointer, or nullptr if not found.
   */
  void* GetService(const std::string& interfaceId) const;

private:

  // purposely not implemented
  ServiceRegistrationBasePrivate(const ServiceRegistrationBasePrivate&);
  ServiceRegistrationBasePrivate& operator=(const ServiceRegistrationBasePrivate&);

};

}


#endif // USSERVICEREGISTRATIONBASEPRIVATE_H
