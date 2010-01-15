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


#ifndef FRAMEWORK_H_
#define FRAMEWORK_H_

#include "../Bundle.h"

namespace osgi {
namespace framework {
namespace launch {

/**
 * A Framework instance. A Framework is also known as a System Bundle.
 *
 * <p>
 * Framework instances are created using a {@link FrameworkFactory}. The methods
 * of this interface can be used to manage and control the created framework
 * instance.
 *
 * @ThreadSafe
 * @version $Revision$
 */
struct Framework : public Bundle {

  /**
   * Initialize this Framework. After calling this method, this Framework
   * must:
   * <ul>
   * <li>Be in the {@link #STARTING} state.</li>
   * <li>Have a valid Bundle Context.</li>
   * <li>Be at start level 0.</li>
   * <li>Have event handling enabled.</li>
   * <li>Have reified Bundle objects for all installed bundles.</li>
   * <li>Have registered any framework services. For example,
   * <code>PackageAdmin</code>, <code>ConditionalPermissionAdmin</code>,
   * <code>StartLevel</code>.</li>
   * </ul>
   *
   * <p>
   * This Framework will not actually be started until {@link #start() start}
   * is called.
   *
   * <p>
   * This method does nothing if called when this Framework is in the
   * {@link #STARTING}, {@link #ACTIVE} or {@link #STOPPING} states.
   *
   * @throws BundleException If this Framework could not be initialized.
   *
   */
  virtual void Init() throw(BundleException) = 0;

  /**
   * Wait until this Framework has completely stopped. The <code>stop</code>
   * and <code>update</code> methods on a Framework performs an asynchronous
   * stop of the Framework. This method can be used to wait until the
   * asynchronous stop of this Framework has completed. This method will only
   * wait if called when this Framework is in the {@link #STARTING},
   * {@link #ACTIVE}, or {@link #STOPPING} states. Otherwise it will return
   * immediately.
   * <p>
   * A Framework Event is returned to indicate why this Framework has stopped.
   *
   * @param timeout Maximum number of milliseconds to wait until this
   *        Framework has completely stopped. A value of zero will wait
   *        indefinitely.
   * @return A Framework Event indicating the reason this method returned. The
   *         following <code>FrameworkEvent</code> types may be returned by
   *         this method.
   *         <ul>
   *         <li>{@link FrameworkEvent#STOPPED STOPPED} - This Framework has
   *         been stopped. </li>
   *
   *         <li>{@link FrameworkEvent#STOPPED_UPDATE STOPPED_UPDATE} - This
   *         Framework has been updated which has shutdown and will now
   *         restart.</li>
   *
   *         <li>{@link FrameworkEvent#ERROR ERROR} - The Framework
   *         encountered an error while shutting down or an error has occurred
   *         which forced the framework to shutdown. </li>
   *
   *         <li> {@link FrameworkEvent#WAIT_TIMEDOUT WAIT_TIMEDOUT} - This
   *         method has timed out and returned before this Framework has
   *         stopped.</li>
   *         </ul>
   * @throws InterruptedException If another thread interrupted the current
   *         thread before or while the current thread was waiting for this
   *         Framework to completely stop. The <i>interrupted status</i> of
   *         the current thread is cleared when this exception is thrown.
   */
  virtual SmartPointer<FrameworkEvent> WaitForStop(unsigned long timeout) throw(InterruptedException) = 0;

  /**
   * Start this Framework.
   *
   * <p>
   * The following steps are taken to start this Framework:
   * <ol>
   * <li>If this Framework is not in the {@link #STARTING} state,
   * {@link #init() initialize} this Framework.</li>
   * <li>All installed bundles must be started in accordance with each
   * bundle's persistent <i>autostart setting</i>. This means some bundles
   * will not be started, some will be started with <i>eager activation</i>
   * and some will be started with their <i>declared activation</i> policy. If
   * this Framework implements the optional <i>Start Level Service
   * Specification</i>, then the start level of this Framework is moved to the
   * start level specified by the
   * {@link Constants#FRAMEWORK_BEGINNING_STARTLEVEL beginning start level}
   * framework property, as described in the <i>Start Level Service
   * Specification</i>. If this framework property is not specified, then the
   * start level of this Framework is moved to start level one (1). Any
   * exceptions that occur during bundle starting must be wrapped in a
   * {@link BundleException} and then published as a framework event of type
   * {@link FrameworkEvent#ERROR}</li>
   * <li>This Framework's state is set to {@link #ACTIVE}.</li>
   * <li>A framework event of type {@link FrameworkEvent#STARTED} is fired</li>
   * </ol>
   *
   * @throws BundleException If this Framework could not be started.
   * @see "Start Level Service Specification"
   */
  virtual void Start() throw(BundleException) = 0;

  /**
   * Start this Framework.
   *
   * <p>
   * Calling this method is the same as calling {@link #start()}. There are no
   * start options for the Framework.
   *
   * @param options Ignored. There are no start options for the Framework.
   * @throws BundleException If this Framework could not be started.
   * @throws SecurityException If the caller does not have the appropriate
   *         <code>AdminPermission[this,EXECUTE]</code>, and the Java Runtime
   *         Environment supports permissions.
   * @see #start()
   */
  virtual void Start(int options) throw(BundleException) = 0;

  /**
   * Stop this Framework.
   *
   * <p>
   * The method returns immediately to the caller after initiating the
   * following steps to be taken on another thread.
   * <ol>
   * <li>This Framework's state is set to {@link #STOPPING}.</li>
   * <li>All installed bundles must be stopped without changing each bundle's
   * persistent <i>autostart setting</i>. If this Framework implements the
   * optional <i>Start Level Service Specification</i>, then the start level
   * of this Framework is moved to start level zero (0), as described in the
   * <i>Start Level Service Specification</i>. Any exceptions that occur
   * during bundle stopping must be wrapped in a {@link BundleException} and
   * then published as a framework event of type {@link FrameworkEvent#ERROR}</li>
   * <li>Unregister all services registered by this Framework.</li>
   * <li>Event handling is disabled.</li>
   * <li>This Framework's state is set to {@link #RESOLVED}.</li>
   * <li>All resources held by this Framework are released. This includes
   * threads, bundle class loaders, open files, etc.</li>
   * <li>Notify all threads that are waiting at {@link #waitForStop(long)
   * waitForStop} that the stop operation has completed.</li>
   * </ol>
   * <p>
   * After being stopped, this Framework may be discarded, initialized or
   * started.
   *
   * @throws BundleException If stopping this Framework could not be
   *         initiated.
   * @throws SecurityException If the caller does not have the appropriate
   *         <code>AdminPermission[this,EXECUTE]</code>, and the Java Runtime
   *         Environment supports permissions.
   * @see "Start Level Service Specification"
   */
  virtual void Stop() throw(BundleException) = 0;

  /**
   * Stop this Framework.
   *
   * <p>
   * Calling this method is the same as calling {@link #stop()}. There are no
   * stop options for the Framework.
   *
   * @param options Ignored. There are no stop options for the Framework.
   * @throws BundleException If stopping this Framework could not be
   *         initiated.
   * @throws SecurityException If the caller does not have the appropriate
   *         <code>AdminPermission[this,EXECUTE]</code>, and the Java Runtime
   *         Environment supports permissions.
   * @see #stop()
   */
  virtual void Stop(int options) throw(BundleException) = 0;

  /**
   * The Framework cannot be uninstalled.
   *
   * <p>
   * This method always throws a BundleException.
   *
   * @throws BundleException This Framework cannot be uninstalled.
   * @throws SecurityException If the caller does not have the appropriate
   *         <code>AdminPermission[this,LIFECYCLE]</code>, and the Java
   *         Runtime Environment supports permissions.
   */
  virtual void Uninstall() throw(BundleException) = 0;

  /**
   * Stop and restart this Framework.
   *
   * <p>
   * The method returns immediately to the caller after initiating the
   * following steps to be taken on another thread.
   * <ol>
   * <li>Perform the steps in the {@link #stop()} method to stop this
   * Framework.</li>
   * <li>Perform the steps in the {@link #start()} method to start this
   * Framework.</li>
   * </ol>
   *
   * @throws BundleException If stopping and restarting this Framework could
   *         not be initiated.
   * @throws SecurityException If the caller does not have the appropriate
   *         <code>AdminPermission[this,LIFECYCLE]</code>, and the Java
   *         Runtime Environment supports permissions.
   */
  virtual void Update() throw(BundleException) = 0;

  /**
   * Stop and restart this Framework.
   *
   * <p>
   * Calling this method is the same as calling {@link #update()} except that
   * any provided InputStream is immediately closed.
   *
   * @param in Any provided InputStream is immediately closed before returning
   *        from this method and otherwise ignored.
   * @throws BundleException If stopping and restarting this Framework could
   *         not be initiated.
   * @throws SecurityException If the caller does not have the appropriate
   *         <code>AdminPermission[this,LIFECYCLE]</code>, and the Java
   *         Runtime Environment supports permissions.
   */
  virtual void Update(std::istream* in) throw(BundleException) = 0;

  /**
   * Returns the Framework unique identifier. This Framework is assigned the
   * unique identifier zero (0) since this Framework is also a System Bundle.
   *
   * @return 0.
   * @see Bundle#getBundleId()
   */
  virtual long GetBundleId() = 0;

  /**
   * Returns the Framework location identifier. This Framework is assigned the
   * unique location &quot;<code>System Bundle</code>&quot; since this
   * Framework is also a System Bundle.
   *
   * @return The string &quot;<code>System Bundle</code>&quot;.
   * @see Bundle#GetLocation()
   * @see Constants#SYSTEM_BUNDLE_LOCATION
   */
  virtual std::string GetLocation() = 0;

  /**
   * Returns the symbolic name of this Framework. The symbolic name is unique
   * for the implementation of the framework. However, the symbolic name
   * &quot;<code>system.bundle</code>&quot; must be recognized as an alias to
   * the implementation-defined symbolic name since this Framework is also a
   * System Bundle.
   *
   * @return The symbolic name of this Framework.
   * @see Bundle#GetSymbolicName()
   * @see Constants#SYSTEM_BUNDLE_SYMBOLICNAME
   */
  virtual std::string GetSymbolicName() = 0;
};

}
}
}

#endif /* FRAMEWORK_H_ */
