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
     * This service has been registered.
     * <p>
     * This event is delivered <strong>after</strong> the service
     * has been registered with the framework.
     *
     * @see ModuleContext#RegisterService()
     */
    REGISTERED = 0x00000001,

    /**
     * The properties of a registered service have been modified.
     * <p>
     * This event is delivered <strong>after</strong> the service
     * properties have been modified.
     *
     * @see ServiceRegistration#SetProperties
     */
    MODIFIED = 0x00000002,

    /**
     * This service is in the process of being unregistered.
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
     * @see ServiceRegistration#Unregister
     * @see ModuleContext#UngetService
     */
    UNREGISTERING = 0x00000004,

    /**
     * The properties of a registered service have been modified and the new
     * properties no longer match the listener's filter.
     * <p>
     * This event is delivered <strong>after</strong> the service
     * properties have been modified. This event is only delivered to listeners
     * which were added with a non-empty filter where the filter
     * matched the service properties prior to the modification but the filter
     * does not match the modified service properties.
     *
     * @see ServiceRegistration#SetProperties
     */
    MODIFIED_ENDMATCH = 0x00000008

  };

  /**
   * Creates an invalid instance.
   */
  ServiceEvent();

  ~ServiceEvent();

  /**
   * Can be used to check if this ServiceEvent instance is valid,
   * or if it has been constructed using the default constructor.
   *
   * @return <code>true</code> if this event object is valid,
   *         <code>false</code> otherwise.
   */
  bool IsNull() const;

  /**
   * Creates a new service event object.
   *
   * @param type The event type.
   * @param reference A <code>ServiceReference</code> object to the service
   *        that had a lifecycle change.
   */
  ServiceEvent(Type type, const ServiceReferenceBase& reference);

  ServiceEvent(const ServiceEvent& other);

  ServiceEvent& operator=(const ServiceEvent& other);

  /**
   * Returns a reference to the service that had a change occur in its
   * lifecycle.
   * <p>
   * This reference is the source of the event.
   *
   * @return Reference to the service that had a lifecycle change.
   */
  ServiceReferenceU GetServiceReference() const;

  template<class S>
  ServiceReference<S> GetServiceReference(InterfaceType<S>) const
  {
    return GetServiceReference();
  }

  /**
   * Returns the type of event. The event type values are:
   * <ul>
   * <li>{@link #REGISTERED} </li>
   * <li>{@link #MODIFIED} </li>
   * <li>{@link #MODIFIED_ENDMATCH} </li>
   * <li>{@link #UNREGISTERING} </li>
   * </ul>
   *
   * @return Type of service lifecycle change.
   */
  Type GetType() const;

};

/**
 * \ingroup MicroServices
 * @{
 */
MITKCPPMICROSERVICES_EXPORT std::ostream& operator<<(std::ostream& os, const ServiceEvent::Type& type);
MITKCPPMICROSERVICES_EXPORT std::ostream& operator<<(std::ostream& os, const ServiceEvent& event);
/** @}*/

}

#endif // USSERVICEEVENT_H
