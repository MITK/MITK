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


#ifndef MITKSERVICEREGISTRATIONPRIVATE_H
#define MITKSERVICEREGISTRATIONPRIVATE_H

#include "mitkModule.h"
#include "mitkServiceReference.h"
#include "mitkServiceProperties.h"
#include "mitkAtomicInt.h"

#include <itkMutexLockHolder.h>

namespace mitk {

class ModulePrivate;
class ServiceRegistration;

/**
 * \ingroup MicroServices
 */
class ServiceRegistrationPrivate
{

protected:

  friend class ServiceRegistration;

  /**
   * Reference count for implicitly shared private implementation.
   */
  AtomicInt ref;

  /**
   * Service or ServiceFactory object.
   */
  itk::LightObject* service;

public:

  typedef itk::SimpleFastMutexLock MutexType;
  typedef itk::MutexLockHolder<MutexType> MutexLocker;

#ifdef MITK_HAS_UNORDERED_MAP_H
  typedef std::unordered_map<Module*,int> ModuleToRefsMap;
  typedef std::unordered_map<Module*, itk::LightObject*> ModuleToServicesMap;
#else
  typedef itk::hash_map<Module*,int> ModuleToRefsMap;
  typedef itk::hash_map<Module*, itk::LightObject*> ModuleToServicesMap;
#endif

  /**
   * Module registering this service.
   */
  ModulePrivate* module;

  /**
   * Reference object to this service registration.
   */
  ServiceReference reference;

  /**
   * Service properties.
   */
  ServiceProperties properties;

  /**
   * Modules dependent on this service. Integer is used as
   * reference counter, counting number of unbalanced getService().
   */
  ModuleToRefsMap dependents;

  /**
   * Object instances that factory has produced.
   */
  ModuleToServicesMap serviceInstances;

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
  MutexType eventLock;

  // needs to be recursive
  MutexType propsLock;

  ServiceRegistrationPrivate(ModulePrivate* module, itk::LightObject* service,
                             const ServiceProperties& props);

  virtual ~ServiceRegistrationPrivate();

  /**
   * Check if a module uses this service
   *
   * @param p Module to check
   * @return true if module uses this service
   */
  bool IsUsedByModule(Module* m);

  virtual itk::LightObject* GetService();

private:

  // disable copy constructor and assignment operator
  ServiceRegistrationPrivate(const ServiceRegistrationPrivate&);
  ServiceRegistrationPrivate& operator=(const ServiceRegistrationPrivate&);

};

}


#endif // MITKSERVICEREGISTRATIONPRIVATE_H
