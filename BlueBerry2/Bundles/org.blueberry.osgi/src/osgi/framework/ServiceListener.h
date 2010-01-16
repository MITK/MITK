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


#ifndef SERVICELISTENER_H_
#define SERVICELISTENER_H_

#include "ServiceEvent.h"

namespace osgi {
namespace framework {

/**
 * A <code>ServiceEvent</code> listener. <code>ServiceListener</code> is a
 * listener interface that may be implemented by a bundle developer. When a
 * <code>ServiceEvent</code> is fired, it is synchronously delivered to a
 * <code>ServiceListener</code>. The Framework may deliver
 * <code>ServiceEvent</code> objects to a <code>ServiceListener</code> out
 * of order and may concurrently call and/or reenter a
 * <code>ServiceListener</code>.
 *
 * <p>
 * A <code>ServiceListener</code> object is registered with the Framework
 * using the <code>BundleContext.addServiceListener</code> method.
 * <code>ServiceListener</code> objects are called with a
 * <code>ServiceEvent</code> object when a service is registered, modified, or
 * is in the process of unregistering.
 *
 * <p>
 * <code>ServiceEvent</code> object delivery to <code>ServiceListener</code>
 * objects is filtered by the filter specified when the listener was registered.
 * If the Java Runtime Environment supports permissions, then additional
 * filtering is done. <code>ServiceEvent</code> objects are only delivered to
 * the listener if the bundle which defines the listener object's class has the
 * appropriate <code>ServicePermission</code> to get the service using at
 * least one of the named classes under which the service was registered.
 *
 *
 * @see ServiceEvent
 * @ThreadSafe
 * @version $Revision$
 */

struct ServiceListener : public virtual Object {

  osgiInterfaceMacro(ServiceListener)

  /**
   * Receives notification that a service has had a lifecycle change.
   *
   * @param event The <code>ServiceEvent</code> object.
   */
  virtual void ServiceChanged(ServiceEvent::Pointer event) = 0;
};

}
}

#endif /* SERVICELISTENER_H_ */
