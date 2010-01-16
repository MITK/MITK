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


#ifndef BERRYBUNDLEHOST_H_
#define BERRYBUNDLEHOST_H_

#include <osgi/framework/Bundle.h>
#include <osgi/framework/BundleActivator.h>
#include <BlueBerryAdapter/framework/berryBundleData.h>

#include <Poco/Thread.h>

namespace berry {
namespace osgi {
namespace internal{

class Framework;
class BundleContextImpl;

using namespace ::osgi::framework;
using namespace ::berry::osgi::framework;

/**
 * This object is given out to bundles and wraps the internal Bundle object. It
 * is destroyed when a bundle is uninstalled and reused if a bundle is updated.
 */
class BundleHost : public Bundle {

protected:

  /** The Framework this bundle is part of */
  SmartPointer<Framework> framework;
  /** The state of the bundle. */
  volatile int state;
  /** A flag to denote whether a bundle state change is in progress */
  volatile Poco::Thread* stateChanging;
  /** Bundle's BundleData object */
  SmartPointer<BundleData> bundledata;
  /** Internal object used for state change synchronization */
  Poco::Mutex statechangeLock;

  /** The BundleContext that represents this Bundle and all of its fragments */
  SmartPointer<BundleContextImpl> context;

  //volatile protected ManifestLocalization manifestLocalization = null;

  /**
   * Bundle object constructor. This constructor should not perform any real
   * work.
   *
   * @param bundledata
   *            BundleData for this bundle
   * @param framework
   *            Framework this bundle is running in
   */
  static BundleHost::Pointer CreateBundle(SmartPointer<BundleData> bundledata,
      SmartPointer<Framework> framework, bool setBundle) throw(BundleException);

  /**
   * Load the bundle.
   */
  void Load();

  /**
   * Reload from a new bundle.
   * This method must be called while holding the bundles lock.
   *
   * @param newBundle Dummy Bundle which contains new data.
   * @return  true if an exported package is "in use". i.e. it has been imported by a bundle
   */
  bool Reload(SmartPointer<BundleHost> newBundle);

  /**
   * Refresh the bundle. This is called by Framework.refreshPackages.
   * This method must be called while holding the bundles lock.
   */
  void Refresh();

  /**
   * Unload the bundle.
   * This method must be called while holding the bundles lock.
   *
   * @return  true if an exported package is "in use". i.e. it has been imported by a bundle
   */
  bool Unload();

  /**
   * Close the the Bundle's file.
   *
   */
  void Close();

  /**
   * Load and instantiate bundle's BundleActivator class
   */
  SmartPointer<BundleActivator> LoadBundleActivator() throw(BundleException);

    /**
   * Internal worker to start a bundle.
   *
   * @param options the start options
   */
  void StartWorker(int options) throw(BundleException);

  /**
   * @throws BundleException
   */
  void StartHook() throw(BundleException);

  bool ReadyToResume();

  /**
   * Create a BundleContext for this bundle.
   *
   * @return BundleContext for this bundle.
   */
  SmartPointer<BundleContextImpl> CreateContext();

  /**
   * Return the current context for this bundle.
   *
   * @return BundleContext for this bundle.
   */
  SmartPointer<BundleContextImpl> GetContext();

  /**
   * Internal worker to stop a bundle.
   *
   * @param options the stop options
   */
  void StopWorker(int options) throw(BundleException);

  /**
   * @throws BundleException
   */
  void StopHook() throw(BundleException);

  /**
   * Return true if the bundle is starting or active.
   *
   */
  bool IsActive();


  /**
   * Start this bundle w/o marking is persistently started.
   *
   * <p>
   * The following steps are followed to start a bundle:
   * <ol>
   * <li>If the bundle is {@link #UNINSTALLED}then an <code>IllegalStateException</code>
   * is thrown.
   * <li>If the bundle is {@link #ACTIVE}or {@link #STARTING}then this
   * method returns immediately.
   * <li>If the bundle is {@link #STOPPING}then this method may wait for
   * the bundle to return to the {@link #RESOLVED}state before continuing.
   * If this does not occur in a reasonable time, a {@link BundleException}
   * is thrown to indicate the bundle was unable to be started.
   * <li>If the bundle is not {@link #RESOLVED}, an attempt is made to
   * resolve the bundle. If the bundle cannot be resolved, a
   * {@link BundleException}is thrown.
   * <li>The state of the bundle is set to {@link #STARTING}.
   * <li>The {@link BundleActivator#start(BundleContext) start}method of the bundle's
   * {@link BundleActivator}, if one is specified, is called. If the
   * {@link BundleActivator}is invalid or throws an exception, the state of
   * the bundle is set back to {@link #RESOLVED}, the bundle's listeners, if
   * any, are removed, service's registered by the bundle, if any, are
   * unregistered, and service's used by the bundle, if any, are released. A
   * {@link BundleException}is then thrown.
   * <li>The state of the bundle is set to {@link #ACTIVE}.
   * <li>A {@link BundleEvent}of type {@link BundleEvent#STARTED}is
   * broadcast.
   * </ol>
   *
   * <h5>Preconditons</h5>
   * <ul>
   * <li>getState() in {{@link #INSTALLED},{@link #RESOLVED}}.
   * </ul>
   * <h5>Postconditons, no exceptions thrown</h5>
   * <ul>
   * <li>getState() in {{@link #ACTIVE}}.
   * <li>{@link BundleActivator#start(BundleContext) BundleActivator.start}has been called
   * and did not throw an exception.
   * </ul>
   * <h5>Postconditions, when an exception is thrown</h5>
   * <ul>
   * <li>getState() not in {{@link #STARTING},{@link #ACTIVE}}.
   * </ul>
   *
   * @exception BundleException
   *                If the bundle couldn't be started. This could be because
   *                a code dependency could not be resolved or the specified
   *                BundleActivator could not be loaded or threw an
   *                exception.
   * @exception java.lang.IllegalStateException
   *                If the bundle tries to change its own state.
   */
  void Resume() throw(BundleException);

  /**
   * Set the persistent status bit for the bundle.
   *
   * @param mask
   *            Mask for bit to set/clear
   * @param state
   *            true to set bit, false to clear bit
   */
  void SetStatus(int mask, bool state);

  /**
   * Stop this bundle w/o marking is persistently stopped.
   *
   * Any services registered by this bundle will be unregistered. Any
   * services used by this bundle will be released. Any listeners registered
   * by this bundle will be removed.
   *
   * <p>
   * The following steps are followed to stop a bundle:
   * <ol>
   * <li>If the bundle is {@link #UNINSTALLED}then an <code>IllegalStateException</code>
   * is thrown.
   * <li>If the bundle is {@link #STOPPING},{@link #RESOLVED}, or
   * {@link #INSTALLED}then this method returns immediately.
   * <li>If the bundle is {@link #STARTING}then this method may wait for
   * the bundle to reach the {@link #ACTIVE}state before continuing. If this
   * does not occur in a reasonable time, a {@link BundleException}is thrown
   * to indicate the bundle was unable to be stopped.
   * <li>The state of the bundle is set to {@link #STOPPING}.
   * <li>The {@link BundleActivator#stop(BundleContext) stop}method of the bundle's
   * {@link BundleActivator}, if one is specified, is called. If the
   * {@link BundleActivator}throws an exception, this method will continue
   * to stop the bundle. A {@link BundleException}will be thrown after
   * completion of the remaining steps.
   * <li>The bundle's listeners, if any, are removed, service's registered
   * by the bundle, if any, are unregistered, and service's used by the
   * bundle, if any, are released.
   * <li>The state of the bundle is set to {@link #RESOLVED}.
   * <li>A {@link BundleEvent}of type {@link BundleEvent#STOPPED}is
   * broadcast.
   * </ol>
   *
   * <h5>Preconditons</h5>
   * <ul>
   * <li>getState() in {{@link #ACTIVE}}.
   * </ul>
   * <h5>Postconditons, no exceptions thrown</h5>
   * <ul>
   * <li>getState() not in {{@link #ACTIVE},{@link #STOPPING}}.
   * <li>{@link BundleActivator#stop(BundleContext) BundleActivator.stop}has been called
   * and did not throw an exception.
   * </ul>
   * <h5>Postconditions, when an exception is thrown</h5>
   * <ul>
   * <li>None.
   * </ul>
   *
   * @param lock
   *            true if state change lock should be held when returning from
   *            this method.
   * @exception BundleException
   *                If the bundle's BundleActivator could not be loaded or
   *                threw an exception.
   * @exception java.lang.IllegalStateException
   *                If the bundle tries to change its own state.
   */
  void Suspend(bool lock) throw(BundleException);

   /**
   * Update worker. Assumes the caller has the state change lock.
   */
  //void UpdateWorker(PrivilegedExceptionAction action) throw(BundleException);

  /**
   * Update worker. Assumes the caller has the state change lock.
   */
  //void UpdateWorkerPrivileged(URLConnection source, AccessControlContext callerContext) throw(BundleException);

  /**
   * Uninstall worker. Assumes the caller has the state change lock.
   */
  //void UninstallWorker(PrivilegedExceptionAction action) throw(BundleException);

  /**
   * Uninstall worker. Assumes the caller has the state change lock.
   */
  void UninstallWorkerPrivileged() throw(BundleException);


  /**
   * This method marks the bundle's state as changing so that other calls to
   * start/stop/suspend/update/uninstall can wait until the state change is
   * complete. If stateChanging is non-null when this method is called, we
   * will wait for the state change to complete. If the timeout expires
   * without changing state (this may happen if the state change is back up
   * our call stack), a BundleException is thrown so that we don't wait
   * forever.
   *
   * A call to this method should be immediately followed by a try block
   * whose finally block calls completeStateChange().
   *
   * beginStateChange(); try { // change the bundle's state here... } finally {
   * completeStateChange(); }
   *
   * @exception org.osgi.framework.BundleException
   *                if the bundles state is still changing after waiting for
   *                the timeout.
   */
  void BeginStateChange() throw(BundleException);

  /**
   * This method completes the bundle state change by setting stateChanging
   * to null and notifying one waiter that the state change has completed.
   */
  void CompleteStateChange();

  /**
   * This method checks that the bundle is not uninstalled. If the bundle is
   * uninstalled, an IllegalStateException is thrown.
   *
   * @exception java.lang.IllegalStateException
   *                If the bundle is uninstalled.
   */
  void CheckValid();

  /**
   * Mark this bundle as resolved.
   */
  void Resolve();


public:

  /**
   * Bundle object constructor. This constructor should not perform any real
   * work.
   *
   * @param bundledata
   *            BundleData for this bundle
   * @param framework
   *            Framework this bundle is running in
   */
  BundleHost(SmartPointer<BundleData> bundledata, SmartPointer<Framework> framework) throw(BundleException);

  /**
   * Find the specified resource in this bundle.
   *
   * This bundle's class loader is called to search for the named resource.
   * If this bundle's state is <tt>INSTALLED</tt>, then only this bundle will
   * be searched for the specified resource. Imported packages cannot be searched
   * when a bundle has not been resolved.
   *
   * @param name The name of the resource.
   * See <tt>java.lang.ClassLoader.getResource</tt> for a description of
   * the format of a resource name.
   * @return a URL to the named resource, or <tt>null</tt> if the resource could
   * not be found or if the caller does not have
   * the <tt>AdminPermission</tt>, and the Java Runtime Environment supports permissions.
   *
   * @exception java.lang.IllegalStateException If this bundle has been uninstalled.
   */
  std::istream* GetResource(const std::string& name) const throw(IllegalStateException);


  /**
   * Provides a list of {@link ServiceReference}s for the services
   * registered by this bundle
   * or <code>null</code> if the bundle has no registered
   * services.
   *
   * <p>The list is valid at the time
   * of the call to this method, but the framework is a very dynamic
   * environment and services can be modified or unregistered at anytime.
   *
   * @return An array of {@link ServiceReference} or <code>null</code>.
   * @exception java.lang.IllegalStateException If the
   * bundle has been uninstalled.
   * @see ServiceRegistration
   * @see ServiceReference
   */
  std::vector<SmartPointer<ServiceReference> > GetRegisteredServices() throw(IllegalStateException);

  /**
   * Provides a list of {@link ServiceReference}s for the
   * services this bundle is using,
   * or <code>null</code> if the bundle is not using any services.
   * A bundle is considered to be using a service if the bundle's
   * use count for the service is greater than zero.
   *
   * <p>The list is valid at the time
   * of the call to this method, but the framework is a very dynamic
   * environment and services can be modified or unregistered at anytime.
   *
   * @return An array of {@link ServiceReference} or <code>null</code>.
   * @exception java.lang.IllegalStateException If the
   * bundle has been uninstalled.
   * @see ServiceReference
   */
  std::vector<SmartPointer<ServiceReference> > GetServicesInUse()  throw(IllegalStateException);

  /**
   * Returns the current state of the bundle.
   *
   * A bundle can only be in one state at any time.
   *
   * @return bundle's state.
   */
  int GetState();

  SmartPointer<Framework> GetFramework();

  /**
   * Return true if the bundle is resolved.
   *
   */
  bool IsResolved();

  /**
   * Start this bundle.
   *
   * If the current start level is less than this bundle's start level, then
   * the Framework must persistently mark this bundle as started and delay
   * the starting of this bundle until the Framework's current start level
   * becomes equal or more than the bundle's start level.
   * <p>
   * Otherwise, the following steps are required to start a bundle:
   * <ol>
   * <li>If the bundle is {@link #UNINSTALLED}then an <code>IllegalStateException</code>
   * is thrown.
   * <li>If the bundle is {@link #ACTIVE}or {@link #STARTING}then this
   * method returns immediately.
   * <li>If the bundle is {@link #STOPPING}then this method may wait for
   * the bundle to return to the {@link #RESOLVED}state before continuing.
   * If this does not occur in a reasonable time, a {@link BundleException}
   * is thrown to indicate the bundle was unable to be started.
   * <li>If the bundle is not {@link #RESOLVED}, an attempt is made to
   * resolve the bundle. If the bundle cannot be resolved, a
   * {@link BundleException}is thrown.
   * <li>The state of the bundle is set to {@link #STARTING}.
   * <li>The {@link BundleActivator#start(BundleContext) start}method of the bundle's
   * {@link BundleActivator}, if one is specified, is called. If the
   * {@link BundleActivator}is invalid or throws an exception, the state of
   * the bundle is set back to {@link #RESOLVED}, the bundle's listeners, if
   * any, are removed, service's registered by the bundle, if any, are
   * unregistered, and service's used by the bundle, if any, are released. A
   * {@link BundleException}is then thrown.
   * <li>It is recorded that this bundle has been started, so that when the
   * framework is restarted, this bundle will be automatically started.
   * <li>The state of the bundle is set to {@link #ACTIVE}.
   * <li>A {@link BundleEvent}of type {@link BundleEvent#STARTED}is
   * broadcast.
   * </ol>
   *
   * <h5>Preconditons</h5>
   * <ul>
   * <li>getState() in {{@link #INSTALLED},{@link #RESOLVED}}.
   * </ul>
   * <h5>Postconditons, no exceptions thrown</h5>
   * <ul>
   * <li>getState() in {{@link #ACTIVE}}.
   * <li>{@link BundleActivator#start(BundleContext) BundleActivator.start}has been called
   * and did not throw an exception.
   * </ul>
   * <h5>Postconditions, when an exception is thrown</h5>
   * <ul>
   * <li>getState() not in {{@link #STARTING},{@link #ACTIVE}}.
   * </ul>
   *
   * @exception BundleException
   *                If the bundle couldn't be started. This could be because
   *                a code dependency could not be resolved or the specified
   *                BundleActivator could not be loaded or threw an
   *                exception.
   * @exception java.lang.IllegalStateException
   *                If the bundle has been uninstalled or the bundle tries to
   *                change its own state.
   * @exception java.lang.SecurityException
   *                If the caller does not have {@link AdminPermission}
   *                permission and the Java runtime environment supports
   *                permissions.
   */
  void Start(int options = 0) throw(BundleException, IllegalStateException);


  /**
   * Stop this bundle.
   *
   * Any services registered by this bundle will be unregistered. Any
   * services used by this bundle will be released. Any listeners registered
   * by this bundle will be removed.
   *
   * <p>
   * The following steps are followed to stop a bundle:
   * <ol>
   * <li>If the bundle is {@link #UNINSTALLED}then an <code>IllegalStateException</code>
   * is thrown.
   * <li>If the bundle is {@link #STOPPING},{@link #RESOLVED}, or
   * {@link #INSTALLED}then this method returns immediately.
   * <li>If the bundle is {@link #STARTING}then this method may wait for
   * the bundle to reach the {@link #ACTIVE}state before continuing. If this
   * does not occur in a reasonable time, a {@link BundleException}is thrown
   * to indicate the bundle was unable to be stopped.
   * <li>The state of the bundle is set to {@link #STOPPING}.
   * <li>It is recorded that this bundle has been stopped, so that when the
   * framework is restarted, this bundle will not be automatically started.
   * <li>The {@link BundleActivator#stop(BundleContext) stop}method of the bundle's
   * {@link BundleActivator}, if one is specified, is called. If the
   * {@link BundleActivator}throws an exception, this method will continue
   * to stop the bundle. A {@link BundleException}will be thrown after
   * completion of the remaining steps.
   * <li>The bundle's listeners, if any, are removed, service's registered
   * by the bundle, if any, are unregistered, and service's used by the
   * bundle, if any, are released.
   * <li>The state of the bundle is set to {@link #RESOLVED}.
   * <li>A {@link BundleEvent}of type {@link BundleEvent#STOPPED}is
   * broadcast.
   * </ol>
   *
   * <h5>Preconditons</h5>
   * <ul>
   * <li>getState() in {{@link #ACTIVE}}.
   * </ul>
   * <h5>Postconditons, no exceptions thrown</h5>
   * <ul>
   * <li>getState() not in {{@link #ACTIVE},{@link #STOPPING}}.
   * <li>{@link BundleActivator#stop(BundleContext) BundleActivator.stop}has been called
   * and did not throw an exception.
   * </ul>
   * <h5>Postconditions, when an exception is thrown</h5>
   * <ul>
   * <li>None.
   * </ul>
   *
   * @exception BundleException
   *                If the bundle's BundleActivator could not be loaded or
   *                threw an exception.
   * @exception java.lang.IllegalStateException
   *                If the bundle has been uninstalled or the bundle tries to
   *                change its own state.
   * @exception java.lang.SecurityException
   *                If the caller does not have {@link AdminPermission}
   *                permission and the Java runtime environment supports
   *                permissions.
   */
  void Stop(int options = 0) throw(BundleException, IllegalStateException);


  void Update(std::istream * const in = 0) throw(BundleException);


  /**
   * Uninstall this bundle.
   * <p>
   * This method removes all traces of the bundle, including any data in the
   * persistent storage area provided for the bundle by the framework.
   *
   * <p>
   * The following steps are followed to uninstall a bundle:
   * <ol>
   * <li>If the bundle is {@link #UNINSTALLED}then an <code>IllegalStateException</code>
   * is thrown.
   * <li>If the bundle is {@link #ACTIVE}or {@link #STARTING}, the bundle
   * is stopped as described in the {@link #stop()}method. If {@link #stop()}
   * throws an exception, a {@link FrameworkEvent}of type
   * {@link FrameworkEvent#ERROR}is broadcast containing the exception.
   * <li>A {@link BundleEvent}of type {@link BundleEvent#UNINSTALLED}is
   * broadcast.
   * <li>The state of the bundle is set to {@link #UNINSTALLED}.
   * <li>The bundle and the persistent storage area provided for the bundle
   * by the framework, if any, is removed.
   * </ol>
   *
   * <h5>Preconditions</h5>
   * <ul>
   * <li>getState() not in {{@link #UNINSTALLED}}.
   * </ul>
   * <h5>Postconditons, no exceptions thrown</h5>
   * <ul>
   * <li>getState() in {{@link #UNINSTALLED}}.
   * <li>The bundle has been uninstalled.
   * </ul>
   * <h5>Postconditions, when an exception is thrown</h5>
   * <ul>
   * <li>getState() not in {{@link #UNINSTALLED}}.
   * <li>The Bundle has not been uninstalled.
   * </ul>
   *
   * @exception BundleException
   *                If the uninstall failed.
   * @exception java.lang.IllegalStateException
   *                If the bundle has been uninstalled or the bundle tries to
   *                change its own state.
   * @exception java.lang.SecurityException
   *                If the caller does not have {@link AdminPermission}
   *                permission and the Java runtime environment supports
   *                permissions.
   * @see #stop()
   */
  void Uninstall() throw(BundleException, IllegalStateException);

  /**
   * Returns this bundle's Manifest headers and values. This method returns
   * all the Manifest headers and values from the main section of the
   * bundle's Manifest file; that is, all lines prior to the first blank
   * line.
   *
   * <p>
   * Manifest header names are case-insensitive. The methods of the returned
   * <tt>Dictionary</tt> object will operate on header names in a
   * case-insensitive manner.
   *
   * If a Manifest header begins with a '%', it will be evaluated with the
   * specified properties file for the specied Locale.
   *
   * <p>
   * For example, the following Manifest headers and values are included if
   * they are present in the Manifest file:
   *
   * <pre>
   *  Bundle-Name
   *  Bundle-Vendor
   *  Bundle-Version
   *  Bundle-Description
   *  Bundle-DocURL
   *  Bundle-ContactAddress
   * </pre>
   *
   * <p>
   * This method will continue to return Manifest header information while
   * this bundle is in the <tt>UNINSTALLED</tt> state.
   *
   * @return A <tt>Dictionary</tt> object containing this bundle's Manifest
   *         headers and values.
   *
   * @exception java.lang.SecurityException
   *                If the caller does not have the <tt>AdminPermission</tt>,
   *                and the Java Runtime Environment supports permissions.
   */
  SmartPointer<Dictionary> GetHeaders(const std::string& localeString = "");

  /**
   * Retrieve the bundle's unique identifier, which the framework assigned to
   * this bundle when it was installed.
   *
   * <p>
   * The unique identifier has the following attributes:
   * <ul>
   * <li>It is unique and persistent.
   * <li>The identifier is a long.
   * <li>Once its value is assigned to a bundle, that value is not reused
   * for another bundle, even after the bundle is uninstalled.
   * <li>Its value does not change as long as the bundle remains installed.
   * <li>Its value does not change when the bundle is updated
   * </ul>
   *
   * <p>
   * This method will continue to return the bundle's unique identifier when
   * the bundle is in the {@link #UNINSTALLED}state.
   *
   * @return This bundle's unique identifier.
   */
  long GetBundleId();

  /**
   * Retrieve the location identifier of the bundle. This is typically the
   * location passed to
   * {@link BundleContextImpl#installBundle(String) BundleContext.installBundle}when the
   * bundle was installed. The location identifier of the bundle may change
   * during bundle update. Calling this method while framework is updating
   * the bundle results in undefined behavior.
   *
   * <p>
   * This method will continue to return the bundle's location identifier
   * when the bundle is in the {@link #UNINSTALLED}state.
   *
   * @return A string that is the location identifier of the bundle.
   * @exception java.lang.SecurityException
   *                If the caller does not have {@link AdminPermission}
   *                permission and the Java runtime environment supports
   *                permissions.
   */
  std::string GetLocation();

  /**
   * Return a string representation of this bundle.
   *
   * @return String
   */
  std::string ToString() const;

  /**
   * Answers an integer indicating the relative positions of the receiver and
   * the argument in the natural order of elements of the receiver's class.
   *
   * @return int which should be <0 if the receiver should sort before the
   *         argument, 0 if the receiver should sort in the same position as
   *         the argument, and >0 if the receiver should sort after the
   *         argument.
   * @param obj
   *            another Bundle an object to compare the receiver to
   * @exception ClassCastException
   *                if the argument can not be converted into something
   *                comparable with the receiver.
   */
  bool operator<(const Object * obj) const;

  std::string GetSymbolicName();

  long GetLastModified();

  SmartPointer<BundleData> GetBundleData();

  Version GetVersion();

  // SmartPointer<BundleDescription> GetBundleDescription();

//  protected: int getStartLevel() {
//    return bundledata.getStartLevel();
//  }



  SmartPointer<BundleContext> GetBundleContext();


  BundleException GetResolutionFailureException();

  std::size_t HashCode() const;

  bool operator==(const Object* other) const;

  /* This method is used by the Bundle Localization Service to obtain
   * a ResourceBundle that resides in a bundle.  This is not an OSGi
   * defined method for org.osgi.framework.Bundle
   *
   */
//  ResourceBundle GetResourceBundle(String localeString) {
//    ManifestLocalization localization;
//    try {
//      localization = getManifestLocalization();
//    } catch (BundleException ex) {
//      return (null);
//    }
//    if (localeString == null) {
//      localeString = Locale.getDefault().toString();
//    }
//    return localization.getResourceBundle(localeString);
//  }

   bool TestStateChanging(Poco::Thread* thread);

  Poco::Thread* GetStateChanging();

//  Enumeration FindEntries(String path, String filePattern, boolean recurse) {
//    try {
//      framework.checkAdminPermission(this, AdminPermission.RESOURCE);
//    } catch (SecurityException e) {
//      return null;
//    }
//    checkValid();
//    // check to see if the bundle is resolved
//    if (!isResolved())
//      framework.packageAdmin.resolveBundles(new Bundle[] {this});
//
//    // a list used to store the results of the search
//    List pathList = new ArrayList();
//    Filter patternFilter = null;
//    Hashtable patternProps = null;
//    if (filePattern != null)
//      try {
//        // create a file pattern filter with 'filename' as the key
//        patternFilter = FilterImpl.newInstance("(filename=" + filePattern + ")"); //$NON-NLS-1$ //$NON-NLS-2$
//        // create a single hashtable to be shared during the recursive search
//        patternProps = new Hashtable(2);
//      } catch (InvalidSyntaxException e) {
//        // cannot happen
//      }
//    // find the local entries of this bundle
//    findLocalEntryPaths(path, patternFilter, patternProps, recurse, pathList);
//    // if this bundle is a host to fragments then search the fragments
//    final BundleFragment[] fragments = getFragments();
//    final int numFragments = fragments == null ? -1 : fragments.length;
//    for (int i = 0; i < numFragments; i++)
//      ((AbstractBundle) fragments[i]).findLocalEntryPaths(path, patternFilter, patternProps, recurse, pathList);
//    // return null if no entries found
//    if (pathList.size() == 0)
//      return null;
//    // create an enumeration to enumerate the pathList
//    final String[] pathArray = (String[]) pathList.toArray(new String[pathList.size()]);
//    return new Enumeration() {
//      int curIndex = 0;
//      int curFragment = -1;
//      URL nextElement = null;
//
//      public boolean hasMoreElements() {
//        if (nextElement != null)
//          return true;
//        getNextElement();
//        return nextElement != null;
//      }
//
//      public Object nextElement() {
//        if (!hasMoreElements())
//          throw new NoSuchElementException();
//        URL result;
//        result = nextElement;
//        // force the next element search
//        getNextElement();
//        return result;
//      }
//
//      private void getNextElement() {
//        nextElement = null;
//        if (curIndex >= pathArray.length)
//          // reached the end of the pathArray; no more elements
//          return;
//        String curPath = pathArray[curIndex];
//        if (curFragment == -1) {
//          // need to search ourselves first
//          nextElement = getEntry0(curPath);
//          curFragment++;
//        }
//        // if the element is not in the host look in the fragments until we have searched them all
//        while (nextElement == null && curFragment < numFragments)
//          nextElement = fragments[curFragment++].getEntry0(curPath);
//        // if we have no fragments or we have searched all fragments then advance to the next path
//        if (numFragments == -1 || curFragment >= numFragments) {
//          curIndex++;
//          curFragment = -1;
//        }
//        // searched all fragments for the current path, move to the next one
//        if (nextElement == null)
//          getNextElement();
//      }
//
//    };
//  }

private:

  // BundleException GetResolverError(SmartPointer<BundleDescription> bundleDesc);


//  synchronized ManifestLocalization GetManifestLocalization() throw(BundleException) {
//    ManifestLocalization currentLocalization = manifestLocalization;
//    if (currentLocalization == null) {
//      Dictionary rawHeaders = bundledata.getManifest();
//      manifestLocalization = currentLocalization = new ManifestLocalization(this, rawHeaders);
//    }
//    return currentLocalization;
//  }



//  protected void findLocalEntryPaths(String path, Filter patternFilter, Hashtable patternProps, boolean recurse, List pathList) {
//    Enumeration entryPaths = bundledata.getEntryPaths(path);
//    if (entryPaths == null)
//      return;
//    while (entryPaths.hasMoreElements()) {
//      String entry = (String) entryPaths.nextElement();
//      int lastSlash = entry.lastIndexOf('/');
//      if (patternProps != null) {
//        int secondToLastSlash = entry.lastIndexOf('/', lastSlash - 1);
//        int fileStart;
//        int fileEnd = entry.length();
//        if (lastSlash < 0)
//          fileStart = 0;
//        else if (lastSlash != entry.length() - 1)
//          fileStart = lastSlash + 1;
//        else {
//          fileEnd = lastSlash; // leave the lastSlash out
//          if (secondToLastSlash < 0)
//            fileStart = 0;
//          else
//            fileStart = secondToLastSlash + 1;
//        }
//        String fileName = entry.substring(fileStart, fileEnd);
//        // set the filename to the current entry
//        patternProps.put("filename", fileName); //$NON-NLS-1$
//      }
//      // prevent duplicates and match on the patterFilter
//      if (!pathList.contains(entry) && (patternFilter == null || patternFilter.matchCase(patternProps)))
//        pathList.add(entry);
//      // rescurse only into entries that are directories
//      if (recurse && !entry.equals(path) && entry.length() > 0 && lastSlash == (entry.length() - 1))
//        findLocalEntryPaths(entry, patternFilter, patternProps, recurse, pathList);
//    }
//    return;
//  }

  class BundleStatusException : public Poco::Exception {

  private:

    Object::Pointer status;

  public:

    BundleStatusException(const std::string& msg, int code = 0, Object::Pointer status = Object::Pointer(0));

    BundleStatusException(const BundleStatusException& exc);

    ~BundleStatusException() throw();

    BundleStatusException& operator = (const BundleStatusException& exc);

    const char* name() const throw();

    const char* className() const throw();

    Poco::Exception* clone() const;

    void rethrow() const;

    Object::Pointer GetStatus() const;

  };

//  public Map/* <X509Certificate, List<X509Certificate>> */getSignerCertificates(int signersType) {
//    if (signersType != SIGNERS_ALL && signersType != SIGNERS_TRUSTED)
//      throw new IllegalArgumentException("Invalid signers type: " + signersType); //$NON-NLS-1$
//    if (framework == null)
//      return Collections.EMPTY_MAP;
//    SignedContentFactory factory = framework.getSignedContentFactory();
//    if (factory == null)
//      return Collections.EMPTY_MAP;
//    try {
//      SignedContent signedContent = factory.getSignedContent(this);
//      SignerInfo[] infos = signedContent.getSignerInfos();
//      if (infos.length == 0)
//        return Collections.EMPTY_MAP;
//      Map/* <X509Certificate, List<X509Certificate>> */results = new HashMap(infos.length);
//      for (int i = 0; i < infos.length; i++) {
//        if (signersType == SIGNERS_TRUSTED && !infos[i].isTrusted())
//          continue;
//        Certificate[] certs = infos[i].getCertificateChain();
//        if (certs == null || certs.length == 0)
//          continue;
//        List/* <X509Certificate> */certChain = new ArrayList();
//        for (int j = 0; j < certs.length; j++)
//          certChain.add(certs[j]);
//        results.put(certs[0], certChain);
//      }
//      return results;
//    } catch (Exception e) {
//      return Collections.EMPTY_MAP;
//    }
//  }
};

}
}
}

#endif /* BERRYBUNDLEHOST_H_ */
