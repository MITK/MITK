/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef USSERVICEEVENT_H
#define USSERVICEEVENT_H

#ifdef REGISTERED
#ifdef _WIN32
#error The REGISTERED preprocessor define clashes with the ServiceEvent::REGISTERED\
 enum type. Try to reorder your includes, compile with WIN32_LEAN_AND_MEAN, or undef\
 the REGISTERED macro before including this header.
#else
#error The REGISTERED preprocessor define clashes with the ServiceEvent::REGISTERED\
 enum type. Try to reorder your includes or undef the REGISTERED macro before including\
 this header.
#endif
#endif

#include <usSharedData.h>

#include <usServiceReference.h>

namespace us {

class ServiceEventData;

/**
 * \ingroup MicroServices
 *
 * An event from the Micro Services framework describing a service lifecycle change.
 * <p>
 * <code>ServiceEvent</code> objects are delivered to
 * listeners connected via ModuleContext::AddServiceListener() when a
 * change occurs in this service's lifecycle. A type code is used to identify
 * the event type for future extendability.
 */
class MITKCPPMICROSERVICES_EXPORT ServiceEvent
{

  SharedDataPointer<ServiceEventData> d;

public:

  enum Type {

    /**
     * \brief This service has been registered.
     * <p>
     * This event is delivered <strong>after</strong> the service
     * has been registered with the framework.
     *
     * \sa ModuleContext#RegisterService()
     */
    REGISTERED = 0x00000001,

    /**
     * \brief The properties of a registered service have been modified.
     * <p>
     * This event is delivered <strong>after</strong> the service
     * properties have been modified.
     *
     * \sa ServiceRegistration#SetProperties
     */
    MODIFIED = 0x00000002,

    /**
     * \brief This service is in the process of being unregistered.
     * <p>
     * This event is delivered <strong>before</strong> the service
     * has completed unregistering.
     *
     * <p>
     * If a module is using a service that is <code>UNREGISTERING</code>, the
     * module should release its use of the service when it receives this event.
     * If the module does not release its use of the service when it receives
     * this event, the framework will automatically release the module's use of
     * the service while completing the service unregistration operation.
     *
     * \sa ServiceRegistration#Unregister
     * \sa ModuleContext#UngetService
     */
    UNREGISTERING = 0x00000004,

    /**
     * \brief The properties of a registered service have been modified and the new
     * properties no longer match the listener's filter.
     * <p>
     * This event is delivered <strong>after</strong> the service
     * properties have been modified. This event is only delivered to listeners
     * which were added with a non-empty filter where the filter
     * matched the service properties prior to the modification but the filter
     * does not match the modified service properties.
     *
     * \sa ServiceRegistration#SetProperties
     */
    MODIFIED_ENDMATCH = 0x00000008

  };

  /**
   * \brief Creates an invalid instance.
   */
  ServiceEvent();

  /** \brief Destructor. */
  ~ServiceEvent();

  /**
   * \brief Can be used to check if this ServiceEvent instance is valid,
   * or if it has been constructed using the default constructor.
   *
   * \return <code>true</code> if this event object is valid,
   *         <code>false</code> otherwise.
   */
  bool IsNull() const;

  /**
   * \brief Creates a new service event object.
   *
   * \param[in] type The event type.
   * \param[in] reference A <code>ServiceReference</code> object to the service
   *        that had a lifecycle change.
   */
  ServiceEvent(Type type, const ServiceReferenceBase& reference);

  /** \brief Copy constructor. */
  ServiceEvent(const ServiceEvent& other);

  /** \brief Copy assignment operator. */
  ServiceEvent& operator=(const ServiceEvent& other);

  /**
   * \brief Returns a reference to the service that had a change occur in its
   * lifecycle.
   * <p>
   * This reference is the source of the event.
   * If this is a null event (default-constructed), returns an invalid
   * ServiceReference.
   *
   * \return Reference to the service that had a lifecycle change.
   */
  ServiceReferenceU GetServiceReference() const;

  /**
   * \brief Returns a typed reference to the service that had a change occur in its lifecycle.
   *
   * \tparam S The service interface type.
   * \return Typed reference to the service that had a lifecycle change.
   */
  template<class S>
  ServiceReference<S> GetServiceReference(InterfaceType<S>) const
  {
    return GetServiceReference();
  }

  /**
   * \brief Returns the type of event.
   *
   * \throws std::logic_error If this ServiceEvent is null (default-constructed).
   *
   * The event type values are:
   * <ul>
   * <li>{@link #REGISTERED} </li>
   * <li>{@link #MODIFIED} </li>
   * <li>{@link #MODIFIED_ENDMATCH} </li>
   * <li>{@link #UNREGISTERING} </li>
   * </ul>
   *
   * \return Type of service lifecycle change.
   */
  Type GetType() const;

};

/**
 * \ingroup MicroServices
 * \{
 */

/** \brief Stream output operator for ServiceEvent::Type. */
MITKCPPMICROSERVICES_EXPORT std::ostream& operator<<(std::ostream& os, const ServiceEvent::Type& type);

/** \brief Stream output operator for ServiceEvent. */
MITKCPPMICROSERVICES_EXPORT std::ostream& operator<<(std::ostream& os, const ServiceEvent& event);
/** \} */

}

#endif // USSERVICEEVENT_H
