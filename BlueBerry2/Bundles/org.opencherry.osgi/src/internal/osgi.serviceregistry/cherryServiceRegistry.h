/*=========================================================================
 
 Program:   openCherry Platform
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


#ifndef CHERRYSERVICEREGISTRY_H_
#define CHERRYSERVICEREGISTRY_H_

#include <osgi/framework/Object.h>
#include <osgi/framework/Macros.h>

#include <osgi/framework/Filter.h>
#include <osgi/framework/ServiceEvent.h>
#include <osgi/util/Dictionary.h>

#include <osgi/framework/ServiceListener.h>
#include "../osgi.framework/eventmgr/cherryCopyOnWriteMap.h"

#include <Poco/HashMap.h>
#include <set>

namespace cherry {
namespace osgi {
namespace internal {

class ServiceReferenceImpl;
class ServiceRegistrationImpl;
class FilteredServiceListener;
class Framework;
class BundleContextImpl;

using namespace ::osgi::framework;

/**
 * The Service Registry. This class is the main control point for service
 * layer operations in the framework.
 *
 * @ThreadSafe
 */
class ServiceRegistry : public Object {

public:

  osgiObjectMacro(cherry::osgi::internal::ServiceRegistry)

private:

  //static const std::string findHookName = FindHook.class.getName();
  //static const std::string eventHookName = EventHook.class.getName();
  //static const std::string listenerHookName = ListenerHook.class.getName();

  Poco::Mutex mutex;
  Poco::Mutex servicesInUseLock;

  /** Published services by class name.
   * Poco::HashMap&lt;std::string,std::set&lt;ServiceRegistrationImpl::Pointer&gt;&gt;
   * The std::set&lt;ServiceRegistrationImpl::Pointer&gt;s are sorted.
   */
  /* @GuardedBy("this") */
  Poco::HashMap<std::string, std::set<SmartPointer<ServiceRegistrationImpl> > > publishedServicesByClass;

  /** All published services.
   * std::set&lt;ServiceRegistrationImpl::Pointer&gt;.
   * The std::set&lt;ServiceRegistrationImpl::Pointer&gt;s are sorted.
   */
  /* @GuardedBy("this") */
  std::set<SmartPointer<ServiceRegistrationImpl> > allPublishedServices;

  typedef Poco::HashMap<SmartPointer<BundleContextImpl>, std::set<SmartPointer<ServiceRegistrationImpl> >, Object::Hash > ServicesByContextMap;

  /** Published services by BundleContextImpl.
   * Map&lt;BundleContextImpl,List&lt;ServiceRegistrationImpl&gt;&gt;.
   * The List&lt;ServiceRegistrationImpl&gt;s are NOT sorted.
   */
  /* @GuardedBy("this") */
  ServicesByContextMap publishedServicesByContext;

  typedef CopyOnWriteMap<SmartPointer<ServiceListener>, SmartPointer<FilteredServiceListener> > ServiceListenerMap;

  /** Active Service Listeners.
   * Map&lt;BundleContextImpl,CopyOnWriteIdentityMap&lt;ServiceListener,FilteredServiceListener&gt;&gt;.
   */
  /* @GuardedBy("serviceEventListenersLock") */
  Poco::HashMap<SmartPointer<BundleContextImpl>, ServiceListenerMap, Object::Hash> serviceEventListeners;

  Poco::Mutex serviceEventListenersLock;

   /** next free service id. */
  /* @GuardedBy("this") */
  long serviceid;

  /** initial capacity of the main data structure */
  static const int initialCapacity; // = 50;
  /** initial capacity of the nested data structure */
  static const int initialSubCapacity; // = 10;
  /** framework which created this service registry */
  const SmartPointer<Framework> framework;


  public:

    static const int SERVICEEVENT; // = 3;

  /**
   * Initializes the internal data structures of this ServiceRegistry.
   *
   */
   ServiceRegistry(SmartPointer<Framework> framework);

  /**
   * Registers the specified service object with the specified properties
   * under the specified class names into the Framework. A
   * <code>ServiceRegistrationImpl</code> object is returned. The
   * <code>ServiceRegistrationImpl</code> object is for the private use of the
   * bundle registering the service and should not be shared with other
   * bundles. The registering bundle is defined to be the context bundle.
   * Other bundles can locate the service by using either the
   * {@link #getServiceReferences} or {@link #getServiceReference} method.
   *
   * <p>
   * A bundle can register a service object that implements the
   * {@link ServiceFactory} interface to have more flexibility in providing
   * service objects to other bundles.
   *
   * <p>
   * The following steps are required to register a service:
   * <ol>
   * <li>If <code>service</code> is not a <code>ServiceFactory</code>,
   * an <code>IllegalArgumentException</code> is thrown if
   * <code>service</code> is not an <code>instanceof</code> all the
   * classes named.
   * <li>The Framework adds these service properties to the specified
   * <code>Dictionary</code> (which may be <code>null</code>): a property
   * named {@link Constants#SERVICE_ID} identifying the registration number of
   * the service and a property named {@link Constants#OBJECTCLASS} containing
   * all the specified classes. If any of these properties have already been
   * specified by the registering bundle, their values will be overwritten by
   * the Framework.
   * <li>The service is added to the Framework service registry and may now
   * be used by other bundles.
   * <li>A service event of type {@link ServiceEvent#REGISTERED} is fired.
   * <li>A <code>ServiceRegistration</code> object for this registration is
   * returned.
   * </ol>
   *
   * @param context The BundleContext of the registering bundle.
   * @param clazzes The class names under which the service can be located.
   *        The class names in this array will be stored in the service's
   *        properties under the key {@link Constants#OBJECTCLASS}.
   * @param service The service object or a <code>ServiceFactory</code>
   *        object.
   * @param properties The properties for this service. The keys in the
   *        properties object must all be <code>String</code> objects. See
   *        {@link Constants} for a list of standard service property keys.
   *        Changes should not be made to this object after calling this
   *        method. To update the service's properties the
   *        {@link ServiceRegistration#setProperties} method must be called.
   *        The set of properties may be <code>null</code> if the service
   *        has no properties.
   *
   * @return A <code>ServiceRegistrationImpl</code> object for use by the bundle
   *         registering the service to update the service's properties or to
   *         unregister the service.
   *
   * @throws java.lang.IllegalArgumentException If one of the following is
   *         true:
   *         <ul>
   *         <li><code>service</code> is <code>null</code>.
   *         <li><code>service</code> is not a <code>ServiceFactory</code>
   *         object and is not an instance of all the named classes in
   *         <code>clazzes</code>.
   *         <li><code>properties</code> contains case variants of the same
   *         key name.
   *         </ul>
   *
   * @throws java.lang.IllegalStateException If this BundleContext is no
   *         longer valid.
   *
   * @see ServiceRegistration
   * @see ServiceFactory
   */
  SmartPointer<ServiceRegistrationImpl> RegisterService(
      SmartPointer<BundleContextImpl> context,
      const std::vector<std::string>& clazzes, Object::Pointer service,
      const SmartPointer<Dictionary> properties) throw(IllegalArgumentException, IllegalStateException);

  /**
   * Returns an array of <code>ServiceReferenceImpl</code> objects. The returned
   * array of <code>ServiceReferenceImpl</code> objects contains services that
   * were registered under the specified class, match the specified filter
   * criteria, and the packages for the class names under which the services
   * were registered match the context bundle's packages as defined in
   * {@link ServiceReference#isAssignableTo(Bundle, String)}.
   *
   * <p>
   * The list is valid at the time of the call to this method, however since
   * the Framework is a very dynamic environment, services can be modified or
   * unregistered at anytime.
   *
   * <p>
   * <code>filter</code> is used to select the registered service whose
   * properties objects contain keys and values which satisfy the filter. See
   * {@link Filter} for a description of the filter string syntax.
   *
   * <p>
   * If <code>filter</code> is <code>null</code>, all registered services
   * are considered to match the filter. If <code>filter</code> cannot be
   * parsed, an {@link InvalidSyntaxException} will be thrown with a human
   * readable message where the filter became unparsable.
   *
   * <p>
   * The following steps are required to select a set of
   * <code>ServiceReferenceImpl</code> objects:
   * <ol>
   * <li>If the filter string is not <code>null</code>, the filter string
   * is parsed and the set <code>ServiceReferenceImpl</code> objects of
   * registered services that satisfy the filter is produced. If the filter
   * string is <code>null</code>, then all registered services are
   * considered to satisfy the filter.
   * <li>If the Java Runtime Environment supports permissions, the set of
   * <code>ServiceReferenceImpl</code> objects produced by the previous step is
   * reduced by checking that the caller has the
   * <code>ServicePermission</code> to get at least one of the class names
   * under which the service was registered. If the caller does not have the
   * correct permission for a particular <code>ServiceReferenceImpl</code>
   * object, then it is removed from the set.
   * <li>If <code>clazz</code> is not <code>null</code>, the set is
   * further reduced to those services that are an <code>instanceof</code>
   * and were registered under the specified class. The complete list of
   * classes of which a service is an instance and which were specified when
   * the service was registered is available from the service's
   * {@link Constants#OBJECTCLASS} property.
   * <li>The set is reduced one final time by cycling through each
   * <code>ServiceReference</code> object and calling
   * {@link ServiceReference#isAssignableTo(Bundle, String)} with the context
   * bundle and each class name under which the <code>ServiceReference</code>
   * object was registered. For any given <code>ServiceReferenceImpl</code>
   * object, if any call to
   * {@link ServiceReference#isAssignableTo(Bundle, String)} returns
   * <code>false</code>, then it is removed from the set of
   * <code>ServiceReferenceImpl</code> objects.
   * <li>An array of the remaining <code>ServiceReferenceImpl</code> objects is
   * returned.
   * </ol>
   *
   * @param context The BundleContext of the requesting bundle.
   * @param clazz The class name with which the service was registered or
   *        <code>null</code> for all services.
   * @param filterstring The filter criteria.
   * @param allservices True if the bundle called getAllServiceReferences.
   * @return An array of <code>ServiceReferenceImpl</code> objects or
   *         <code>null</code> if no services are registered which satisfy
   *         the search.
   * @throws InvalidSyntaxException If <code>filter</code> contains an
   *         invalid filter string that cannot be parsed.
   * @throws java.lang.IllegalStateException If this BundleContext is no
   *         longer valid.
   */
  std::vector<SmartPointer<ServiceReferenceImpl> > GetServiceReferences(
      SmartPointer<BundleContextImpl> context, const std::string& clazz,
      const std::string& filterstring, bool allservices) throw(InvalidSyntaxException);

  /**
   * Returns a <code>ServiceReference</code> object for a service that
   * implements and was registered under the specified class.
   *
   * <p>
   * This <code>ServiceReference</code> object is valid at the time of the
   * call to this method, however as the Framework is a very dynamic
   * environment, services can be modified or unregistered at anytime.
   *
   * <p>
   * This method is the same as calling
   * {@link BundleContext#getServiceReferences(String, String)} with a
   * <code>null</code> filter string. It is provided as a convenience for
   * when the caller is interested in any service that implements the
   * specified class.
   * <p>
   * If multiple such services exist, the service with the highest ranking (as
   * specified in its {@link Constants#SERVICE_RANKING} property) is returned.
   * <p>
   * If there is a tie in ranking, the service with the lowest service ID (as
   * specified in its {@link Constants#SERVICE_ID} property); that is, the
   * service that was registered first is returned.
   *
   * @param context The BundleContext of the requesting bundle.
   * @param clazz The class name with which the service was registered.
   * @return A <code>ServiceReference</code> object, or <code>null</code>
   *         if no services are registered which implement the named class.
   * @throws java.lang.IllegalStateException If this BundleContext is no
   *         longer valid.
   */
  SmartPointer<ServiceReferenceImpl> GetServiceReference(
      SmartPointer<BundleContextImpl> context, const std::string& clazz);

  /**
   * Returns the specified service object for a service.
   * <p>
   * A bundle's use of a service is tracked by the bundle's use count of that
   * service. Each time a service's service object is returned by
   * {@link #getService(ServiceReference)} the context bundle's use count for
   * that service is incremented by one. Each time the service is released by
   * {@link #ungetService(ServiceReference)} the context bundle's use count
   * for that service is decremented by one.
   * <p>
   * When a bundle's use count for a service drops to zero, the bundle should
   * no longer use that service.
   *
   * <p>
   * This method will always return <code>null</code> when the service
   * associated with this <code>reference</code> has been unregistered.
   *
   * <p>
   * The following steps are required to get the service object:
   * <ol>
   * <li>If the service has been unregistered, <code>null</code> is
   * returned.
   * <li>The context bundle's use count for this service is incremented by
   * one.
   * <li>If the context bundle's use count for the service is currently one
   * and the service was registered with an object implementing the
   * <code>ServiceFactory</code> interface, the
   * {@link ServiceFactory#getService(Bundle, ServiceRegistration)} method is
   * called to create a service object for the context bundle. This service
   * object is cached by the Framework. While the context bundle's use count
   * for the service is greater than zero, subsequent calls to get the
   * services's service object for the context bundle will return the cached
   * service object. <br>
   * If the service object returned by the <code>ServiceFactory</code>
   * object is not an <code>instanceof</code> all the classes named when the
   * service was registered or the <code>ServiceFactory</code> object throws
   * an exception, <code>null</code> is returned and a Framework event of
   * type {@link FrameworkEvent#ERROR} containing a {@link ServiceException}
   * describing the error is fired.
   * <li>The service object for the service is returned.
   * </ol>
   *
   * @param context The BundleContext of the requesting bundle.
   * @param reference A reference to the service.
   * @return A service object for the service associated with
   *         <code>reference</code> or <code>null</code> if the service is
   *         not registered, the service object returned by a
   *         <code>ServiceFactory</code> does not implement the classes
   *         under which it was registered or the <code>ServiceFactory</code>
   *         threw an exception.
   * @throws java.lang.SecurityException If the caller does not have the
   *         <code>ServicePermission</code> to get the service using at
   *         least one of the named classes the service was registered under
   *         and the Java Runtime Environment supports permissions.
   * @throws java.lang.IllegalStateException If this BundleContext is no
   *         longer valid.
   * @see #ungetService(ServiceReference)
   * @see ServiceFactory
   */
  Object::Pointer GetService(SmartPointer<BundleContextImpl> context,
      SmartPointer<ServiceReferenceImpl> reference);

  /**
   * Releases the service object referenced by the specified
   * <code>ServiceReference</code> object. If the context bundle's use count
   * for the service is zero, this method returns <code>false</code>.
   * Otherwise, the context bundle's use count for the service is decremented
   * by one.
   *
   * <p>
   * The service's service object should no longer be used and all references
   * to it should be destroyed when a bundle's use count for the service drops
   * to zero.
   *
   * <p>
   * The following steps are required to unget the service object:
   * <ol>
   * <li>If the context bundle's use count for the service is zero or the
   * service has been unregistered, <code>false</code> is returned.
   * <li>The context bundle's use count for this service is decremented by
   * one.
   * <li>If the context bundle's use count for the service is currently zero
   * and the service was registered with a <code>ServiceFactory</code>
   * object, the
   * {@link ServiceFactory#ungetService(Bundle, ServiceRegistration, Object)}
   * method is called to release the service object for the context bundle.
   * <li><code>true</code> is returned.
   * </ol>
   *
   * @param context The BundleContext of the requesting bundle.
   * @param reference A reference to the service to be released.
   * @return <code>false</code> if the context bundle's use count for the
   *         service is zero or if the service has been unregistered;
   *         <code>true</code> otherwise.
   * @throws java.lang.IllegalStateException If this BundleContext is no
   *         longer valid.
   * @see #getService
   * @see ServiceFactory
   */
  bool UngetService(SmartPointer<BundleContextImpl> context,
      SmartPointer<ServiceReferenceImpl> reference);

  /**
   * Returns this bundle's <code>ServiceReference</code> list for all
   * services it has registered or <code>null</code> if this bundle has no
   * registered services.
   *
   * <p>
   * If the Java runtime supports permissions, a <code>ServiceReference</code>
   * object to a service is included in the returned list only if the caller
   * has the <code>ServicePermission</code> to get the service using at
   * least one of the named classes the service was registered under.
   *
   * <p>
   * The list is valid at the time of the call to this method, however, as the
   * Framework is a very dynamic environment, services can be modified or
   * unregistered at anytime.
   *
   * @param context The BundleContext of the requesting bundle.
   * @return An array of <code>ServiceReference</code> objects or
   *         <code>null</code>.
   * @throws java.lang.IllegalStateException If this bundle has been
   *         uninstalled.
   * @see ServiceRegistration
   * @see ServiceReference
   * @see ServicePermission
   */
  std::vector<SmartPointer<ServiceReferenceImpl> > GetRegisteredServices(
      SmartPointer<BundleContextImpl> context);

  /**
   * Returns this bundle's <code>ServiceReference</code> list for all
   * services it is using or returns <code>null</code> if this bundle is not
   * using any services. A bundle is considered to be using a service if its
   * use count for that service is greater than zero.
   *
   * <p>
   * If the Java Runtime Environment supports permissions, a
   * <code>ServiceReference</code> object to a service is included in the
   * returned list only if the caller has the <code>ServicePermission</code>
   * to get the service using at least one of the named classes the service
   * was registered under.
   * <p>
   * The list is valid at the time of the call to this method, however, as the
   * Framework is a very dynamic environment, services can be modified or
   * unregistered at anytime.
   *
   * @param context The BundleContext of the requesting bundle.
   * @return An array of <code>ServiceReference</code> objects or
   *         <code>null</code>.
   * @throws java.lang.IllegalStateException If this bundle has been
   *         uninstalled.
   * @see ServiceReference
   * @see ServicePermission
   */
  std::vector<SmartPointer<ServiceReferenceImpl> > GetServicesInUse(
      SmartPointer<BundleContextImpl> context);

  /**
   * Called when the BundleContext is closing to unregister all services
   * currently registered by the bundle.
   *
   * @param context The BundleContext of the closing bundle.
   */
  void UnregisterServices(SmartPointer<BundleContextImpl> context);

  /**
   * Called when the BundleContext is closing to unget all services
   * currently used by the bundle.
   *
   * @param context The BundleContext of the closing bundle.
   */
  void ReleaseServicesInUse(SmartPointer<BundleContextImpl> context);

  /**
   * Add a new Service Listener for a bundle.
   *
   * @param context Context of bundle adding listener.
   * @param listener Service Listener to be added.
   * @param filter Filter string for listener or null.
   * @throws InvalidSyntaxException If the filter string is invalid.
   */
  void AddServiceListener(SmartPointer<BundleContextImpl> context,
      SmartPointer<ServiceListener> listener, const std::string& filter)
  throw(InvalidSyntaxException);

  /**
   * Remove a Service Listener for a bundle.
   *
   * @param context Context of bundle removing listener.
   * @param listener Service Listener to be removed.
   */
  void RemoveServiceListener(SmartPointer<BundleContextImpl> context,
      SmartPointer<ServiceListener> listener);

  /**
   * Remove all Service Listener for a bundle.
   *
   * @param context Context of bundle removing all listeners.
   */
  void RemoveAllServiceListeners(SmartPointer<BundleContextImpl> context);

  /**
   * Deliver a ServiceEvent.
   *
   * @param type ServiceEvent type.
   * @param reference Affected service reference.
   */
  void PublishServiceEvent(ServiceEvent::Type type, SmartPointer<ServiceReferenceImpl> reference);

  /**
   * Return the next available service id.
   *
   * @return next service id.
   */
  long GetNextServiceId();

  /**
   * Add the ServiceRegistrationImpl to the data structure.
   *
   * @param context The BundleContext of the bundle registering the service.
   * @param registration The new ServiceRegistration.
   */
  /* @GuardedBy("this") */
  void AddServiceRegistration(SmartPointer<BundleContextImpl> context,
      SmartPointer<ServiceRegistrationImpl> registration);

  /**
   * Remove the ServiceRegistrationImpl from the data structure.
   *
   * @param context The BundleContext of the bundle registering the service.
   * @param registration The ServiceRegistration to remove.
   */
  /* @GuardedBy("this") */
  void RemoveServiceRegistration(
      SmartPointer<BundleContextImpl> context,
      SmartPointer<ServiceRegistrationImpl> serviceReg);

  /**
   * Return the name of the class that is not satisfied by the service object.
   * @param clazzes Array of class names.
   * @param serviceObject Service object.
   * @return The name of the class that is not satisfied by the service object.
   */
  static std::string CheckServiceClass(const std::vector<std::string>& clazzes,
      const Object::Pointer serviceObject);

private:

   /**
   * Lookup Service Registrations in the data structure by class name and filter.
   *
   * @param clazz The class name with which the service was registered or
   *        <code>null</code> for all services.
   * @param filter The filter criteria.
   * @return List<ServiceRegistrationImpl>
   */
  std::vector<SmartPointer<ServiceRegistrationImpl> >
  LookupServiceRegistrations(const std::string& clazz, SmartPointer<Filter> filter);

  /**
   * Lookup Service Registrations in the data structure by BundleContext.
   *
   * @param context The BundleContext for which to return Service Registrations.
   * @return List<ServiceRegistrationImpl>
   */
  std::vector<SmartPointer<ServiceRegistrationImpl> >
  LookupServiceRegistrations(SmartPointer<BundleContextImpl> context);

  /**
   * Convert a std::vector<ServiceRegistrationImpl::Pointer> to a
   * std::vector<ServiceReferenceImpl::Pointer>.
   *
   * @param input The input std::vector<ServiceRegistrationImpl::Pointer>
   * @param result The converted std::vector<ServiceReferenceImpl::Pointer>
   */
  static void
  ChangeRegistrationsToReferences(const std::vector<SmartPointer<ServiceRegistrationImpl> >& input, std::vector<SmartPointer<ServiceReferenceImpl> >& result);


  /**
   * Call the registered FindHook services to allow them to inspect and possibly shrink the result.
   * The FindHook must be called in order: descending by service.ranking, then ascending by service.id.
   * This is the natural order for ServiceReference.
   *
   * @param context The context of the bundle getting the service references.
   * @param clazz The class name used to search for the service references.
   * @param filterstring The filter used to search for the service references.
   * @param allservices True if getAllServiceReferences called.
   * @param result The result to return to the caller which may have been shrunk by the FindHooks.
   */
//  private void notifyFindHooks(final BundleContextImpl context, final String clazz, final String filterstring, final boolean allservices, final Collection result) {
//    if (System.getSecurityManager() == null) {
//      notifyFindHooksPrivileged(context, clazz, filterstring, allservices, result);
//    } else {
//      AccessController.doPrivileged(new PrivilegedAction() {
//        public Object run() {
//          notifyFindHooksPrivileged(context, clazz, filterstring, allservices, result);
//          return null;
//        }
//      });
//    }
//  }

//  void notifyFindHooksPrivileged(BundleContextImpl context, String clazz, String filterstring, boolean allservices, Collection result) {
//    BundleContextImpl systemBundleContext = framework.getSystemBundleContext();
//    if (systemBundleContext == null) { // if no system bundle context, we are done!
//      return;
//    }
//
//    if (Debug.DEBUG && Debug.DEBUG_SERVICES) {
//      Debug.println("notifyFindHooks(" + context.getBundleImpl() + "," + clazz + "," + filterstring + "," + allservices + "," + result + ")"); //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$ //$NON-NLS-4$ //$NON-NLS-5$ //$NON-NLS-6$
//    }
//
//    List hooks = lookupServiceRegistrations(findHookName, null);
//    // Since the list is already sorted, we don't need to sort the list to call the hooks
//    // in the proper order.
//
//    for (Iterator iter = hooks.iterator(); iter.hasNext();) {
//      ServiceRegistrationImpl registration = (ServiceRegistrationImpl) iter.next();
//      Object findHook = registration.getService(systemBundleContext);
//      if (findHook == null) { // if the hook is null
//        continue;
//      }
//      try {
//        if (findHook instanceof FindHook) { // if the hook is usable
//          ((FindHook) findHook).find(context, clazz, filterstring, allservices, result);
//        }
//      } catch (Throwable t) {
//        if (Debug.DEBUG && Debug.DEBUG_SERVICES) {
//          Debug.println(findHook + ".find() exception: " + t.getMessage()); //$NON-NLS-1$
//          Debug.printStackTrace(t);
//        }
//        // allow the adaptor to handle this unexpected error
//        framework.getAdaptor().handleRuntimeError(t);
//        ServiceException se = new ServiceException(NLS.bind(Msg.SERVICE_FACTORY_EXCEPTION, findHook.getClass().getName(), "find"), t); //$NON-NLS-1$
//        framework.publishFrameworkEvent(FrameworkEvent.ERROR, registration.getBundle(), se);
//      } finally {
//        registration.ungetService(systemBundleContext);
//      }
//    }
//  }

  /**
   * Call the registered EventHook services to allow them to inspect and possibly shrink the result.
   * The EventHooks must be called in order: descending by service.ranking, then ascending by service.id.
   * This is the natural order for ServiceReference.
   *
   * @param event The service event to be delivered.
   * @param result The result to return to the caller which may have been shrunk by the EventHooks.
   */
//  private void notifyEventHooksPrivileged(ServiceEvent event, Collection result) {
//    BundleContextImpl systemBundleContext = framework.getSystemBundleContext();
//    if (systemBundleContext == null) { // if no system bundle context, we are done!
//      return;
//    }
//
//    if (Debug.DEBUG && Debug.DEBUG_SERVICES) {
//      Debug.println("notifyEventHooks(" + event.getType() + ":" + event.getServiceReference() + "," + result + ")"); //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$ //$NON-NLS-4$
//    }
//
//    List hooks = lookupServiceRegistrations(eventHookName, null);
//    // Since the list is already sorted, we don't need to sort the list to call the hooks
//    // in the proper order.
//
//    for (Iterator iter = hooks.iterator(); iter.hasNext();) {
//      ServiceRegistrationImpl registration = (ServiceRegistrationImpl) iter.next();
//      Object eventHook = registration.getService(systemBundleContext);
//      if (eventHook == null) { // if the hook is null
//        continue;
//      }
//      try {
//        if (eventHook instanceof EventHook) { // if the hook is usable
//          ((EventHook) eventHook).event(event, result);
//        }
//      } catch (Throwable t) {
//        if (Debug.DEBUG && Debug.DEBUG_SERVICES) {
//          Debug.println(eventHook + ".event() exception: " + t.getMessage()); //$NON-NLS-1$
//          Debug.printStackTrace(t);
//        }
//        // allow the adaptor to handle this unexpected error
//        framework.getAdaptor().handleRuntimeError(t);
//        ServiceException se = new ServiceException(NLS.bind(Msg.SERVICE_FACTORY_EXCEPTION, eventHook.getClass().getName(), "event"), t); //$NON-NLS-1$
//        framework.publishFrameworkEvent(FrameworkEvent.ERROR, registration.getBundle(), se);
//      } finally {
//        registration.ungetService(systemBundleContext);
//      }
//    }
//  }

  /**
   * Call a newly registered ListenerHook service to provide the current collection of
   * service listeners.
   *
   * @param registration The newly registered ListenerHook service.
   */
//  private void notifyNewListenerHook(final ServiceRegistrationImpl registration) {
//    if (System.getSecurityManager() == null) {
//      notifyNewListenerHookPrivileged(registration);
//    } else {
//      AccessController.doPrivileged(new PrivilegedAction() {
//        public Object run() {
//          notifyNewListenerHookPrivileged(registration);
//          return null;
//        }
//      });
//    }
//
//  }

//  void notifyNewListenerHookPrivileged(ServiceRegistrationImpl registration) {
//    BundleContextImpl systemBundleContext = framework.getSystemBundleContext();
//    if (systemBundleContext == null) { // if no system bundle context, we are done!
//      return;
//    }
//
//    if (Debug.DEBUG && Debug.DEBUG_SERVICES) {
//      Debug.println("notifyNewListenerHook(" + registration + ")"); //$NON-NLS-1$ //$NON-NLS-2$
//    }
//
//    Collection addedListeners = new ArrayList(initialCapacity);
//    synchronized (serviceEventListeners) {
//      for (Iterator iter = serviceEventListeners.values().iterator(); iter.hasNext();) {
//        Map listeners = (Map) iter.next();
//        if (!listeners.isEmpty()) {
//          addedListeners.addAll(listeners.values());
//        }
//      }
//    }
//    addedListeners = Collections.unmodifiableCollection(addedListeners);
//
//    Object listenerHook = registration.getService(systemBundleContext);
//    if (listenerHook == null) { // if the hook is null
//      return;
//    }
//    try {
//      if (listenerHook instanceof ListenerHook) { // if the hook is usable
//        ((ListenerHook) listenerHook).added(addedListeners);
//      }
//    } catch (Throwable t) {
//      if (Debug.DEBUG && Debug.DEBUG_SERVICES) {
//        Debug.println(listenerHook + ".added() exception: " + t.getMessage()); //$NON-NLS-1$
//        Debug.printStackTrace(t);
//      }
//      // allow the adaptor to handle this unexpected error
//      framework.getAdaptor().handleRuntimeError(t);
//      ServiceException se = new ServiceException(NLS.bind(Msg.SERVICE_FACTORY_EXCEPTION, listenerHook.getClass().getName(), "event"), t); //$NON-NLS-1$
//      framework.publishFrameworkEvent(FrameworkEvent.ERROR, registration.getBundle(), se);
//    } finally {
//      registration.ungetService(systemBundleContext);
//    }
//  }

  /**
   * Call the registered ListenerHook services to notify them of newly added or removed service listeners.
   * The ListenerHook must be called in order: descending by service.ranking, then ascending by service.id.
   * This is the natural order for ServiceReference.
   *
   * @param listeners An unmodifiable collection of ListenerInfo objects.
   * @param added <code>true</code> if the specified listeners are being added. <code>false</code>
   * if they are being removed.
   */
//  private void notifyListenerHooks(final Collection listeners, final boolean added) {
//    if (System.getSecurityManager() == null) {
//      notifyListenerHooksPrivileged(listeners, added);
//    } else {
//      AccessController.doPrivileged(new PrivilegedAction() {
//        public Object run() {
//          notifyListenerHooksPrivileged(listeners, added);
//          return null;
//        }
//      });
//    }
//
//  }

//  void notifyListenerHooksPrivileged(Collection listeners, boolean added) {
//    BundleContextImpl systemBundleContext = framework.getSystemBundleContext();
//    if (systemBundleContext == null) { // if no system bundle context, we are done!
//      return;
//    }
//
//    if (Debug.DEBUG && Debug.DEBUG_SERVICES) {
//      Debug.println("notifyListenerHooks(" + listeners + "," + (added ? "added" : "removed") + ")"); //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$ //$NON-NLS-4$ //$NON-NLS-5$
//    }
//
//    List hooks = lookupServiceRegistrations(listenerHookName, null);
//    // Since the list is already sorted, we don't need to sort the list to call the hooks
//    // in the proper order.
//
//    for (Iterator iter = hooks.iterator(); iter.hasNext();) {
//      ServiceRegistrationImpl registration = (ServiceRegistrationImpl) iter.next();
//      Object listenerHook = registration.getService(systemBundleContext);
//      if (listenerHook == null) { // if the hook is null
//        continue;
//      }
//      try {
//        if (listenerHook instanceof ListenerHook) { // if the hook is usable
//          if (added) {
//            ((ListenerHook) listenerHook).added(listeners);
//          } else {
//            ((ListenerHook) listenerHook).removed(listeners);
//          }
//        }
//      } catch (Throwable t) {
//        if (Debug.DEBUG && Debug.DEBUG_SERVICES) {
//          Debug.println(listenerHook + "." + (added ? "added" : "removed") + "() exception: " + t.getMessage()); //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$ //$NON-NLS-4$
//          Debug.printStackTrace(t);
//        }
//        // allow the adaptor to handle this unexpected error
//        framework.getAdaptor().handleRuntimeError(t);
//        ServiceException se = new ServiceException(NLS.bind(Msg.SERVICE_FACTORY_EXCEPTION, listenerHook.getClass().getName(), "event"), t); //$NON-NLS-1$
//        framework.publishFrameworkEvent(FrameworkEvent.ERROR, registration.getBundle(), se);
//      } finally {
//        registration.ungetService(systemBundleContext);
//      }
//    }
//  }
};

}
}
}


#endif /* CHERRYSERVICEREGISTRY_H_ */
