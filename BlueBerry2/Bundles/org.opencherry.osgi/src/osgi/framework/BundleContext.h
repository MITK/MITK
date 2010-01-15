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


#ifndef BUNDLECONTEXT_H_
#define BUNDLECONTEXT_H_

#include "Object.h"
#include "Macros.h"
#include "Exceptions.h"
#include "Filter.h"

#include "../util/Dictionary.h"

#include <Poco/File.h>

namespace osgi {

namespace framework {

using namespace ::osgi::util;

struct Bundle;
struct ServiceRegistration;

/**
 * A bundle's execution context within the Framework. The context is used to
 * grant access to other methods so that this bundle can interact with the
 * Framework.
 *
 * <p>
 * <code>BundleContext</code> methods allow a bundle to:
 * <ul>
 * <li>Subscribe to events published by the Framework.
 * <li>Register service objects with the Framework service registry.
 * <li>Retrieve <code>ServiceReferences</code> from the Framework service
 * registry.
 * <li>Get and release service objects for a referenced service.
 * <li>Install new bundles in the Framework.
 * <li>Get the list of bundles installed in the Framework.
 * <li>Get the {@link Bundle} object for a bundle.
 * <li>Create <code>File</code> objects for files in a persistent storage
 * area provided for the bundle by the Framework.
 * </ul>
 *
 * <p>
 * A <code>BundleContext</code> object will be created and provided to the
 * bundle associated with this context when it is started using the
 * {@link BundleActivator#start} method. The same <code>BundleContext</code>
 * object will be passed to the bundle associated with this context when it is
 * stopped using the {@link BundleActivator#stop} method. A
 * <code>BundleContext</code> object is generally for the private use of its
 * associated bundle and is not meant to be shared with other bundles in the
 * OSGi environment.
 *
 * <p>
 * The <code>Bundle</code> object associated with a <code>BundleContext</code>
 * object is called the <em>context bundle</em>.
 *
 * <p>
 * The <code>BundleContext</code> object is only valid during the execution of
 * its context bundle; that is, during the period from when the context bundle
 * is in the <code>STARTING</code>, <code>STOPPING</code>, and
 * <code>ACTIVE</code> bundle states. If the <code>BundleContext</code>
 * object is used subsequently, an <code>IllegalStateException</code> must be
 * thrown. The <code>BundleContext</code> object must never be reused after
 * its context bundle is stopped.
 *
 * <p>
 * The Framework is the only entity that can create <code>BundleContext</code>
 * objects and they are only valid within the Framework that created them.
 *
 * @ThreadSafe
 * @version $Revision$
 */

struct BundleContext : public Object {

  osgiInterfaceMacro(osgi::framework::BundleContext)


  /**
   * Returns the value of the specified property. If the key is not found in
   * the Framework properties, the system properties are then searched. The
   * method returns <code>"undefined"</code> if the property is not found.
   *
   * <p>
   * All bundles must have permission to read properties whose names start
   * with &quot;org.osgi.&quot;.
   *
   * @param key The name of the requested property.
   * @return The value of the requested property, or <code>"undefined"</code> if the
   *         property is undefined.
   */
  virtual std::string GetProperty(const std::string& key) = 0;

  /**
   * Returns the <code>Bundle</code> object associated with this
   * <code>BundleContext</code>. This bundle is called the context bundle.
   *
   * @return The <code>Bundle</code> object associated with this
   *         <code>BundleContext</code>.
   * @throws IllegalStateException If this BundleContext is no
   *         longer valid.
   */
  virtual SmartPointer<Bundle> GetBundle() throw(IllegalStateException) = 0;

  /**
   * Installs a bundle from the specified <code>InputStream</code> object.
   *
   * <p>
   * If the specified <code>InputStream</code> is <code>null</code>, the
   * Framework must create the <code>InputStream</code> from which to read the
   * bundle by interpreting, in an implementation dependent manner, the
   * specified <code>location</code>.
   *
   * <p>
   * The specified <code>location</code> identifier will be used as the
   * identity of the bundle. Every installed bundle is uniquely identified by
   * its location identifier which is typically in the form of a URL.
   *
   * <p>
   * The following steps are required to install a bundle:
   * <ol>
   * <li>If a bundle containing the same location identifier is already
   * installed, the <code>Bundle</code> object for that bundle is returned.
   *
   * <li>The bundle's content is read from the input stream. If this fails, a
   * {@link BundleException} is thrown.
   *
   * <li>The bundle's associated resources are allocated. The associated
   * resources minimally consist of a unique identifier and a persistent
   * storage area if the platform has file system support. If this step fails,
   * a <code>BundleException</code> is thrown.
   *
   * <li>The bundle's state is set to <code>INSTALLED</code>.
   *
   * <li>A bundle event of type {@link BundleEvent#INSTALLED} is fired.
   *
   * <li>The <code>Bundle</code> object for the newly or previously installed
   * bundle is returned.
   * </ol>
   *
   * <b>Postconditions, no exceptions thrown </b>
   * <ul>
   * <li><code>getState()</code> in &#x007B; <code>INSTALLED</code>,
   * <code>RESOLVED</code> &#x007D;.
   * <li>Bundle has a unique ID.
   * </ul>
   * <b>Postconditions, when an exception is thrown </b>
   * <ul>
   * <li>Bundle is not installed and no trace of the bundle exists.
   * </ul>
   *
   * @param location The location identifier of the bundle to install.
   * @param input The <code>InputStream</code> object from which this bundle
   *        will be read or <code>null</code> to indicate the Framework must
   *        create the input stream from the specified location identifier.
   *        The input stream must always be closed when this method completes,
   *        even if an exception is thrown.
   * @return The <code>Bundle</code> object of the installed bundle.
   * @throws BundleException If the input stream cannot be read or the
   *         installation failed.
   * @throws SecurityException If the caller does not have the appropriate
   *         <code>AdminPermission[installed bundle,LIFECYCLE]</code>, and the
   *         Java Runtime Environment supports permissions.
   * @throws IllegalStateException If this BundleContext is no longer valid.
   */
  //public Bundle installBundle(String location, InputStream input)
  //    throws BundleException;

  /**
   * Installs a bundle from the specified <code>location</code> identifier.
   *
   * <p>
   * This method performs the same function as calling
   * {@link #installBundle(String,InputStream)} with the specified
   * <code>location</code> identifier and a <code>null</code> InputStream.
   *
   * @param location The location identifier of the bundle to install.
   * @return The <code>Bundle</code> object of the installed bundle.
   * @throws BundleException If the installation failed.
   * @throws SecurityException If the caller does not have the appropriate
   *         <code>AdminPermission[installed bundle,LIFECYCLE]</code>, and the
   *         Java Runtime Environment supports permissions.
   * @throws IllegalStateException If this BundleContext is no longer valid.
   * @see #installBundle(String, InputStream)
   */
  //public Bundle installBundle(String location) throws BundleException;

  /**
   * Returns the bundle with the specified identifier.
   *
   * @param id The identifier of the bundle to retrieve.
   * @return A <code>Bundle</code> object or <code>null</code> if the
   *         identifier does not match any installed bundle.
   */
  virtual SmartPointer<Bundle> GetBundle(long id) = 0;

  /**
   * Returns a list of all installed bundles.
   * <p>
   * This method returns a list of all bundles installed in the OSGi
   * environment at the time of the call to this method. However, since the
   * Framework is a very dynamic environment, bundles can be installed or
   * uninstalled at anytime.
   *
   * @return An array of <code>Bundle</code> objects, one object per
   *         installed bundle.
   */
  virtual std::vector<SmartPointer<Bundle> > GetBundles() = 0;

  /**
   * Adds the specified <code>ServiceListener</code> object with the
   * specified <code>filter</code> to the context bundle's list of
   * listeners. See {@link Filter} for a description of the filter syntax.
   * <code>ServiceListener</code> objects are notified when a service has a
   * lifecycle state change.
   *
   * <p>
   * If the context bundle's list of listeners already contains a listener
   * <code>l</code> such that <code>(l==listener)</code>, then this
   * method replaces that listener's filter (which may be empty)
   * with the specified one (which may be empty).
   *
   * <p>
   * The listener is called if the filter criteria is met. To filter based
   * upon the class of the service, the filter should reference the
   * {@link Constants#OBJECTCLASS} property. If <code>filter</code> is
   * empty, all services are considered to match the filter.
   *
   * <p>
   * When using a <code>filter</code>, it is possible that the
   * <code>ServiceEvent</code>s for the complete lifecycle of a service
   * will not be delivered to the listener. For example, if the
   * <code>filter</code> only matches when the property <code>x</code> has
   * the value <code>1</code>, the listener will not be called if the
   * service is registered with the property <code>x</code> not set to the
   * value <code>1</code>. Subsequently, when the service is modified
   * setting property <code>x</code> to the value <code>1</code>, the
   * filter will match and the listener will be called with a
   * <code>ServiceEvent</code> of type <code>MODIFIED</code>. Thus, the
   * listener will not be called with a <code>ServiceEvent</code> of type
   * <code>REGISTERED</code>.
   *
   * @param listener The <code>ServiceListener</code> object to be added.
   * @param filter The filter criteria.
   * @throws InvalidSyntaxException If <code>filter</code> contains an
   *         invalid filter string that cannot be parsed.
   * @throws IllegalStateException If this BundleContext is no
   *         longer valid.
   * @see ServiceEvent
   * @see ServiceListener
   * @see ServicePermission
   */
 // virtual void AddServiceListener(ServiceListener::Pointer listener, const std::string& filter = "")
 //     throw(InvalidSyntaxException, IllegalStateException) = 0;

  /**
   * Removes the specified <code>ServiceListener</code> object from the
   * context bundle's list of listeners.
   *
   * <p>
   * If <code>listener</code> is not contained in this context bundle's list
   * of listeners, this method does nothing.
   *
   * @param listener The <code>ServiceListener</code> to be removed.
   * @throws IllegalStateException If this BundleContext is no
   *         longer valid.
   */
  //virtual void RemoveServiceListener(ServiceListener::Pointer listener) = 0;

  /**
   * Adds the specified <code>BundleListener</code> object to the context
   * bundle's list of listeners if not already present. BundleListener objects
   * are notified when a bundle has a lifecycle state change.
   *
   * <p>
   * If the context bundle's list of listeners already contains a listener
   * <code>l</code> such that <code>(l==listener)</code>, this method
   * does nothing.
   *
   * @param listener The <code>BundleListener</code> to be added.
   * @throws IllegalStateException If this BundleContext is no
   *         longer valid.
   *
   * @see BundleEvent
   * @see BundleListener
   */
 // virtual void AddBundleListener(BundleListener::Pointer listener) throw(IllegalStateException) = 0;

  /**
   * Removes the specified <code>BundleListener</code> object from the
   * context bundle's list of listeners.
   *
   * <p>
   * If <code>listener</code> is not contained in the context bundle's list
   * of listeners, this method does nothing.
   *
   * @param listener The <code>BundleListener</code> object to be removed.
   * @throws IllegalStateException If this BundleContext is no
   *         longer valid.
   */
  //virtual void RemoveBundleListener(BundleListener::Pointer listener) throw(IllegalStateException) = 0;

  /**
   * Adds the specified <code>FrameworkListener</code> object to the context
   * bundle's list of listeners if not already present. FrameworkListeners are
   * notified of general Framework events.
   *
   * <p>
   * If the context bundle's list of listeners already contains a listener
   * <code>l</code> such that <code>(l==listener)</code>, this method
   * does nothing.
   *
   * @param listener The <code>FrameworkListener</code> object to be added.
   * @throws IllegalStateException If this BundleContext is no
   *         longer valid.
   * @see FrameworkEvent
   * @see FrameworkListener
   */
  //virtual void AddFrameworkListener(FrameworkListener::Pointer listener) throw(IllegalStateException) = 0;

  /**
   * Removes the specified <code>FrameworkListener</code> object from the
   * context bundle's list of listeners.
   *
   * <p>
   * If <code>listener</code> is not contained in the context bundle's list
   * of listeners, this method does nothing.
   *
   * @param listener The <code>FrameworkListener</code> object to be
   *        removed.
   * @throws IllegalStateException If this BundleContext is no
   *         longer valid.
   */
  //virtual void RemoveFrameworkListener(FrameworkListener::Pointer listener) throw(IllegalStateException) = 0;

  /**
   * Registers the specified service object with the specified properties
   * under the specified class names into the Framework. A
   * <code>ServiceRegistration</code> object is returned. The
   * <code>ServiceRegistration</code> object is for the private use of the
   * bundle registering the service and should not be shared with other
   * bundles. The registering bundle is defined to be the context bundle.
   * Other bundles can locate the service by using either the
   * {@link #GetServiceReferences} or {@link #GetServiceReference} method.
   *
   * <p>
   * A bundle can register a service object that implements the
   * {@link ServiceFactory} interface to have more flexibility in providing
   * service objects to other bundles.
   *
   * <p>
   * The following steps are required to register a service:
   * <ol>
   * <li>The Framework adds the following service properties to the service
   * properties from the specified <code>Dictionary</code> (which may be
   * empty): <br/>
   * A property named {@link Constants#SERVICE_ID} identifying the
   * registration number of the service <br/>
   * A property named {@link Constants#OBJECTCLASS} containing all the
   * specified classes. <br/>
   * Properties with these names in the specified <code>Dictionary</code> will
   * be ignored.
   * <li>The service is added to the Framework service registry and may now be
   * used by other bundles.
   * <li>A service event of type {@link ServiceEvent#REGISTERED} is fired.
   * <li>A <code>ServiceRegistration</code> object for this registration is
   * returned.
   * </ol>
   *
   * @param clazzes The class names under which the service can be located.
   *        The class names in this array will be stored in the service's
   *        properties under the key {@link Constants#OBJECTCLASS}.
   * @param service The service object or a <code>ServiceFactory</code>
   *        object.
   * @param properties The properties for this service. The keys in the
   *        properties object must all be <code>String</code> objects. See
   *        {@link Constants} for a list of standard service property keys.
   *        To update the service's properties the
   *        {@link ServiceRegistration#SetProperties} method must be called.
   *        The set of properties may be empty if the service has
   *        no properties.
   * @return A <code>ServiceRegistration</code> object for use by the bundle
   *         registering the service to update the service's properties or to
   *         unregister the service.
   * @throws IllegalArgumentException If one of the following is true:
   *         <ul>
   *         <li><code>service</code> is <code>null</code>.
   *         <li><code>properties</code> contains case variants of the same key
   *         name.
   *         </ul>
   * @throws IllegalStateException If this BundleContext is no longer valid.
   * @see ServiceRegistration
   * @see ServiceFactory
   */
  virtual SmartPointer<ServiceRegistration> RegisterService(const std::vector<std::string>& clazzes,
      Object::Pointer service, SmartPointer<const Dictionary> properties) throw(IllegalArgumentException, IllegalStateException) = 0;

  /**
   * Registers the specified service object with the specified properties
   * under the specified class name with the Framework.
   *
   * <p>
   * This method is otherwise identical to
   * {@link #RegisterService(const std::vector<std::string>&, Object::Pointer, const Dictionary&)} and is provided as
   * a convenience when <code>service</code> will only be registered under a
   * single class name. Note that even in this case the value of the service's
   * {@link Constants#OBJECTCLASS} property will be an array of string, rather
   * than just a single string.
   *
   * @param clazz The class name under which the service can be located.
   * @param service The service object or a <code>ServiceFactory</code>
   *        object.
   * @param properties The properties for this service.
   * @return A <code>ServiceRegistration</code> object for use by the bundle
   *         registering the service to update the service's properties or to
   *         unregister the service.
   * @throws IllegalArgumentException If one of the following is true:
   *         <ul>
   *         <li><code>service</code> is <code>null</code>.
   *         <li><code>properties</code> contains case variants of the same key
   *         name.
   *         </ul>
   * @throws IllegalStateException If this BundleContext is no longer valid.
   * @see #RegisterService(const std::vector<std::string>&, Object::Pointer, const Dictionary&)
   */
  virtual SmartPointer<ServiceRegistration> RegisterService(const std::string& clazz, Object::Pointer service,
      SmartPointer<const Dictionary> properties) throw(IllegalArgumentException, IllegalStateException);

  /**
   * Returns an array of <code>ServiceReference</code> objects. The returned
   * array of <code>ServiceReference</code> objects contains services that
   * were registered under the specified class and match the specified filter
   * expression.
   *
   * <p>
   * The list is valid at the time of the call to this method. However since
   * the Framework is a very dynamic environment, services can be modified or
   * unregistered at any time.
   *
   * <p>
   * The specified <code>filter</code> expression is used to select the
   * registered services whose service properties contain keys and values
   * which satisfy the filter expression. See {@link Filter} for a description
   * of the filter syntax. If the specified <code>filter</code> is
   * empty, all registered services are considered to match the
   * filter. If the specified <code>filter</code> expression cannot be parsed,
   * an {@link InvalidSyntaxException} will be thrown with a human readable
   * message where the filter became unparsable.
   *
   * <p>
   * The result is an array of <code>ServiceReference</code> objects for all
   * services that meet all of the following conditions:
   * <ul>
   * <li>If the specified class name, <code>clazz</code>, is not
   * empty, the service must have been registered with the
   * specified class name. The complete list of class names with which a
   * service was registered is available from the service's
   * {@link Constants#OBJECTCLASS objectClass} property.
   * <li>If the specified <code>filter</code> is not empty, the
   * filter expression must match the service.
   * </ul>
   *
   * @param clazz The class name with which the service was registered or
   *        empty for all services.
   * @param filter The filter expression or empty for all
   *        services.
   * @return An array of <code>ServiceReference</code> objects.
   *
   * @throws InvalidSyntaxException If the specified <code>filter</code>
   *         contains an invalid filter expression that cannot be parsed.
   * @throws IllegalStateException If this BundleContext is no longer valid.
   */
  virtual std::vector<SmartPointer<ServiceReference> > GetServiceReferences(const std::string& clazz, const std::string& filter)
      throw(InvalidSyntaxException, IllegalStateException) = 0;

  /**
   * Returns an array of <code>ServiceReference</code> objects. The returned
   * array of <code>ServiceReference</code> objects contains services that
   * were registered under the specified class and match the specified filter
   * expression.
   *
   * <p>
   * The list is valid at the time of the call to this method. However since
   * the Framework is a very dynamic environment, services can be modified or
   * unregistered at any time.
   *
   * <p>
   * The specified <code>filter</code> expression is used to select the
   * registered services whose service properties contain keys and values
   * which satisfy the filter expression. See {@link Filter} for a description
   * of the filter syntax. If the specified <code>filter</code> is
   * empty, all registered services are considered to match the
   * filter. If the specified <code>filter</code> expression cannot be parsed,
   * an {@link InvalidSyntaxException} will be thrown with a human readable
   * message where the filter became unparsable.
   *
   * <p>
   * The result is an array of <code>ServiceReference</code> objects for all
   * services that meet all of the following conditions:
   * <ul>
   * <li>If the specified class name, <code>clazz</code>, is not
   * empty, the service must have been registered with the
   * specified class name. The complete list of class names with which a
   * service was registered is available from the service's
   * {@link Constants#OBJECTCLASS objectClass} property.
   * <li>If the specified <code>filter</code> is not empty, the
   * filter expression must match the service.
   * </ul>
   *
   * @param clazz The class name with which the service was registered or
   *        empty for all services.
   * @param filter The filter expression or empty for all services.
   * @return An array of <code>ServiceReference</code> objects.
   *
   * @throws InvalidSyntaxException If the specified <code>filter</code>
   *         contains an invalid filter expression that cannot be parsed.
   * @throws IllegalStateException If this BundleContext is no longer valid.
   */
  virtual std::vector<SmartPointer<ServiceReference> > GetAllServiceReferences(const std::string& clazz,
      const std::string& filter) throw(InvalidSyntaxException, IllegalStateException) = 0;

  /**
   * Returns a <code>ServiceReference</code> object for a service that
   * implements and was registered under the specified class.
   *
   * <p>
   * The returned <code>ServiceReference</code> object is valid at the time of
   * the call to this method. However as the Framework is a very dynamic
   * environment, services can be modified or unregistered at any time.
   *
   * <p>
   * This method is the same as calling
   * {@link BundleContext#GetServiceReferences(const std::sring&, const std::string&)} with an
   * empty filter expression. It is provided as a convenience for
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
   * @param clazz The class name with which the service was registered.
   * @return A <code>ServiceReference</code> object, or <code>null</code> if
   *         no services are registered which implement the named class.
   * @throws IllegalStateException If this BundleContext is no longer valid.
   * @see #GetServiceReferences(const std::string&, const std::string&)
   */
  virtual SmartPointer<ServiceReference> GetServiceReference(const std::string& clazz)
      throw(IllegalStateException) = 0;

  /**
   * Returns the service object referenced by the specified
   * <code>ServiceReference</code> object.
   * <p>
   * A bundle's use of a service is tracked by the bundle's use count of that
   * service. Each time a service's service object is returned by
   * {@link #GetService(SmartPointer<ServiceReference>)} the context bundle's use count for
   * that service is incremented by one. Each time the service is released by
   * {@link #UngetService(SmartPointer<ServiceReference>)} the context bundle's use count
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
   * <li>If the service has been unregistered, <code>null</code> is returned.
   * <li>The context bundle's use count for this service is incremented by
   * one.
   * <li>If the context bundle's use count for the service is currently one
   * and the service was registered with an object implementing the
   * <code>ServiceFactory</code> interface, the
   * {@link ServiceFactory#GetService(SmartPointer<Bundle>, SmartPointer<ServiceRegistration>)} method is
   * called to create a service object for the context bundle. This service
   * object is cached by the Framework. While the context bundle's use count
   * for the service is greater than zero, subsequent calls to get the
   * services's service object for the context bundle will return the cached
   * service object. <br>
   * If the <code>ServiceFactory</code> object throws an
   * exception, <code>null</code> is returned and a Framework event of type
   * {@link FrameworkEvent#ERROR} containing a {@link ServiceException}
   * describing the error is fired.
   * <li>The service object for the service is returned.
   * </ol>
   *
   * @param reference A reference to the service.
   * @return A service object for the service associated with
   *         <code>reference</code> or <code>null</code> if the service is not
   *         registered, the service object returned by a
   *         <code>ServiceFactory</code> does not implement the classes under
   *         which it was registered or the <code>ServiceFactory</code> threw
   *         an exception.
   * @throws IllegalStateException If this BundleContext is no
   *         longer valid.
   * @throws IllegalArgumentException If the specified
   *         <code>ServiceReference</code> was not created by the same
   *         framework instance as this <code>BundleContext</code>.
   * @see #UngetService(SmartPointer<ServiceReference>)
   * @see ServiceFactory
   */
  virtual Object::Pointer GetService(SmartPointer<ServiceReference> reference)
      throw(IllegalStateException, IllegalArgumentException) = 0;

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
   * and the service was registered with a <code>ServiceFactory</code> object,
   * the
   * {@link ServiceFactory#UngetService(SmartPointer<Bundle>, SmartPointer<ServiceRegistration>, Object::Pointer)}
   * method is called to release the service object for the context bundle.
   * <li><code>true</code> is returned.
   * </ol>
   *
   * @param reference A reference to the service to be released.
   * @return <code>false</code> if the context bundle's use count for the
   *         service is zero or if the service has been unregistered;
   *         <code>true</code> otherwise.
   * @throws IllegalStateException If this BundleContext is no
   *         longer valid.
   * @throws IllegalArgumentException If the specified
   *         <code>ServiceReference</code> was not created by the same
   *         framework instance as this <code>BundleContext</code>.
   * @see #GetService
   * @see ServiceFactory
   */
  virtual bool UngetService(SmartPointer<ServiceReference> reference)
     throw(IllegalStateException, IllegalArgumentException) = 0;

  /**
   * Creates a <code>Poco::File</code> object for a file in the persistent storage
   * area provided for the bundle by the Framework.
   * <p>
   * A <code>File</code> object for the base directory of the persistent
   * storage area provided for the context bundle by the Framework can be
   * obtained by calling this method with an empty string as
   * <code>filename</code>.
   *
   * @param filename A relative name to the file to be accessed.
   * @return A <code>Poco::File</code> object that represents the requested file.
   *
   * @throws IllegalStateException If this BundleContext is no
   *         longer valid.
   */
  virtual Poco::File GetDataFile(const std::string& filename)
     throw(IllegalStateException) = 0;

  /**
   * Creates a <code>Filter</code> object. This <code>Filter</code> object may
   * be used to match a <code>ServiceReference</code> object or a
   * <code>Dictionary</code> object.
   *
   * <p>
   * If the filter cannot be parsed, an {@link InvalidSyntaxException} will be
   * thrown with a human readable message where the filter became unparsable.
   *
   * @param filter The filter string.
   * @return A <code>Filter</code> object encapsulating the filter string.
   * @throws InvalidSyntaxException If <code>filter</code> contains an invalid
   *         filter string that cannot be parsed.
   * @throws IllegalStateException If this BundleContext is no longer valid.
   * @see "Framework specification for a description of the filter string syntax."
   * @see FrameworkUtil#CreateFilter(const std::string&)
   */
  virtual Filter::Pointer CreateFilter(const std::string& filter)
     throw(InvalidSyntaxException, IllegalStateException);
};

}

}

#endif /* BUNDLECONTEXT_H_ */
