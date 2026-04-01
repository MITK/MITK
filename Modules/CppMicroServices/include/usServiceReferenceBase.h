/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef USSERVICEREFERENCEBASE_H
#define USSERVICEREFERENCEBASE_H

#include <usAny.h>

US_MSVC_PUSH_DISABLE_WARNING(4396)

namespace us {

class Module;
class ServiceRegistrationBasePrivate;
class ServiceReferenceBasePrivate;

/**
 * \ingroup MicroServices
 *
 * A reference to a service.
 *
 * \note This class is provided as public API for low-level service queries only.
 *       In almost all cases you should use the template ServiceReference instead.
 */
class MITKCPPMICROSERVICES_EXPORT ServiceReferenceBase {

private:

  typedef ServiceReferenceBasePrivate* ServiceReferenceBase::*bool_type;

public:

  /** \brief Copy constructor. */
  ServiceReferenceBase(const ServiceReferenceBase& ref);

  /**
   * \brief Converts this ServiceReferenceBase instance into a boolean
   * expression. If this instance was default constructed or
   * the service it references has been unregistered, the conversion
   * returns <code>false</code>, otherwise it returns <code>true</code>.
   */
  operator bool_type() const;

  /**
   * \brief Releases any resources held or locked by this
   * <code>ServiceReferenceBase</code> and renders it invalid.
   */
  ServiceReferenceBase& operator=(int null);

  /** \brief Destructor. */
  ~ServiceReferenceBase();

  /**
   * \brief Returns the property value to which the specified property key is mapped
   * in the properties <code>ServiceProperties</code> object of the service
   * referenced by this <code>ServiceReferenceBase</code> object.
   *
   * <p>
   * Property keys are case-insensitive.
   *
   * <p>
   * This method continues to return property values after the service has
   * been unregistered. This is so references to unregistered services can
   * still be interrogated.
   *
   * \pre This ServiceReferenceBase is valid (not default-constructed).
   *
   * \param[in] key The property key.
   * \return The property value to which the key is mapped; an invalid Any
   *         if there is no property named after the key.
   */
  Any GetProperty(const std::string& key) const;

  /**
   * \brief Returns a list of the keys in the <code>ServiceProperties</code>
   * object of the service referenced by this <code>ServiceReferenceBase</code>
   * object.
   *
   * <p>
   * This method will continue to return the keys after the service has been
   * unregistered. This is so references to unregistered services can
   * still be interrogated.
   *
   * \pre This ServiceReferenceBase is valid (not default-constructed).
   *
   * \param[out] keys A vector being filled with the property keys.
   */
  void GetPropertyKeys(std::vector<std::string>& keys) const;

  /**
   * \brief Returns the module that registered the service referenced by this
   * <code>ServiceReferenceBase</code> object.
   *
   * <p>
   * This method must return <code>0</code> when the service has been
   * unregistered. This can be used to determine if the service has been
   * unregistered.
   *
   * \return The module that registered the service referenced by this
   *         <code>ServiceReferenceBase</code> object; <code>0</code> if that
   *         service has already been unregistered.
   * \sa ModuleContext::RegisterService(const InterfaceMap&, const ServiceProperties&)
   */
  Module* GetModule() const;

  /**
   * \brief Returns the modules that are using the service referenced by this
   * <code>ServiceReferenceBase</code> object. Specifically, this method returns
   * the modules whose usage count for that service is greater than zero.
   *
   * \pre This ServiceReferenceBase is valid (not default-constructed).
   *
   * \param[out] modules A list of modules whose usage count for the service referenced
   *         by this <code>ServiceReferenceBase</code> object is greater than
   *         zero.
   */
  void GetUsingModules(std::vector<Module*>& modules) const;

  /**
   * \brief Returns the interface identifier this ServiceReferenceBase object
   * is bound to.
   *
   * A default constructed ServiceReferenceBase object is not bound to
   * any interface identifier and calling this method will return an
   * empty string.
   *
   * \return The interface identifier for this ServiceReferenceBase object.
   */
  std::string GetInterfaceId() const;

  /**
   * \brief Checks whether this ServiceReferenceBase object can be converted to
   * another ServiceReferenceBase object, which will be bound to the
   * given interface identifier.
   *
   * ServiceReferenceBase objects can be converted if the underlying service
   * implementation was registered under multiple service interfaces.
   *
   * \param[in] interfaceid The target interface identifier.
   * \return \c true if this ServiceReferenceBase object can be converted,
   *         \c false otherwise.
   */
  bool IsConvertibleTo(const std::string& interfaceid) const;

  /**
   * \brief Compares this <code>ServiceReferenceBase</code> with the specified
   * <code>ServiceReferenceBase</code> for order.
   *
   * <p>
   * If this <code>ServiceReferenceBase</code> and the specified
   * <code>ServiceReferenceBase</code> have the same \link ServiceConstants::SERVICE_ID()
   * service id\endlink they are equal. This <code>ServiceReferenceBase</code> is less
   * than the specified <code>ServiceReferenceBase</code> if it has a lower
   * {@link ServiceConstants::SERVICE_RANKING service ranking} and greater if it has a
   * higher service ranking. Otherwise, if this <code>ServiceReferenceBase</code>
   * and the specified <code>ServiceReferenceBase</code> have the same
   * {@link ServiceConstants::SERVICE_RANKING service ranking}, this
   * <code>ServiceReferenceBase</code> is less than the specified
   * <code>ServiceReferenceBase</code> if it has a higher
   * {@link ServiceConstants::SERVICE_ID service id} and greater if it has a lower
   * service id.
   *
   * \param[in] reference The <code>ServiceReferenceBase</code> to be compared.
   * \return Returns a false or true if this
   *         <code>ServiceReferenceBase</code> is less than or greater
   *         than the specified <code>ServiceReferenceBase</code>.
   */
  bool operator<(const ServiceReferenceBase& reference) const;

  /** \brief Equality comparison operator. */
  bool operator==(const ServiceReferenceBase& reference) const;

  /** \brief Equality comparison with nullptr. */
  bool operator==(std::nullptr_t) const noexcept;

  /** \brief Copy assignment operator. */
  ServiceReferenceBase& operator=(const ServiceReferenceBase& reference);

private:

  friend class ModulePrivate;
  friend class ModuleContext;
  friend class ModuleHooks;
  friend class ServiceHooks;
  friend class ServiceObjectsBase;
  friend class ServiceObjectsBasePrivate;
  friend class ServiceRegistrationBase;
  friend class ServiceRegistrationBasePrivate;
  friend class ServiceListeners;
  friend class ServiceRegistry;
  friend class LDAPFilter;

  template<class S> friend class ServiceReference;

  friend struct std::hash<ServiceReferenceBase>;

  std::size_t Hash() const;

  /**
   * Creates an invalid ServiceReferenceBase object. You can use
   * this object in boolean expressions and it will evaluate to
   * <code>false</code>.
   */
  ServiceReferenceBase();

  ServiceReferenceBase(ServiceRegistrationBasePrivate* reg);

  void SetInterfaceId(const std::string& interfaceId);

  ServiceReferenceBasePrivate* d;

};

}

US_MSVC_POP_WARNING

/**
 * \ingroup MicroServices
 *
 * \brief Stream output operator for ServiceReferenceBase.
 */
MITKCPPMICROSERVICES_EXPORT std::ostream& operator<<(std::ostream& os, const us::ServiceReferenceBase& serviceRef);

namespace std {
template<> struct hash<us::ServiceReferenceBase> { std::size_t operator()(const us::ServiceReferenceBase& arg) const {
  return arg.Hash();
} };
}

#endif // USSERVICEREFERENCEBASE_H
