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


#ifndef CHERRYBUNDLECONTEXTIMPL_H_
#define CHERRYBUNDLECONTEXTIMPL_H_

#include <osgi/framework/BundleContext.h>

#include "eventmgr/cherryEventDispatcher.h"

namespace cherry {
namespace osgi {
namespace internal {

class ServiceUse;
class ServiceRegistrationImpl;
class BundleHost;
struct BundleActivator;

using namespace ::osgi::framework;

/**
 * Bundle's execution context.
 *
 * This object is given out to bundles and wraps the internal
 * BundleContext object. It is destroyed when a bundle is stopped.
 */

class BundleContextImpl : public BundleContext
{

public:

  osgiObjectMacro(cherry::osgi::internal::BundleContextImpl)

  typedef Poco::HashMap<SmartPointer<ServiceRegistrationImpl>, SmartPointer<ServiceUse>, Object::Hash > ServiceUseMap;

private:

  /** true if the bundle context is still valid */
  volatile bool valid;

  /** Bundle object this context is associated with. */
  // This slot is accessed directly by the Framework instead of using
  // the getBundle() method because the Framework needs access to the bundle
  // even when the context is invalid while the close method is being called.
  //const BundleHost bundle;

  /** Internal framework object. */
  //const Framework framework;

  /** Services that bundle is using. Key is ServiceRegistrationImpl,
   Value is ServiceUse */
  /* @GuardedBy("contextLock") */
  ServiceUseMap servicesInUse;

  /** private object for locking */
  Poco::Mutex contextLock;

  struct BundleEventDispatcher : public EventDispatcher {
    /**
   * Bottom level event dispatcher for the BundleContext.
   *
   * @param originalListener listener object registered under.
   * @param l listener to call (may be filtered).
   * @param action Event class type
   * @param object Event object
   */
  void DispatchEvent(Object::Pointer originalListener, Object::Pointer l, int action, Object::Pointer object);
  };

  EventDispatcher::Pointer eventDispatcher;

protected:

  /** Listener list for bundle's BundleListeners */
  //Map bundleEvent;

  /** Listener list for bundle's SynchronousBundleListeners */
  //Map bundleEventSync;

  /** Listener list for bundle's FrameworkListeners */
  //Map frameworkEvent;

  /** The current instantiation of the activator. */
  //BundleActivator activator;



  /**
   * Construct a BundleContext which wrappers the framework for a
   * bundle
   *
   * @param bundle The bundle we are wrapping.
   */
  BundleContextImpl(SmartPointer<BundleHost> bundle);

  /**
   * Destroy the wrapper. This is called when the bundle is stopped.
   *
   */
  void Close();

   /**
   * This method checks that the context is still valid.
   *
   * @return true if the context is still valid; false otherwise
   */
  bool IsValid() const;

  /**
   * Call bundle's BundleActivator.start()
   * This method is called by Bundle.startWorker to start the bundle.
   *
   * @exception BundleException if
   *            the bundle has a class that implements the BundleActivator interface,
   *            but Framework couldn't instantiate it, or the BundleActivator.start()
   *            method failed
   */
  void Start() throw(BundleException);

  /**
   * Calls the start method of a BundleActivator.
   * @param bundleActivator that activator to start
   */
  void StartActivator(BundleActivator* bundleActivator) throw(BundleException);

  /**
   * Call bundle's BundleActivator.stop()
   * This method is called by Bundle.stopWorker to stop the bundle.
   *
   * @exception BundleException if
   *            the bundle has a class that implements the BundleActivator interface,
   *            and the BundleActivator.stop() method failed
   */
  void Stop() throw(BundleException);


public:

  /**
   * Retrieve the value of the named environment property.
   *
   * @param key The name of the requested property.
   * @return The value of the requested property, or <code>null</code> if
   * the property is undefined.
   */
  std::string GetProperty(const std::string& key);

  /**
   * Retrieve the Bundle object for the context bundle.
   *
   * @return The context bundle's Bundle object.
   */
  SmartPointer<Bundle> GetBundle() throw(IllegalStateException);

  SmartPointer<BundleHost> GetBundleImpl();

  //Bundle InstallBundle(String location) throws BundleException;

//  Bundle InstallBundle(String location, InputStream in) throws BundleException;

  /**
   * Retrieve the bundle that has the given unique identifier.
   *
   * @param id The identifier of the bundle to retrieve.
   * @return A Bundle object, or <code>null</code>
   * if the identifier doesn't match any installed bundle.
   */
  SmartPointer<Bundle> GetBundle(long id);

  /**
   * Retrieve the bundle that has the given location.
   *
   * @param location The location string of the bundle to retrieve.
   * @return A Bundle object, or <code>null</code>
   * if the location doesn't match any installed bundle.
   */
  SmartPointer<BundleHost> GetBundleByLocation(const std::string& location);

  /**
   * Retrieve a list of all installed bundles.
   * The list is valid at the time
   * of the call to getBundles, but the framework is a very dynamic
   * environment and bundles can be installed or uninstalled at anytime.
   *
   * @return An array of {@link AbstractBundle} objects, one
   * object per installed bundle.
   */
  std::vector<SmartPointer<Bundle> > GetBundles();

  /**
   * Add a service listener with a filter.
   * {@link ServiceListener}s are notified when a service has a lifecycle
   * state change.
   * See {@link #getServiceReferences(String, String) getServiceReferences}
   * for a description of the filter syntax.
   * The listener is added to the context bundle's list of listeners.
   * See {@link #getBundle() getBundle()}
   * for a definition of context bundle.
   *
   * <p>The listener is called if the filter criteria is met.
   * To filter based upon the class of the service, the filter
   * should reference the "objectClass" property.
   * If the filter paramater is <code>null</code>, all services
   * are considered to match the filter.
   * <p>If the Java runtime environment supports permissions, then additional
   * filtering is done.
   * {@link AbstractBundle#hasPermission(Object) Bundle.hasPermission} is called for the
   * bundle which defines the listener to validate that the listener has the
   * {@link ServicePermission} permission to <code>"get"</code> the service
   * using at least one of the named classes the service was registered under.
   *
   * @param listener The service listener to add.
   * @param filter The filter criteria.
   * @exception InvalidSyntaxException If the filter parameter contains
   * an invalid filter string which cannot be parsed.
   * @see ServiceEvent
   * @see ServiceListener
   * @exception java.lang.IllegalStateException
   * If the bundle context has stopped.
   */
//  void AddServiceListener(SmartPointer<ServiceListener> listener, const std::string& filter) throw(InvalidSyntaxException);

  /**
   * Add a service listener.
   *
   * <p>This method is the same as calling
   * {@link #addServiceListener(ServiceListener, String)}
   * with filter set to <code>null</code>.
   *
   * @see #addServiceListener(ServiceListener, String)
   */
//  void AddServiceListener(SmartPointer<ServiceListener> listener);

  /**
   * Remove a service listener.
   * The listener is removed from the context bundle's list of listeners.
   * See {@link #getBundle() getBundle()}
   * for a definition of context bundle.
   *
   * <p>If this method is called with a listener which is not registered,
   * then this method does nothing.
   *
   * @param listener The service listener to remove.
   * @exception java.lang.IllegalStateException
   * If the bundle context has stopped.
   */
//  void RemoveServiceListener(SmartPointer<ServiceListener> listener);

  /**
   * Add a bundle listener.
   * {@link BundleListener}s are notified when a bundle has a lifecycle
   * state change.
   * The listener is added to the context bundle's list of listeners.
   * See {@link #getBundle() getBundle()}
   * for a definition of context bundle.
   *
   * @param listener The bundle listener to add.
   * @exception java.lang.IllegalStateException
   * If the bundle context has stopped.
   * @see BundleEvent
   * @see BundleListener
   */
//  void AddBundleListener(SmartPointer<BundleListener> listener);

  /**
   * Remove a bundle listener.
   * The listener is removed from the context bundle's list of listeners.
   * See {@link #getBundle() getBundle()}
   * for a definition of context bundle.
   *
   * <p>If this method is called with a listener which is not registered,
   * then this method does nothing.
   *
   * @param listener The bundle listener to remove.
   * @exception java.lang.IllegalStateException
   * If the bundle context has stopped.
   */
//  void RemoveBundleListener(SmartPointer<BundleListener> listener);

  /**
   * Add a general framework listener.
   * {@link FrameworkListener}s are notified of general framework events.
   * The listener is added to the context bundle's list of listeners.
   * See {@link #getBundle() getBundle()}
   * for a definition of context bundle.
   *
   * @param listener The framework listener to add.
   * @exception java.lang.IllegalStateException
   * If the bundle context has stopped.
   * @see FrameworkEvent
   * @see FrameworkListener
   */
//  void AddFrameworkListener(SmartPointer<FrameworkListener> listener);

  /**
   * Remove a framework listener.
   * The listener is removed from the context bundle's list of listeners.
   * See {@link #getBundle() getBundle()}
   * for a definition of context bundle.
   *
   * <p>If this method is called with a listener which is not registered,
   * then this method does nothing.
   *
   * @param listener The framework listener to remove.
   * @exception java.lang.IllegalStateException
   * If the bundle context has stopped.
   */
//  void RemoveFrameworkListener(SmartPointer<FrameworkListener> listener);

  /**
   * Register a service with multiple names.
   * This method registers the given service object with the given properties
   * under the given class names.
   * A {@link ServiceRegistration} object is returned.
   * The {@link ServiceRegistration} object is for the private use of the bundle
   * registering the service and should not be shared with other bundles.
   * The registering bundle is defined to be the context bundle.
   * See {@link #getBundle()} for a definition of context bundle.
   * Other bundles can locate the service by using either the
   * {@link #getServiceReferences getServiceReferences} or
   * {@link #getServiceReference getServiceReference} method.
   *
   * <p>A bundle can register a service object that implements the
   * {@link ServiceFactory} interface to
   * have more flexiblity in providing service objects to different
   * bundles.
   *
   * <p>The following steps are followed to register a service:
   * <ol>
   * <li>If the service parameter is not a {@link ServiceFactory},
   * an <code>IllegalArgumentException</code> is thrown if the
   * service parameter is not an <code>instanceof</code>
   * all the classes named.
   * <li>The service is added to the framework's service registry
   * and may now be used by other bundles.
   * <li>A {@link ServiceEvent} of type {@link ServiceEvent#REGISTERED}
   * is synchronously sent.
   * <li>A {@link ServiceRegistration} object for this registration
   * is returned.
   * </ol>
   *
   * @param clazzes The class names under which the service can be located.
   *                The class names in this array will be stored in the service's
   *                properties under the key "objectClass".
   * @param service The service object or a {@link ServiceFactory} object.
   * @param properties The properties for this service.
   *        The keys in the properties object must all be Strings.
   *        Changes should not be made to this object after calling this method.
   *        To update the service's properties call the
   *        {@link ServiceRegistration#setProperties ServiceRegistration.setProperties}
   *        method.
   *        This parameter may be <code>null</code> if the service has no properties.
   * @return A {@link ServiceRegistration} object for use by the bundle
   *        registering the service to update the
   *        service's properties or to unregister the service.
   * @exception java.lang.IllegalArgumentException If one of the following is true:
   * <ul>
   * <li>The service parameter is null.
   * <li>The service parameter is not a {@link ServiceFactory} and is not an
   * <code>instanceof</code> all the named classes in the clazzes parameter.
   * </ul>
   * @exception java.lang.SecurityException If the caller does not have
   * {@link ServicePermission} permission to "register" the service for
   * all the named classes
   * and the Java runtime environment supports permissions.
   * @exception java.lang.IllegalStateException
   * If the bundle context has stopped.
   * @see ServiceRegistration
   * @see ServiceFactory
   */
  SmartPointer<ServiceRegistration> RegisterService(const std::vector<std::string>& clazzes, Object::Pointer service,
      SmartPointer<const Dictionary> properties) throw(IllegalArgumentException, IllegalStateException);

  /**
   * Register a service with a single name.
   * This method registers the given service object with the given properties
   * under the given class name.
   *
   * <p>This method is otherwise identical to
   * {@link #registerService(java.lang.String[], java.lang.Object, java.util.Dictionary)}
   * and is provided as a convenience when the service parameter will only be registered
   * under a single class name.
   *
   * @see #registerService(java.lang.String[], java.lang.Object, java.util.Dictionary)
   */
  SmartPointer<ServiceRegistration> RegisterService(const std::string& clazz, Object::Pointer service, SmartPointer<const Dictionary> properties) throw(IllegalArgumentException, IllegalStateException);

  /**
   * Returns a list of <tt>ServiceReference</tt> objects. This method returns a list of
   * <tt>ServiceReference</tt> objects for services which implement and were registered under
   * the specified class and match the specified filter criteria.
   *
   * <p>The list is valid at the time of the call to this method, however as the Framework is
   * a very dynamic environment, services can be modified or unregistered at anytime.
   *
   * <p><tt>filter</tt> is used to select the registered service whose
   * properties objects contain keys and values which satisfy the filter.
   * See {@link Filter}for a description of the filter string syntax.
   *
   * <p>If <tt>filter</tt> is <tt>null</tt>, all registered services
   * are considered to match the filter.
   * <p>If <tt>filter</tt> cannot be parsed, an {@link InvalidSyntaxException} will
   * be thrown with a human readable message where the filter became unparsable.
   *
   * <p>The following steps are required to select a service:
   * <ol>
   * <li>If the Java Runtime Environment supports permissions, the caller is checked for the
   * <tt>ServicePermission</tt> to get the service with the specified class.
   * If the caller does not have the correct permission, <tt>null</tt> is returned.
   * <li>If the filter string is not <tt>null</tt>, the filter string is
   * parsed and the set of registered services which satisfy the filter is
   * produced.
   * If the filter string is <tt>null</tt>, then all registered services
   * are considered to satisfy the filter.
   * <li>If <code>clazz</code> is not <tt>null</tt>, the set is further reduced to
   * those services which are an <tt>instanceof</tt> and were registered under the specified class.
   * The complete list of classes of which a service is an instance and which
   * were specified when the service was registered is available from the
   * service's {@link Constants#OBJECTCLASS}property.
   * <li>An array of <tt>ServiceReference</tt> to the selected services is returned.
   * </ol>
   *
   * @param clazz The class name with which the service was registered, or
   * <tt>null</tt> for all services.
   * @param filter The filter criteria.
   * @return An array of <tt>ServiceReference</tt> objects, or
   * <tt>null</tt> if no services are registered which satisfy the search.
   * @exception InvalidSyntaxException If <tt>filter</tt> contains
   * an invalid filter string which cannot be parsed.
   */
  std::vector<SmartPointer<ServiceReference> > GetServiceReferences(const std::string& clazz, const std::string& filter) throw(InvalidSyntaxException);

  std::vector<SmartPointer<ServiceReference> > GetAllServiceReferences(const std::string& clazz, const std::string& filter) throw(InvalidSyntaxException);

  /**
   * Get a service reference.
   * Retrieves a {@link ServiceReference} for a service
   * which implements the named class.
   *
   * <p>This reference is valid at the time
   * of the call to this method, but since the framework is a very dynamic
   * environment, services can be modified or unregistered at anytime.
   *
   * <p>This method is provided as a convenience for when the caller is
   * interested in any service which implements a named class. This method is
   * the same as calling {@link #getServiceReferences getServiceReferences}
   * with a <code>null</code> filter string but only a single {@link ServiceReference}
   * is returned.
   *
   * @param clazz The class name which the service must implement.
   * @return A {@link ServiceReference} object, or <code>null</code>
   * if no services are registered which implement the named class.
   * @see #getServiceReferences
   */
  SmartPointer<ServiceReference> GetServiceReference(const std::string& clazz) throw(IllegalStateException);

  /**
   * Get a service's service object.
   * Retrieves the service object for a service.
   * A bundle's use of a service is tracked by a
   * use count. Each time a service's service object is returned by
   * {@link #getService}, the context bundle's use count for the service
   * is incremented by one. Each time the service is release by
   * {@link #ungetService}, the context bundle's use count
   * for the service is decremented by one.
   * When a bundle's use count for a service
   * drops to zero, the bundle should no longer use the service.
   * See {@link #getBundle()} for a definition of context bundle.
   *
   * <p>This method will always return <code>null</code> when the
   * service associated with this reference has been unregistered.
   *
   * <p>The following steps are followed to get the service object:
   * <ol>
   * <li>If the service has been unregistered,
   * <code>null</code> is returned.
   * <li>The context bundle's use count for this service is incremented by one.
   * <li>If the context bundle's use count for the service is now one and
   * the service was registered with a {@link ServiceFactory},
   * the {@link ServiceFactory#getService ServiceFactory.getService} method
   * is called to create a service object for the context bundle.
   * This service object is cached by the framework.
   * While the context bundle's use count for the service is greater than zero,
   * subsequent calls to get the services's service object for the context bundle
   * will return the cached service object.
   * <br>If the service object returned by the {@link ServiceFactory}
   * is not an <code>instanceof</code>
   * all the classes named when the service was registered or
   * the {@link ServiceFactory} throws an exception,
   * <code>null</code> is returned and a
   * {@link FrameworkEvent} of type {@link FrameworkEvent#ERROR} is broadcast.
   * <li>The service object for the service is returned.
   * </ol>
   *
   * @param reference A reference to the service whose service object is desired.
   * @return A service object for the service associated with this
   * reference, or <code>null</code> if the service is not registered.
   * @exception java.lang.SecurityException If the caller does not have
   * {@link ServicePermission} permission to "get" the service
   * using at least one of the named classes the service was registered under
   * and the Java runtime environment supports permissions.
   * @exception java.lang.IllegalStateException
   * If the bundle context has stopped.
   * @see #ungetService
   * @see ServiceFactory
   */
   Object::Pointer GetService(SmartPointer<ServiceReference> reference) throw(IllegalStateException, IllegalArgumentException);

  /**
   * Unget a service's service object.
   * Releases the service object for a service.
   * If the context bundle's use count for the service is zero, this method
   * returns <code>false</code>. Otherwise, the context bundle's use count for the
   * service is decremented by one.
   * See {@link #getBundle()} for a definition of context bundle.
   *
   * <p>The service's service object
   * should no longer be used and all references to it should be destroyed
   * when a bundle's use count for the service
   * drops to zero.
   *
   * <p>The following steps are followed to unget the service object:
   * <ol>
   * <li>If the context bundle's use count for the service is zero or
   * the service has been unregistered,
   * <code>false</code> is returned.
   * <li>The context bundle's use count for this service is decremented by one.
   * <li>If the context bundle's use count for the service is now zero and
   * the service was registered with a {@link ServiceFactory},
   * the {@link ServiceFactory#ungetService ServiceFactory.ungetService} method
   * is called to release the service object for the context bundle.
   * <li><code>true</code> is returned.
   * </ol>
   *
   * @param reference A reference to the service to be released.
   * @return <code>false</code> if the context bundle's use count for the service
   *         is zero or if the service has been unregistered,
   *         otherwise <code>true</code>.
   * @exception java.lang.IllegalStateException
   * If the bundle context has stopped.
   * @see #getService
   * @see ServiceFactory
   */
  bool UngetService(SmartPointer<ServiceReference> reference) throw(IllegalStateException, IllegalArgumentException);

  /**
   * Creates a <code>File</code> object for a file in the
   * persistent storage area provided for the bundle by the framework.
   * If the adaptor does not have file system support, this method will
   * return <code>null</code>.
   *
   * <p>A <code>File</code> object for the base directory of the
   * persistent storage area provided for the context bundle by the framework
   * can be obtained by calling this method with the empty string ("")
   * as the parameter.
   * See {@link #getBundle()} for a definition of context bundle.
   *
   * <p>If the Java runtime environment supports permissions,
   * the framework the will ensure that the bundle has
   * <code>java.io.FilePermission</code> with actions
   * "read","write","execute","delete" for all files (recursively) in the
   * persistent storage area provided for the context bundle by the framework.
   *
   * @param filename A relative name to the file to be accessed.
   * @return A <code>File</code> object that represents the requested file or
   * <code>null</code> if the adaptor does not have file system support.
   * @exception java.lang.IllegalStateException
   * If the bundle context has stopped.
   */
  Poco::File GetDataFile(const std::string& filename) throw(IllegalStateException);

  /**
   * Return the map of ServiceRegistrationImpl to ServiceUse for services being
   * used by this context.
   * @return A map of ServiceRegistrationImpl to ServiceUse for services in use by
   * this context.
   */
  ServiceUseMap& GetServicesInUseMap();

  /**
   * Get the EventDispatcher for this bundle context.
   *
   * @return the EventDispatcher
   */
  EventDispatcher::Pointer GetEventDispatcher() const;

  /**
   * Construct a Filter object. This filter object may be used
   * to match a ServiceReference or a Dictionary.
   * See Filter
   * for a description of the filter string syntax.
   *
   * @param filter The filter string.
   * @return A Filter object encapsulating the filter string.
   * @exception InvalidSyntaxException If the filter parameter contains
   * an invalid filter string which cannot be parsed.
   */
  SmartPointer<Filter> CreateFilter(const std::string& filter) const throw(InvalidSyntaxException);

  /**
   * This method checks that the context is still valid. If the context is
   * no longer valid, an IllegalStateException is thrown.
   *
   * @exception java.lang.IllegalStateException
   * If the context bundle has stopped.
   */
  void CheckValid() const;

//  Framework GetFramework();

};

}
}
}

#endif /* CHERRYBUNDLECONTEXTIMPL_H_ */
