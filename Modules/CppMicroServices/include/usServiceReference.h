/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef USSERVICEREFERENCE_H
#define USSERVICEREFERENCE_H

#include <usServiceInterface.h>
#include <usServiceReferenceBase.h>

namespace us {

/**
 * \ingroup MicroServices
 *
 * A reference to a service.
 *
 * <p>
 * The framework returns <code>ServiceReference</code> objects from the
 * <code>ModuleContext::GetServiceReference</code> and
 * <code>ModuleContext::GetServiceReferences</code> methods.
 * <p>
 * A <code>ServiceReference</code> object may be shared between modules and
 * can be used to examine the properties of the service and to get the service
 * object.
 * <p>
 * Every service registered in the framework has a unique
 * <code>ServiceRegistration</code> object and may have multiple, distinct
 * <code>ServiceReference</code> objects referring to it.
 * <code>ServiceReference</code> objects associated with a
 * <code>ServiceRegistration</code> are considered equal
 * (more specifically, their <code>operator==()</code>
 * method will return <code>true</code> when compared).
 * <p>
 * If the same service object is registered multiple times,
 * <code>ServiceReference</code> objects associated with different
 * <code>ServiceRegistration</code> objects are not equal.
 *
 * \tparam S The class type of the service interface
 * \sa ModuleContext::GetServiceReference
 * \sa ModuleContext::GetServiceReferences
 * \sa ModuleContext::GetService
 * \remarks This class is thread safe.
 */
template<class S>
class ServiceReference : public ServiceReferenceBase {

public:

  /** \brief The service interface type. */
  typedef S ServiceType;

  /**
   * \brief Creates an invalid ServiceReference object. You can use
   * this object in boolean expressions and it will evaluate to
   * <code>false</code>.
   */
  ServiceReference() : ServiceReferenceBase()
  {
  }

  /**
   * \brief Constructs a ServiceReference from a ServiceReferenceBase.
   *
   * If the base reference is not convertible to this service interface type,
   * the resulting ServiceReference will be invalid.
   *
   * \param[in] base The base service reference to convert.
   */
  ServiceReference(const ServiceReferenceBase& base)
    : ServiceReferenceBase(base)
  {
    const std::string interfaceId(us_service_interface_iid<S>());
    if (GetInterfaceId() != interfaceId)
    {
      if (this->IsConvertibleTo(interfaceId))
      {
        this->SetInterfaceId(interfaceId);
      }
      else
      {
        this->operator =(0);
      }
    }
  }

  ServiceReference(const ServiceReference&) = default;
  ServiceReference& operator=(const ServiceReference&) = default;

  using ServiceReferenceBase::operator=;

};

/**
 * \cond
 *
 * Specialization for void, representing a generic service
 * reference not bound to any interface identifier.
 *
 */
template<>
class ServiceReference<void> : public ServiceReferenceBase
{

public:

  /**
   * \brief Creates an invalid ServiceReference object. You can use
   * this object in boolean expressions and it will evaluate to
   * <code>false</code>.
   */
  ServiceReference() : ServiceReferenceBase()
  {
  }

  /**
   * \brief Constructs a ServiceReference from a ServiceReferenceBase.
   *
   * \param[in] base The base service reference to convert.
   */
  ServiceReference(const ServiceReferenceBase& base)
    : ServiceReferenceBase(base)
  {
  }

  ServiceReference(const ServiceReference&) = default;
  ServiceReference& operator=(const ServiceReference&) = default;

  using ServiceReferenceBase::operator=;

  /** \brief The service interface type (void for untyped references). */
  typedef void ServiceType;
};
/// \endcond

/**
 * \ingroup MicroServices
 *
 * A service reference of unknown type, which is not bound to any
 * interface identifier.
 */
typedef ServiceReference<void> ServiceReferenceU;

}

#endif // USSERVICEREFERENCE_H
