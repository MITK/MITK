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


#ifndef BERRYFILTEREDSERVICELISTENER_H_
#define BERRYFILTEREDSERVICELISTENER_H_

#include <osgi/framework/ServiceListener.h>
#include <osgi/framework/ServiceEvent.h>
#include <osgi/framework/Exceptions.h>
#include <osgi/framework/hooks/ListenerHook.h>

#include "../osgi.framework/berryFilterImpl.h"

namespace berry {
namespace osgi {
namespace internal {

using namespace ::osgi::framework;
using ::osgi::framework::hooks::ListenerHook;

class BundleContextImpl;

/**
 * Service Listener delegate.
 */
class FilteredServiceListener : public ServiceListener, public ListenerHook::ListenerInfo
{

private:

  /** Filter for listener. */
  FilterImpl::Pointer filter;
  /** Real listener. */
  const ServiceListener::Pointer listener;
  /** The bundle context */
  const SmartPointer<BundleContextImpl> context;
  /** an objectClass required by the filter */
  std::string objectClass;
  std::size_t objectClassHash;
  /** Indicates if the last event was delivered because of a filter match */
  /* @GuardedBy matchMutex */
  bool matched;
  /** indicates whether the listener has been removed */
  volatile bool removed;

  Poco::FastMutex matchMutex;

public:

  osgiObjectMacro(berry::osgi::internal::FilteredServiceListener)

  /**
   * Constructor.
   *
   * @param context The bundle context of the bundle which added the specified service listener.
   * @param filterstring The filter string specified when this service listener was added.
   * @param listener The service listener object.
   * @exception InvalidSyntaxException if the filter is invalid.
   */
  FilteredServiceListener(SmartPointer<BundleContextImpl> context, ServiceListener::Pointer listener, const std::string& filterstring) throw(InvalidSyntaxException);

  /**
   * Receives notification that a service has had a lifecycle change.
   *
   * @param event The <code>ServiceEvent</code> object.
   */
  void ServiceChanged(ServiceEvent::Pointer event);

  /**
   * The string representation of this Filtered listener.
   *
   * @return The string representation of this listener.
   */
  std::string ToString() const;

  /**
   * Return the bundle context for the ListenerHook.
   * @return The context of the bundle which added the service listener.
   * @see org.osgi.framework.hooks.service.ListenerHook.ListenerInfo#getBundleContext()
   */
  BundleContext::Pointer GetBundleContext();

  /**
   * Return the filter string for the ListenerHook.
   * @return The filter string with which the listener was added. This may
   * be <code>null</code> if the listener was added without a filter.
   * @see org.osgi.framework.hooks.service.ListenerHook.ListenerInfo#getFilter()
   */
  std::string GetFilter() const;

  /**
   * Return the state of the listener for this addition and removal life
   * cycle. Initially this method will return <code>false</code>
   * indicating the listener has been added but has not been removed.
   * After the listener has been removed, this method must always return
   * <code>true</code>.
   *
   * @return <code>false</code> if the listener has not been been removed,
   *         <code>true</code> otherwise.
   */
  bool IsRemoved() const;

  /**
   * Mark the service listener registration as removed.
   */
  void MarkRemoved();


private:

  /**
   * Returns an objectClass filter string for the specified class name.
   * @return A filter string for the specified class name or <code>null</code> if the
   * specified class name is <code>null</code>.
   */
  static std::string GetObjectClassFilterString(const std::string& className);

};

}
}
}

#endif /* BERRYFILTEREDSERVICELISTENER_H_ */

