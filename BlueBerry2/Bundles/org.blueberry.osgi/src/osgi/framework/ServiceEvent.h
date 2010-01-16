/*=========================================================================
 
 Program:   BlueBerry Platform
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


#ifndef SERVICEEVENT_H_
#define SERVICEEVENT_H_

#include "ServiceReference.h"

namespace osgi {
namespace framework {

/**
 * An event from the Framework describing a service lifecycle change.
 * <p>
 * <code>ServiceEvent</code> objects are delivered to
 * <code>ServiceListener</code>s and <code>AllServiceListener</code>s when a
 * change occurs in this service's lifecycle. A type code is used to identify
 * the event type for future extendability.
 *
 * <p>
 * OSGi Alliance reserves the right to extend the set of types.
 *
 * @Immutable
 * @see ServiceListener
 * @see AllServiceListener
 * @version $Revision$
 */

class ServiceEvent : public Object {

public:

  osgiObjectMacro(osgi::framework::ServiceEvent)

  enum Type {
  /**
   * This service has been registered.
   * <p>
   * This event is synchronously delivered <strong>after</strong> the service
   * has been registered with the Framework.
   *
   * @see BundleContext#registerService(String[],Object,Dictionary)
   */
   REGISTERED = 0x00000001,

  /**
   * The properties of a registered service have been modified.
   * <p>
   * This event is synchronously delivered <strong>after</strong> the service
   * properties have been modified.
   *
   * @see ServiceRegistration#setProperties
   */
    MODIFIED = 0x00000002,

  /**
   * This service is in the process of being unregistered.
   * <p>
   * This event is synchronously delivered <strong>before</strong> the service
   * has completed unregistering.
   *
   * <p>
   * If a bundle is using a service that is <code>UNREGISTERING</code>, the
   * bundle should release its use of the service when it receives this event.
   * If the bundle does not release its use of the service when it receives
   * this event, the Framework will automatically release the bundle's use of
   * the service while completing the service unregistration operation.
   *
   * @see ServiceRegistration#unregister
   * @see BundleContext#ungetService
   */
     UNREGISTERING = 0x00000004,

  /**
   * The properties of a registered service have been modified and the new
   * properties no longer match the listener's filter.
   * <p>
   * This event is synchronously delivered <strong>after</strong> the service
   * properties have been modified. This event is only delivered to listeners
   * which were added with a non-<code>null</code> filter where the filter
   * matched the service properties prior to the modification but the filter
   * does not match the modified service properties.
   *
   * @see ServiceRegistration#setProperties
   * @since 1.5
   */
    MODIFIED_ENDMATCH = 0x00000008

  };

  /**
   * Creates a new service event object.
   *
   * @param type The event type.
   * @param reference A <code>ServiceReference</code> object to the service
   *  that had a lifecycle change.
   */
  ServiceEvent(Type type, ServiceReference::Pointer reference);

  /**
   * Returns a reference to the service that had a change occur in its
   * lifecycle.
   * <p>
   * This reference is the source of the event.
   *
   * @return Reference to the service that had a lifecycle change.
   */
  ServiceReference::Pointer GetServiceReference() const;

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

private:

  /**
   * Reference to the service that had a change occur in its lifecycle.
   */
  const ServiceReference::Pointer  reference;

  /**
   * Type of service lifecycle change.
   */
  const Type type;
};

}
}

#endif /* SERVICEEVENT_H_ */
