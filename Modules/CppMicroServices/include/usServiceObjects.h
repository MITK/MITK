/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef USSERVICEOBJECTS_H
#define USSERVICEOBJECTS_H

#include <usGlobalConfig.h>
#include <MitkCppMicroServicesExports.h>

#include <usModuleContext.h>
#include <usPrototypeServiceFactory.h>
#include <usServiceReference.h>
#include <usServiceProperties.h>

namespace us {

class ServiceObjectsBasePrivate;

/**
 * \ingroup MicroServices
 *
 * \brief Base class for ServiceObjects providing type-erased service object management.
 *
 * This class is not intended to be used directly. Use ServiceObjects instead.
 *
 * \sa ServiceObjects
 */
class MITKCPPMICROSERVICES_EXPORT ServiceObjectsBase
{

private:

  ServiceObjectsBasePrivate* d;

protected:

  /** \brief Constructs a ServiceObjectsBase instance.
   *
   * \param[in] context The module context.
   * \param[in] reference The service reference.
   */
  ServiceObjectsBase(ModuleContext* context, const ServiceReferenceBase& reference);

  /** \brief Copy constructor. */
  ServiceObjectsBase(const ServiceObjectsBase& other);

  /** \brief Destructor. */
  ~ServiceObjectsBase();

  /** \brief Copy assignment operator. */
  ServiceObjectsBase& operator=(const ServiceObjectsBase& other);

  /** \brief Called by ServiceObjects\<S\> with S != void to get a service object. */
  void* GetService() const;

  /** \brief Called by the ServiceObjects<void> specialization to get a service InterfaceMap. */
  InterfaceMap GetServiceInterfaceMap() const;

  /** \brief Called by ServiceObjects\<S\> with S != void to release a service object. */
  void UngetService(void* service);

  /** \brief Called by the ServiceObjects<void> specialization to release a service InterfaceMap. */
  void UngetService(const InterfaceMap& interfaceMap);

  /** \brief Returns the ServiceReferenceBase for this ServiceObjects object. */
  ServiceReferenceBase GetReference() const;

};

/**
 * \ingroup MicroServices
 *
 * \brief Allows multiple service objects for a service to be obtained.
 *
 * For services with \link ServiceConstants::SCOPE_PROTOTYPE prototype\endlink scope,
 * multiple service objects for the service can be obtained. For services with
 * \link ServiceConstants::SCOPE_SINGLETON singleton\endlink or
 * \link ServiceConstants::SCOPE_MODULE module \endlink scope, only one, use-counted
 * service object is available. Any unreleased service objects obtained from this
 * ServiceObjects object are automatically released by the framework when the modules
 * associated with the ModuleContext used to create this ServiceObjects object is
 * stopped.
 *
 * \tparam S Type of Service.
 */
template<class S>
class ServiceObjects : private ServiceObjectsBase
{

public:

  /**
   * \brief Returns a service object for the referenced service.
   *
   * This ServiceObjects object can be used to obtain multiple service objects for
   * the referenced service if the service has \link ServiceConstants::SCOPE_PROTOTYPE prototype\endlink
   * scope. If the referenced service has \link ServiceConstants::SCOPE_SINGLETON singleton\endlink
   * or \link ServiceConstants::SCOPE_MODULE module\endlink scope, this method
   * behaves the same as calling the ModuleContext::GetService(const ServiceReferenceBase&)
   * method for the referenced service. That is, only one, use-counted service object
   * is available from this ServiceObjects object.
   *
   * This method will always return \c nullptr when the referenced service has been unregistered.
   *
   * For a prototype scope service, the following steps are taken to get the service object:
   *
   * <ol>
   *   <li>If the referenced service has been unregistered, \c nullptr is returned.</li>
   *   <li>The PrototypeServiceFactory::GetService(Module*, const ServiceRegistrationBase&)
   *       method is called to create a service object for the caller.</li>
   *   <li>If the service object (an instance of InterfaceMap) returned by the
   *       PrototypeServiceFactory object is empty, does not contain all the interfaces
   *       named when the service was registered or the PrototypeServiceFactory object
   *       throws an exception, \c nullptr is returned and a warning message is issued.</li>
   *   <li>The service object is returned.</li>
   * </ol>
   *
   * \return A service object for the referenced service or \c nullptr if the service is not
   *         registered, the service object returned by a ServiceFactory does not contain
   *         all the classes under which it was registered or the ServiceFactory threw an
   *         exception.
   *
   * \throws std::logic_error If the ModuleContext used to create this ServiceObjects object
   *        is no longer valid.
   *
   * \sa UngetService()
   */
  S* GetService() const
  {
    return reinterpret_cast<S*>(this->ServiceObjectsBase::GetService());
  }

  /**
   * \brief Releases a service object for the referenced service.
   *
   * This ServiceObjects object can be used to obtain multiple service objects for
   * the referenced service if the service has \link ServiceConstants::SCOPE_PROTOTYPE prototype\endlink
   * scope. If the referenced service has \link ServiceConstants::SCOPE_SINGLETON singleton\endlink
   * or \link ServiceConstants::SCOPE_MODULE module\endlink scope, this method
   * behaves the same as calling the ModuleContext::UngetService(const ServiceReferenceBase&)
   * method for the referenced service. That is, only one, use-counted service object
   * is available from this ServiceObjects object.
   *
   * For a prototype scope service, the following steps are take to release the service object:
   *
   * <ol>
   *   <li>If the referenced service has been unregistered, this method returns without
   *       doing anything.</li>
   *   <li>The PrototypeServiceFactory::UngetService(Module*, const ServiceRegistrationBase&, const InterfaceMap&)
   *       method is called to release the specified service object.</li>
   *   <li>The specified service object must no longer be used and all references to it
   *       should be destroyed after calling this method.</li>
   * </ol>
   *
   * \param[in] service A service object previously provided by this ServiceObjects object.
   *
   * \throws std::logic_error If the ModuleContext used to create this ServiceObjects
   *        object is no longer valid.
   * \throws std::invalid_argument If the specified service was not provided by this
   *        ServiceObjects object.
   *
   * \sa GetService()
   */
  void UngetService(S* service)
  {
    this->ServiceObjectsBase::UngetService(service);
  }

  /**
   * \brief Returns the ServiceReference for this ServiceObjects object.
   *
   * \return The ServiceReference for this ServiceObjects object.
   */
  ServiceReference<S> GetServiceReference() const
  {
    return this->ServiceObjectsBase::GetReference();
  }

private:

  friend class ModuleContext;

  ServiceObjects(ModuleContext* context, const ServiceReference<S>& reference)
    : ServiceObjectsBase(context, reference)
  {}

};

/**
 * \ingroup MicroServices
 *
 * \brief Allows multiple service objects for a service to be obtained.
 *
 * This is a specialization of the ServiceObjects class template for
 * "void", which maps to all service interface types.
 *
 * \sa ServiceObjects
 */
template<>
class MITKCPPMICROSERVICES_EXPORT ServiceObjects<void> : private ServiceObjectsBase
{

public:

  /**
   * \brief Returns a service object as a InterfaceMap instance for the referenced service.
   *
   * This method is the same as ServiceObjects\<S\>::GetService() except for the
   * return type. Further, this method will always return an empty InterfaeMap
   * object when the referenced service has been unregistered.
   *
   * \return A InterfaceMap object for the referenced service, which is empty if the
   *         service is not registered, the InterfaceMap returned by a ServiceFactory
   *         does not contain all the classes under which the service object was
   *         registered or the ServiceFactory threw an exception.
   *
   * \throws std::logic_error If the ModuleContext used to create this ServiceObjects object
   *        is no longer valid.
   *
   * \sa ServiceObjects\<S\>::GetService()
   * \sa UngetService()
   */
  InterfaceMap GetService() const;

  /**
   * \brief Releases a service object for the referenced service.
   *
   * This method is the same as ServiceObjects\<S\>::UngetService() except for the
   * parameter type.
   *
   * \param[in] service An InterfaceMap object previously provided by this ServiceObjects object.
   *
   * \throws std::logic_error If the ModuleContext used to create this ServiceObjects
   *        object is no longer valid.
   * \throws std::invalid_argument If the specified service was not provided by this
   *        ServiceObjects object.
   *
   * \sa ServiceObjects\<S\>::UngetService()
   * \sa GetService()
   */
  void UngetService(const InterfaceMap& service);

  /**
   * \brief Returns the ServiceReference for this ServiceObjects object.
   *
   * \return The ServiceReference for this ServiceObjects object.
   */
  ServiceReferenceU GetServiceReference() const;

private:

  friend class ModuleContext;

  ServiceObjects(ModuleContext* context, const ServiceReferenceU& reference);

};

}

#endif // USSERVICEOBJECTS_H
