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

#ifndef MITKSERVICEEVENT_H
#define MITKSERVICEEVENT_H

#include "mitkSharedData.h"

#include "mitkServiceReference.h"

namespace mitk {

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
class MITK_CORE_EXPORT ServiceEvent
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
  ServiceEvent(Type type, const ServiceReference& reference);

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
  ServiceReference GetServiceReference() const;

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

}

/**
 * \ingroup MicroServices
 * @{
 */
MITK_CORE_EXPORT std::ostream& operator<<(std::ostream& os, const mitk::ServiceEvent::Type& type);
MITK_CORE_EXPORT std::ostream& operator<<(std::ostream& os, const mitk::ServiceEvent& event);
/** @}*/

#endif // MITKSERVICEEVENT_H
