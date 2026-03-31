/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef USSERVICEREFERENCEBASEPRIVATE_H
#define USSERVICEREFERENCEBASEPRIVATE_H

#include <usAtomicInt_p.h>

#include <usServiceInterface.h>

#include <string>

namespace us {

class Any;
class Module;
class ServicePropertiesImpl;
class ServiceRegistrationBasePrivate;
class ServiceReferenceBasePrivate;


/**
 * \brief Private implementation data for ServiceReferenceBase.
 *
 * \ingroup MicroServices
 * \sa ServiceReferenceBase ServiceRegistrationBasePrivate
 */
class ServiceReferenceBasePrivate
{
public:

  /** \brief Construct from a service registration.
   *  \param[in] reg The service registration this reference points to.
   */
  ServiceReferenceBasePrivate(ServiceRegistrationBasePrivate* reg);

  /** \brief Destructor. */
  ~ServiceReferenceBasePrivate();

  /**
   * \brief Get the service object.
   *
   * \param[in] module The module requesting the service.
   * \return The service requested, or nullptr in case of failure.
   */
  void* GetService(Module* module);

  /** \brief Get the service as an InterfaceMap.
   *  \param[in] module The module requesting the service.
   *  \return The InterfaceMap for the service.
   */
  InterfaceMap GetServiceInterfaceMap(Module* module);

  /**
   * \brief Get a new prototype scope service instance.
   *
   * \param[in] module The module requesting the service.
   * \return The InterfaceMap for the new service instance, or empty on failure.
   */
  InterfaceMap GetPrototypeService(Module* module);

  /**
   * \brief Unget the service object.
   *
   * \param[in] module The module releasing the service.
   * \param[in] checkRefCounter If \c true, decrement reference counter and remove
   *            service if it reaches zero. If \c false, remove service without
   *            checking the reference counter.
   * \return \c true if the service was removed, \c false if only the reference
   *         counter was decremented.
   */
  bool UngetService(Module* module, bool checkRefCounter);

  /**
   * \brief Unget a prototype scope service object.
   *
   * \param[in] module The module releasing the prototype scope service.
   * \param[in] service The prototype scope service pointer.
   * \return \c true if the service was removed, \c false otherwise.
   */
  bool UngetPrototypeService(Module* module, void* service);

  /** \brief Unget a prototype scope service object (InterfaceMap overload).
   *  \param[in] module The module releasing the prototype scope service.
   *  \param[in] service The prototype scope service InterfaceMap.
   *  \return \c true if the service was removed, \c false otherwise.
   */
  bool UngetPrototypeService(Module* module, const InterfaceMap& service);

  /**
   * \brief Get all properties registered with this service.
   *
   * \return A ServicePropertiesImpl object containing properties, or empty
   *         if the service has been removed.
   */
  const ServicePropertiesImpl& GetProperties() const;

  /**
   * \brief Get a property value by key.
   *
   * Returns the property value to which the specified property key is mapped
   * in the properties of the service referenced by this object.
   *
   * Property keys are case-insensitive. This method continues to return
   * property values after the service has been unregistered.
   *
   * \param[in] key The property key.
   * \param[in] lock If \c true, access to the service properties will be
   *            synchronized.
   * \return The property value to which the key is mapped; an invalid Any
   *         if there is no property named after the key.
   */
  Any GetProperty(const std::string& key, bool lock) const;

  /** \brief Check if the service is convertible to a given interface.
   *  \param[in] interfaceId The interface identifier to check.
   *  \return \c true if the service can be cast to the specified interface.
   */
  bool IsConvertibleTo(const std::string& interfaceId) const;

  /**
   * Reference count for implicitly shared private implementation.
   */
  AtomicInt ref;

  /**
   * Link to registration object for this reference.
   */
  ServiceRegistrationBasePrivate* const registration;

  /**
   * The service interface id for this reference.
   */
  std::string interfaceId;

private:

  /** \brief Obtain a service instance from a ServiceFactory.
   *  \param[in] module The module requesting the service.
   *  \param[in] factory The factory to create the service from.
   *  \param[in] isModuleScope Whether the factory produces module-scoped instances.
   *  \return The InterfaceMap for the created service instance.
   */
  InterfaceMap GetServiceFromFactory(Module* module, ServiceFactory* factory,
                                     bool isModuleScope);

  // purposely not implemented
  ServiceReferenceBasePrivate(const ServiceReferenceBasePrivate&);
  ServiceReferenceBasePrivate& operator=(const ServiceReferenceBasePrivate&);
};

}

#endif // USSERVICEREFERENCEBASEPRIVATE_H
