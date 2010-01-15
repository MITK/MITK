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


#ifndef CHERRYFRAMEWORK_H_
#define CHERRYFRAMEWORK_H_

#include <osgi/framework/Object.h>
#include <osgi/framework/Macros.h>

#include "eventmgr/cherryListenerQueue.h"
#include "eventmgr/cherryEventManager.h"

namespace cherry {
namespace osgi {
namespace internal {

/**
 * Core OSGi Framework class.
 */
class Framework : public Object { // : EventDispatcher, EventPublisher, Runnable {

protected:

  /** EventManager for event delivery. */
  EventManager::Pointer eventManager;

public:

  osgiObjectMacro(cherry::osgi::internal::Framework)

//  public static final String PROP_FRAMEWORK_THREAD = "osgi.framework.activeThreadType"; //$NON-NLS-1$
//  public static final String THREAD_NORMAL = "normal"; //$NON-NLS-1$
//
//  /** FrameworkAdaptor specific functions. */
//  protected FrameworkAdaptor adaptor;
//  /** Framework properties object.  A reference to the
//   * System.getProperies() object.  The properties from
//   * the adaptor will be merged into these properties.
//   */
//  protected Properties properties;
//  /** Has the framework been started */
//  protected boolean active;
//  /** Event indicating the reason for shutdown*/
//  private FrameworkEvent[] shutdownEvent;
//  /** The bundles installed in the framework */
//  protected BundleRepository bundles;
//
//  /** The ServiceRegistry */
//  private ServiceRegistry serviceRegistry;
//
//  /*
//   * The following EventListeners objects keep track of event listeners
//   * by BundleContext.  Each element is a EventListeners that is the list
//   * of event listeners for a particular BundleContext.  The max number of
//   * elements each of the following lists will have is the number of bundles
//   * installed in the Framework.
//   */
//  /** List of BundleContexts for bundle's BundleListeners. */
//  protected Map bundleEvent;
//  protected static final int BUNDLEEVENT = 1;
//  /** List of BundleContexts for bundle's SynchronousBundleListeners. */
//  protected Map bundleEventSync;
//  protected static final int BUNDLEEVENTSYNC = 2;
//  /* SERVICEEVENT(3) is now handled by ServiceRegistry */
//  /** List of BundleContexts for bundle's FrameworkListeners. */
//  protected Map frameworkEvent;
//  protected static final int FRAMEWORKEVENT = 4;
//  protected static final int BATCHEVENT_BEGIN = Integer.MIN_VALUE + 1;
//  protected static final int BATCHEVENT_END = Integer.MIN_VALUE;
//  /* Reservation object for install synchronization */
//  protected Hashtable installLock;
//  /** System Bundle object */
//  protected InternalSystemBundle systemBundle;
//  private String[] bootDelegation;
//  private String[] bootDelegationStems;
//  private boolean bootDelegateAll = false;
//  public final boolean contextBootDelegation = "true".equals(FrameworkProperties.getProperty("osgi.context.bootdelegation", "true")); //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$
//  public final boolean compatibiltyBootDelegation = "true".equals(FrameworkProperties.getProperty(Constants.OSGI_COMPATIBILITY_BOOTDELEGATION, "true")); //$NON-NLS-1$ //$NON-NLS-2$
//  ClassLoaderDelegateHook[] delegateHooks;
//  private volatile boolean forcedRestart = false;
//  /**
//   * The AliasMapper used to alias OS Names.
//   */
//  protected static AliasMapper aliasMapper = new AliasMapper();
//  SecureAction secureAction = (SecureAction) AccessController.doPrivileged(SecureAction.createSecureAction());
//  // cache of AdminPermissions keyed by Bundle ID
//  private HashMap adminPermissions = new HashMap();
//
//  // we need to hold these so that we can unregister them at shutdown
//  private StreamHandlerFactory streamHandlerFactory;
//  private ContentHandlerFactory contentHandlerFactory;
//
//  private volatile ServiceTracker signedContentFactory;
//  private volatile ContextFinder contextFinder;
//
//  /*
//   * We need to make sure that the GetDataFileAction class loads early to prevent a ClassCircularityError when checking permissions.
//   * see bug 161561
//   */
//  static {
//    Class c;
//    c = GetDataFileAction.class;
//    c.getName(); // to prevent compiler warnings
//  }
//
//  static class GetDataFileAction implements PrivilegedAction {
//    private AbstractBundle bundle;
//    private String filename;
//
//    public GetDataFileAction(AbstractBundle bundle, String filename) {
//      this.bundle = bundle;
//      this.filename = filename;
//    }
//
//    public Object run() {
//      return bundle.getBundleData().getDataFile(filename);
//    }
//  }

  /**
   * Constructor for the Framework instance. This method initializes the
   * framework to an unlaunched state.
   *
   */
public: Framework(/*FrameworkAdaptor adaptor*/);

  /**
   * Initialize the framework to an unlaunched state. This method is called
   * by the Framework constructor.
   *
   */
protected: void Initialize(/*FrameworkAdaptor adaptor*/);

//
//  public FrameworkAdaptor getAdaptor() {
//    return adaptor;
//  }
//
//  public ClassLoaderDelegateHook[] getDelegateHooks() {
//    return delegateHooks;
//  }
//
//  public ServiceRegistry getServiceRegistry() {
//    return serviceRegistry;
//  }
//
//  private void setNLSFrameworkLog() {
//    try {
//      Field frameworkLogField = NLS.class.getDeclaredField("frameworkLog"); //$NON-NLS-1$
//      frameworkLogField.setAccessible(true);
//      frameworkLogField.set(null, adaptor.getFrameworkLog());
//    } catch (Exception e) {
//      adaptor.getFrameworkLog().log(new FrameworkLogEntry(FrameworkAdaptor.FRAMEWORK_SYMBOLICNAME, FrameworkLogEntry.ERROR, 0, e.getMessage(), 0, e, null));
//    }
//  }
//
//  private void createSystemBundle() {
//    try {
//      systemBundle = new InternalSystemBundle(this);
//      systemBundle.getBundleData().setBundle(systemBundle);
//    } catch (BundleException e) { // fatal error
//      e.printStackTrace();
//      throw new RuntimeException(NLS.bind(Msg.OSGI_SYSTEMBUNDLE_CREATE_EXCEPTION, e.getMessage()));
//    }
//  }
//
//  /**
//   * Initialize the System properties by copying properties from the adaptor
//   * properties object. This method is called by the initialize method.
//   *
//   */
//  protected void initializeProperties(Properties adaptorProperties) {
//    properties = FrameworkProperties.getProperties();
//    Enumeration enumKeys = adaptorProperties.propertyNames();
//    while (enumKeys.hasMoreElements()) {
//      String key = (String) enumKeys.nextElement();
//      if (properties.getProperty(key) == null) {
//        properties.put(key, adaptorProperties.getProperty(key));
//      }
//    }
//    properties.put(Constants.FRAMEWORK_VENDOR, Constants.OSGI_FRAMEWORK_VENDOR);
//    properties.put(Constants.FRAMEWORK_VERSION, Constants.OSGI_FRAMEWORK_VERSION);
//    String value = properties.getProperty(Constants.FRAMEWORK_PROCESSOR);
//    if (value == null) {
//      value = properties.getProperty(Constants.JVM_OS_ARCH);
//      if (value != null) {
//        properties.put(Constants.FRAMEWORK_PROCESSOR, aliasMapper.aliasProcessor(value));
//      }
//    }
//    value = properties.getProperty(Constants.FRAMEWORK_OS_NAME);
//    if (value == null) {
//      value = properties.getProperty(Constants.JVM_OS_NAME);
//      try {
//        String canonicalValue = (String) aliasMapper.aliasOSName(value);
//        if (canonicalValue != null) {
//          value = canonicalValue;
//        }
//      } catch (ClassCastException ex) {
//        //A vector was returned from the alias mapper.
//        //The alias mapped to more than one canonical value
//        //such as "win32" for example
//      }
//      if (value != null) {
//        properties.put(Constants.FRAMEWORK_OS_NAME, value);
//      }
//    }
//    value = properties.getProperty(Constants.FRAMEWORK_OS_VERSION);
//    if (value == null) {
//      value = properties.getProperty(Constants.JVM_OS_VERSION);
//      if (value != null) {
//        // only use the value upto the first space
//        int space = value.indexOf(' ');
//        if (space > 0) {
//          value = value.substring(0, space);
//        }
//        // fix up cases where the os version does not make a valid Version string.
//        int major = 0, minor = 0, micro = 0;
//        String qualifier = ""; //$NON-NLS-1$
//        try {
//          StringTokenizer st = new StringTokenizer(value, ".", true); //$NON-NLS-1$
//          major = parseVersionInt(st.nextToken());
//
//          if (st.hasMoreTokens()) {
//            st.nextToken(); // consume delimiter
//            minor = parseVersionInt(st.nextToken());
//
//            if (st.hasMoreTokens()) {
//              st.nextToken(); // consume delimiter
//              micro = parseVersionInt(st.nextToken());
//
//              if (st.hasMoreTokens()) {
//                st.nextToken(); // consume delimiter
//                qualifier = st.nextToken();
//              }
//            }
//          }
//        } catch (NoSuchElementException e) {
//          // ignore, use the values parsed so far
//        }
//        value = new Version(major, minor, micro, qualifier).toString();
//        properties.put(Constants.FRAMEWORK_OS_VERSION, value);
//      }
//    }
//    value = properties.getProperty(Constants.FRAMEWORK_LANGUAGE);
//    if (value == null)
//      // set the value of the framework language property
//      properties.put(Constants.FRAMEWORK_LANGUAGE, Locale.getDefault().getLanguage());
//    // set the support properties for fragments and require-bundle (bug 173090)
//    properties.put(Constants.SUPPORTS_FRAMEWORK_FRAGMENT, "true"); //$NON-NLS-1$
//    properties.put(Constants.SUPPORTS_FRAMEWORK_REQUIREBUNDLE, "true"); //$NON-NLS-1$
//  }
//
//  private int parseVersionInt(String value) {
//    try {
//      return Integer.parseInt(value);
//    } catch (NumberFormatException e) {
//      // try up to the first non-number char
//      StringBuffer sb = new StringBuffer(value.length());
//      char[] chars = value.toCharArray();
//      for (int i = 0; i < chars.length; i++) {
//        if (!Character.isDigit(chars[i]))
//          break;
//        sb.append(chars[i]);
//      }
//      if (sb.length() > 0)
//        return Integer.parseInt(sb.toString());
//      return 0;
//    }
//  }
//
//
//
//  private URL findInSystemBundle(String entry) {
//    URL result = systemBundle.getEntry0(entry);
//    if (result == null) {
//      // Check the ClassLoader in case we're launched off the Java boot classpath
//      ClassLoader loader = getClass().getClassLoader();
//      result = loader == null ? ClassLoader.getSystemResource(entry) : loader.getResource(entry);
//    }
//    return result;
//  }
//
//  /**
//   * This method return the state of the framework.
//   *
//   */
//  protected boolean isActive() {
//    return (active);
//  }
//
//  /**
//   * This method is called to destory the framework instance.
//   *
//   */
//  public synchronized void close() {
//    if (adaptor == null)
//      return;
//    if (active)
//      shutdown(FrameworkEvent.STOPPED);
//
//    synchronized (bundles) {
//      List allBundles = bundles.getBundles();
//      int size = allBundles.size();
//      for (int i = 0; i < size; i++) {
//        AbstractBundle bundle = (AbstractBundle) allBundles.get(i);
//        bundle.close();
//      }
//      bundles.removeAllBundles();
//    }
//    serviceRegistry = null;
//    if (bundleEvent != null) {
//      bundleEvent.clear();
//      bundleEvent = null;
//    }
//    if (bundleEventSync != null) {
//      bundleEventSync.clear();
//      bundleEventSync = null;
//    }
//    if (frameworkEvent != null) {
//      frameworkEvent.clear();
//      frameworkEvent = null;
//    }
//    if (eventManager != null) {
//      eventManager.close();
//      eventManager = null;
//    }
//    secureAction = null;
//    packageAdmin = null;
//    adaptor = null;
//    uninstallURLStreamHandlerFactory();
//    uninstallContentHandlerFactory();
//    if (System.getSecurityManager() instanceof EquinoxSecurityManager)
//      System.setSecurityManager(null);
//  }
//
//  /**
//   * Start the framework.
//   *
//   * When the framework is started. The following actions occur: 1. Event
//   * handling is enabled. Events can now be delivered to listeners. 2. All
//   * bundles which are recorded as started are started as described in the
//   * Bundle.start() method. These bundles are the bundles that were started
//   * when the framework was last stopped. Reports any exceptions that occur
//   * during startup using FrameworkEvents. 3. A FrameworkEvent of type
//   * FrameworkEvent.STARTED is broadcast.
//   *
//   */
//  public synchronized void launch() {
//    /* Return if framework already started */
//    if (active) {
//      return;
//    }
//    /* mark framework as started */
//    active = true;
//    shutdownEvent = new FrameworkEvent[1];
//    if (THREAD_NORMAL.equals(FrameworkProperties.getProperty(PROP_FRAMEWORK_THREAD))) {
//      Thread fwkThread = new Thread(this, "Framework Active Thread"); //$NON-NLS-1$
//      fwkThread.setDaemon(false);
//      fwkThread.start();
//    }
//    /* Resume systembundle */
//    if (Debug.DEBUG && Debug.DEBUG_GENERAL) {
//      Debug.println("Trying to launch framework"); //$NON-NLS-1$
//    }
//    systemBundle.resume();
//    signedContentFactory = new ServiceTracker(systemBundle.getBundleContext(), SignedContentFactory.class.getName(), null);
//    signedContentFactory.open();
//  }
//
//  /**
//   * Stop the framework.
//   *
//   * When the framework is stopped. The following actions occur: 1. Suspend
//   * all started bundles as described in the Bundle.stop method except that
//   * the bundle is recorded as started. These bundles will be restarted when
//   * the framework is next started. Reports any exceptions that occur during
//   * stopping using FrameworkEvents. 2. Event handling is disabled.
//   *
//   */
//  public synchronized void shutdown(int eventType) {
//    /* Return if framework already stopped */
//    if (!active)
//      return;
//    this.shutdownEvent[0] = new FrameworkEvent(eventType, systemBundle, null);
//    /*
//     * set the state of the System Bundle to STOPPING.
//     * this must be done first according to section 4.19.2 from the OSGi R3 spec.
//     */
//    systemBundle.state = AbstractBundle.STOPPING;
//    publishBundleEvent(BundleEvent.STOPPING, systemBundle); // need to send system bundle stopping event
//    /* call the FrameworkAdaptor.frameworkStopping method first */
//    try {
//      adaptor.frameworkStopping(systemBundle.getContext());
//    } catch (Throwable t) {
//      publishFrameworkEvent(FrameworkEvent.ERROR, systemBundle, t);
//    }
//    /* Suspend systembundle */
//    if (Debug.DEBUG && Debug.DEBUG_GENERAL) {
//      Debug.println("Trying to shutdown Framework"); //$NON-NLS-1$
//    }
//    systemBundle.suspend();
//    try {
//      adaptor.compactStorage();
//    } catch (IOException e) {
//      publishFrameworkEvent(FrameworkEvent.ERROR, systemBundle, e);
//    }
//    if (signedContentFactory != null)
//      signedContentFactory.close();
//    /* mark framework as stopped */
//    active = false;
//    notifyAll();
//  }
//
//  /**
//   * Create a new Bundle object.
//   *
//   * @param bundledata the BundleData of the Bundle to create
//   */
//  AbstractBundle createAndVerifyBundle(BundleData bundledata, boolean setBundle) throws BundleException {
//    // Check for a bundle already installed with the same symbolic name and version.
//    if (bundledata.getSymbolicName() != null) {
//      AbstractBundle installedBundle = getBundleBySymbolicName(bundledata.getSymbolicName(), bundledata.getVersion());
//      if (installedBundle != null && installedBundle.getBundleId() != bundledata.getBundleID()) {
//        String msg = NLS.bind(Msg.BUNDLE_INSTALL_SAME_UNIQUEID, new Object[] {installedBundle.getSymbolicName(), installedBundle.getVersion().toString(), installedBundle.getLocation()});
//        throw new DuplicateBundleException(msg, installedBundle);
//      }
//    }
//    return AbstractBundle.createBundle(bundledata, this, setBundle);
//  }
//
//  private class DuplicateBundleException extends BundleException implements StatusException {
//    private static final long serialVersionUID = 135669822846323624L;
//    private final Bundle duplicate;
//
//    public DuplicateBundleException(String msg, Bundle duplicate) {
//      super(msg, BundleException.DUPLICATE_BUNDLE_ERROR);
//      this.duplicate = duplicate;
//    }
//
//    public Object getStatus() {
//      return duplicate;
//    }
//
//    public int getStatusCode() {
//      return StatusException.CODE_OK;
//    }
//
//  }
//
//  /**
//   * Retrieve the value of the named environment property. Values are
//   * provided for the following properties:
//   * <dl>
//   * <dt><code>org.osgi.framework.version</code>
//   * <dd>The version of the framework.
//   * <dt><code>org.osgi.framework.vendor</code>
//   * <dd>The vendor of this framework implementation.
//   * <dt><code>org.osgi.framework.language</code>
//   * <dd>The language being used. See ISO 639 for possible values.
//   * <dt><code>org.osgi.framework.os.name</code>
//   * <dd>The name of the operating system of the hosting computer.
//   * <dt><code>org.osgi.framework.os.version</code>
//   * <dd>The version number of the operating system of the hosting computer.
//   * <dt><code>org.osgi.framework.processor</code>
//   * <dd>The name of the processor of the hosting computer.
//   * </dl>
//   *
//   * <p>
//   * Note: These last four properties are used by the <code>Bundle-NativeCode</code>
//   * manifest header's matching algorithm for selecting native code.
//   *
//   * @param key
//   *            The name of the requested property.
//   * @return The value of the requested property, or <code>null</code> if
//   *         the property is undefined.
//   */
//  public String getProperty(String key) {
//    return properties.getProperty(key);
//  }
//
//  /**
//   * Retrieve the value of the named environment property. Values are
//   * provided for the following properties:
//   * <dl>
//   * <dt><code>org.osgi.framework.version</code>
//   * <dd>The version of the framework.
//   * <dt><code>org.osgi.framework.vendor</code>
//   * <dd>The vendor of this framework implementation.
//   * <dt><code>org.osgi.framework.language</code>
//   * <dd>The language being used. See ISO 639 for possible values.
//   * <dt><code>org.osgi.framework.os.name</code>
//   * <dd>The name of the operating system of the hosting computer.
//   * <dt><code>org.osgi.framework.os.version</code>
//   * <dd>The version number of the operating system of the hosting computer.
//   * <dt><code>org.osgi.framework.processor</code>
//   * <dd>The name of the processor of the hosting computer.
//   * </dl>
//   *
//   * <p>
//   * Note: These last four properties are used by the <code>Bundle-NativeCode</code>
//   * manifest header's matching algorithm for selecting native code.
//   *
//   * @param key
//   *            The name of the requested property.
//   * @param def
//   *            A default value is the requested property is not present.
//   * @return The value of the requested property, or the default value if the
//   *         property is undefined.
//   */
//  protected String getProperty(String key, String def) {
//    return properties.getProperty(key, def);
//  }
//
//  /**
//   * Set a system property.
//   *
//   * @param key
//   *            The name of the property to set.
//   * @param value
//   *            The value to set.
//   * @return The previous value of the property or null if the property was
//   *         not previously set.
//   */
//  protected Object setProperty(String key, String value) {
//    return properties.put(key, value);
//  }
//
//  /**
//   * Install a bundle from an InputStream.
//   *
//   * <p>
//   * This method performs all the steps listed in
//   * {@link #installBundle(java.lang.String)}, except the bundle's content
//   * will be read from the InputStream. The location identifier specified
//   * will be used as the identity of the bundle.
//   *
//   * @param location
//   *            The location identifier of the bundle to install.
//   * @param in
//   *            The InputStream from which the bundle will be read.  If null
//   *            then the location is used to get the bundle content.
//   * @return The Bundle of the installed bundle.
//   */
//  protected AbstractBundle installBundle(final String location, final InputStream in) throws BundleException {
//    if (Debug.DEBUG && Debug.DEBUG_GENERAL) {
//      Debug.println("install from inputstream: " + location + ", " + in); //$NON-NLS-1$ //$NON-NLS-2$
//    }
//    final AccessControlContext callerContext = AccessController.getContext();
//    return installWorker(location, new PrivilegedExceptionAction() {
//      public Object run() throws BundleException {
//        /* Map the InputStream or location to a URLConnection */
//        URLConnection source = in != null ? new BundleSource(in) : adaptor.mapLocationToURLConnection(location);
//        /* call the worker to install the bundle */
//        return installWorkerPrivileged(location, source, callerContext);
//      }
//    });
//  }
//
//  /**
//   * Worker method to install a bundle. It obtains the reservation for the
//   * location and calls the specified action.
//   *
//   * @param location
//   *            The location identifier of the bundle to install.
//   * @param action
//   *            A PrivilegedExceptionAction which calls the real worker.
//   * @return The {@link AbstractBundle}of the installed bundle.
//   * @exception BundleException
//   *                If the action throws an error.
//   */
//  protected AbstractBundle installWorker(String location, PrivilegedExceptionAction action) throws BundleException {
//    synchronized (installLock) {
//      while (true) {
//        /* Check that the bundle is not already installed. */
//        AbstractBundle bundle = getBundleByLocation(location);
//        /* If already installed, return bundle object */
//        if (bundle != null) {
//          return bundle;
//        }
//        Thread current = Thread.currentThread();
//        /* Check for and make reservation */
//        Thread reservation = (Thread) installLock.put(location, current);
//        /* if the location is not already reserved */
//        if (reservation == null) {
//          /* we have made the reservation and can continue */
//          break;
//        }
//        /* the location was already reserved */
//        /*
//         * If the reservation is held by the current thread, we have
//         * recursed to install the same bundle!
//         */
//        if (current.equals(reservation)) {
//          throw new BundleException(Msg.BUNDLE_INSTALL_RECURSION_EXCEPTION, BundleException.STATECHANGE_ERROR);
//        }
//        try {
//          /* wait for the reservation to be released */
//          installLock.wait();
//        } catch (InterruptedException e) {
//        }
//      }
//    }
//    /* Don't call adaptor while holding the install lock */
//    try {
//      AbstractBundle bundle = (AbstractBundle) AccessController.doPrivileged(action);
//      publishBundleEvent(BundleEvent.INSTALLED, bundle);
//      return bundle;
//    } catch (PrivilegedActionException e) {
//      if (e.getException() instanceof RuntimeException)
//        throw (RuntimeException) e.getException();
//      throw (BundleException) e.getException();
//    } finally {
//      synchronized (installLock) {
//        /* release reservation */
//        installLock.remove(location);
//        /* wake up all waiters */
//        installLock.notifyAll();
//      }
//    }
//  }
//
//  /**
//   * Worker method to install a bundle. It calls the FrameworkAdaptor object
//   * to install the bundle in persistent storage.
//   *
//   * @param location
//   *            The location identifier of the bundle to install.
//   * @param source
//   *            The URLConnection from which the bundle will be read.
//   * @return The {@link AbstractBundle}of the installed bundle.
//   * @exception BundleException
//   *                If the provided stream cannot be read.
//   */
//  protected AbstractBundle installWorkerPrivileged(String location, URLConnection source, AccessControlContext callerContext) throws BundleException {
//    BundleOperation storage = adaptor.installBundle(location, source);
//    final AbstractBundle bundle;
//    try {
//      BundleData bundledata = storage.begin();
//      bundle = createAndVerifyBundle(bundledata, true);
//      if (Debug.DEBUG) {
//        BundleWatcher bundleStats = adaptor.getBundleWatcher();
//        if (bundleStats != null)
//          bundleStats.watchBundle(bundle, BundleWatcher.START_INSTALLING);
//      }
//      try {
//        bundle.load();
//        if (System.getSecurityManager() != null) {
//          final boolean extension = (bundledata.getType() & (BundleData.TYPE_BOOTCLASSPATH_EXTENSION | BundleData.TYPE_FRAMEWORK_EXTENSION | BundleData.TYPE_EXTCLASSPATH_EXTENSION)) != 0;
//          // must check for AllPermission before allow a bundle extension to be installed
//          if (extension && !bundle.hasPermission(new AllPermission()))
//            throw new BundleException(Msg.BUNDLE_EXTENSION_PERMISSION, BundleException.SECURITY_ERROR, new SecurityException(Msg.BUNDLE_EXTENSION_PERMISSION));
//          try {
//            AccessController.doPrivileged(new PrivilegedExceptionAction() {
//              public Object run() throws Exception {
//                checkAdminPermission(bundle, AdminPermission.LIFECYCLE);
//                if (extension) // need special permission to install extension bundles
//                  checkAdminPermission(bundle, AdminPermission.EXTENSIONLIFECYCLE);
//                return null;
//              }
//            }, callerContext);
//          } catch (PrivilegedActionException e) {
//            throw e.getException();
//          }
//        }
//        storage.commit(false);
//      } catch (Throwable error) {
//        synchronized (bundles) {
//          bundle.unload();
//        }
//        bundle.close();
//        throw error;
//      } finally {
//        if (Debug.DEBUG) {
//          BundleWatcher bundleStats = adaptor.getBundleWatcher();
//          if (bundleStats != null)
//            bundleStats.watchBundle(bundle, BundleWatcher.END_INSTALLING);
//        }
//      }
//      /* bundle has been successfully installed */
//      bundles.add(bundle);
//    } catch (Throwable t) {
//      try {
//        storage.undo();
//      } catch (BundleException ee) {
//        publishFrameworkEvent(FrameworkEvent.ERROR, systemBundle, ee);
//      }
//      if (t instanceof SecurityException)
//        throw (SecurityException) t;
//      if (t instanceof BundleException)
//        throw (BundleException) t;
//      throw new BundleException(t.getMessage(), t);
//    }
//    return bundle;
//  }
//
//  /**
//   * Retrieve the bundle that has the given unique identifier.
//   *
//   * @param id
//   *            The identifier of the bundle to retrieve.
//   * @return A {@link AbstractBundle}object, or <code>null</code> if the
//   *         identifier doesn't match any installed bundle.
//   */
//  // changed visibility to gain access through the adaptor
//  public AbstractBundle getBundle(long id) {
//    synchronized (bundles) {
//      return bundles.getBundle(id);
//    }
//  }
//
//  public BundleContextImpl getSystemBundleContext() {
//    if (systemBundle == null)
//      return null;
//    return systemBundle.context;
//  }
//
//  public PackageAdminImpl getPackageAdmin() {
//    return packageAdmin;
//  }
//
//  /**
//   * Retrieve the bundle that has the given symbolic name and version.
//   *
//   * @param symbolicName
//   *            The symbolic name of the bundle to retrieve
//   * @param version The version of the bundle to retrieve
//   * @return A {@link AbstractBundle}object, or <code>null</code> if the
//   *         identifier doesn't match any installed bundle.
//   */
//  public AbstractBundle getBundleBySymbolicName(String symbolicName, Version version) {
//    synchronized (bundles) {
//      return bundles.getBundle(symbolicName, version);
//    }
//  }
//
//  /**
//   * Retrieve the BundleRepository of all installed bundles. The list is
//   * valid at the time of the call to getBundles, but the framework is a very
//   * dynamic environment and bundles can be installed or uninstalled at
//   * anytime.
//   *
//   * @return The BundleRepository.
//   */
//  protected BundleRepository getBundles() {
//    return (bundles);
//  }
//
//  /**
//   * Retrieve a list of all installed bundles. The list is valid at the time
//   * of the call to getBundleAlls, but the framework is a very dynamic
//   * environment and bundles can be installed or uninstalled at anytime.
//   *
//   * @return An Array of {@link AbstractBundle}objects, one object per installed
//   *         bundle.
//   */
//  protected AbstractBundle[] getAllBundles() {
//    synchronized (bundles) {
//      List allBundles = bundles.getBundles();
//      int size = allBundles.size();
//      if (size == 0) {
//        return (null);
//      }
//      AbstractBundle[] bundlelist = new AbstractBundle[size];
//      allBundles.toArray(bundlelist);
//      return (bundlelist);
//    }
//  }
//
//  /**
//   * Resume a bundle.
//   *
//   * @param bundle
//   *            Bundle to resume.
//   */
//  protected void resumeBundle(AbstractBundle bundle) {
//    if (bundle.isActive()) {
//      // if bundle is active.
//      return;
//    }
//    try {
//      if (Debug.DEBUG && Debug.DEBUG_GENERAL) {
//        Debug.println("Trying to resume bundle " + bundle); //$NON-NLS-1$
//      }
//      bundle.resume();
//    } catch (BundleException be) {
//      if (Debug.DEBUG && Debug.DEBUG_GENERAL) {
//        Debug.println("Bundle resume exception: " + be.getMessage()); //$NON-NLS-1$
//        Debug.printStackTrace(be.getNestedException() == null ? be : be.getNestedException());
//      }
//      publishFrameworkEvent(FrameworkEvent.ERROR, bundle, be);
//    }
//  }
//
//  /**
//   * Suspend a bundle.
//   *
//   * @param bundle
//   *            Bundle to suspend.
//   * @param lock
//   *            true if state change lock should be held when returning from
//   *            this method.
//   * @return true if bundle was active and is now suspended.
//   */
//  protected boolean suspendBundle(AbstractBundle bundle, boolean lock) {
//    boolean changed = false;
//    if (!bundle.isActive() || bundle.isFragment()) {
//      // if bundle is not active or is a fragment then do nothing.
//      return changed;
//    }
//    try {
//      if (Debug.DEBUG && Debug.DEBUG_GENERAL) {
//        Debug.println("Trying to suspend bundle " + bundle); //$NON-NLS-1$
//      }
//      bundle.suspend(lock);
//    } catch (BundleException be) {
//      if (Debug.DEBUG && Debug.DEBUG_GENERAL) {
//        Debug.println("Bundle suspend exception: " + be.getMessage()); //$NON-NLS-1$
//        Debug.printStackTrace(be.getNestedException() == null ? be : be.getNestedException());
//      }
//      publishFrameworkEvent(FrameworkEvent.ERROR, bundle, be);
//    }
//    if (!bundle.isActive()) {
//      changed = true;
//    }
//    return (changed);
//  }
//
//  /**
//   * Locate an installed bundle with a given identity.
//   *
//   * @param location
//   *            string for the bundle
//   * @return Bundle object for bundle with the specified location or null if
//   *         no bundle is installed with the specified location.
//   */
//  protected AbstractBundle getBundleByLocation(String location) {
//    synchronized (bundles) {
//      // this is not optimized; do not think it will get called
//      // that much.
//      final String finalLocation = location;
//
//      //Bundle.getLocation requires AdminPermission (metadata)
//      return (AbstractBundle) AccessController.doPrivileged(new PrivilegedAction() {
//        public Object run() {
//          List allBundles = bundles.getBundles();
//          int size = allBundles.size();
//          for (int i = 0; i < size; i++) {
//            AbstractBundle bundle = (AbstractBundle) allBundles.get(i);
//            if (finalLocation.equals(bundle.getLocation())) {
//              return (bundle);
//            }
//          }
//          return (null);
//        }
//      });
//    }
//  }
//
//  /**
//   * Locate an installed bundle with a given symbolic name
//   *
//   * @param symbolicName
//   *            The symbolic name for the bundle
//   * @return Bundle object for bundle with the specified Unique or null if no
//   *         bundle is installed with the specified location.
//   */
//  protected AbstractBundle[] getBundleBySymbolicName(String symbolicName) {
//    synchronized (bundles) {
//      return bundles.getBundles(symbolicName);
//    }
//  }
//
//  /**
//   * Creates a <code>File</code> object for a file in the persistent
//   * storage area provided for the bundle by the framework. If the adaptor
//   * does not have file system support, this method will return <code>null</code>.
//   *
//   * <p>
//   * A <code>File</code> object for the base directory of the persistent
//   * storage area provided for the context bundle by the framework can be
//   * obtained by calling this method with the empty string ("") as the
//   * parameter.
//   */
//  protected File getDataFile(final AbstractBundle bundle, final String filename) {
//    return (File) AccessController.doPrivileged(new GetDataFileAction(bundle, filename));
//  }
//
//  /**
//   * Deliver a FrameworkEvent.
//   *
//   * @param type
//   *            FrameworkEvent type.
//   * @param bundle
//   *            Affected bundle or null for system bundle.
//   * @param throwable
//   *            Related exception or null.
//   */
//  public void publishFrameworkEvent(int type, org.osgi.framework.Bundle bundle, Throwable throwable) {
//    if (frameworkEvent != null) {
//      if (bundle == null)
//        bundle = systemBundle;
//      final FrameworkEvent event = new FrameworkEvent(type, bundle, throwable);
//      if (System.getSecurityManager() == null) {
//        publishFrameworkEventPrivileged(event);
//      } else {
//        AccessController.doPrivileged(new PrivilegedAction() {
//          public Object run() {
//            publishFrameworkEventPrivileged(event);
//            return null;
//          }
//        });
//      }
//    }
//  }
//
//  public void publishFrameworkEventPrivileged(FrameworkEvent event) {
//    /* if the event is an error then it should be logged */
//    if (event.getType() == FrameworkEvent.ERROR) {
//      FrameworkLog frameworkLog = adaptor.getFrameworkLog();
//      if (frameworkLog != null)
//        frameworkLog.log(event);
//    }
//    /* queue to hold set of listeners */
//    ListenerQueue listeners = new ListenerQueue(eventManager);
//    /* queue to hold set of BundleContexts w/ listeners */
//    ListenerQueue contexts = new ListenerQueue(eventManager);
//    /* synchronize while building the listener list */
//    synchronized (frameworkEvent) {
//      /* add set of BundleContexts w/ listeners to queue */
//      contexts.queueListeners(frameworkEvent.entrySet(), this);
//      /* synchronously dispatch to populate listeners queue */
//      contexts.dispatchEventSynchronous(FRAMEWORKEVENT, listeners);
//    }
//    /* dispatch event to set of listeners */
//    listeners.dispatchEventAsynchronous(FRAMEWORKEVENT, event);
//  }
//
//  /**
//   * Deliver a BundleEvent to SynchronousBundleListeners (synchronous). and
//   * BundleListeners (asynchronous).
//   *
//   * @param type
//   *            BundleEvent type.
//   * @param bundle
//   *            Affected bundle or null.
//   */
//  public void publishBundleEvent(int type, org.osgi.framework.Bundle bundle) {
//    if ((bundleEventSync != null) || (bundleEvent != null)) {
//      final BundleEvent event = new BundleEvent(type, bundle);
//      if (System.getSecurityManager() == null) {
//        publishBundleEventPrivileged(event);
//      } else {
//        AccessController.doPrivileged(new PrivilegedAction() {
//          public Object run() {
//            publishBundleEventPrivileged(event);
//            return null;
//          }
//        });
//      }
//    }
//  }
//
//  public void publishBundleEventPrivileged(BundleEvent event) {
//    /*
//     * We must collect the snapshots of the sync and async listeners
//     * BEFORE we dispatch the event.
//     */
//    /* Collect snapshot of SynchronousBundleListeners */
//    ListenerQueue listenersSync = null;
//    if (bundleEventSync != null) {
//      /* queue to hold set of listeners */
//      listenersSync = new ListenerQueue(eventManager);
//      /* queue to hold set of BundleContexts w/ listeners */
//      ListenerQueue contexts = new ListenerQueue(eventManager);
//      /* synchronize while building the listener list */
//      synchronized (bundleEventSync) {
//        /* add set of BundleContexts w/ listeners to queue */
//        contexts.queueListeners(bundleEventSync.entrySet(), this);
//        /* synchronously dispatch to populate listeners queue */
//        contexts.dispatchEventSynchronous(BUNDLEEVENTSYNC, listenersSync);
//      }
//    }
//    /* Collect snapshot of BundleListeners; only if the event is NOT STARTING or STOPPING or LAZY_ACTIVATION */
//    ListenerQueue listenersAsync = null;
//    if (bundleEvent != null && (event.getType() & (BundleEvent.STARTING | BundleEvent.STOPPING | BundleEvent.LAZY_ACTIVATION)) == 0) {
//      /* queue to hold set of listeners */
//      listenersAsync = new ListenerQueue(eventManager);
//      /* queue to hold set of BundleContexts w/ listeners */
//      ListenerQueue contexts = new ListenerQueue(eventManager);
//      /* synchronize while building the listener list */
//      synchronized (bundleEvent) {
//        /* add set of BundleContexts w/ listeners to queue */
//        contexts.queueListeners(bundleEvent.entrySet(), this);
//        /* synchronously dispatch to populate listeners queue */
//        contexts.dispatchEventSynchronous(BUNDLEEVENT, listenersAsync);
//      }
//    }
//    /* Dispatch BundleEvent to SynchronousBundleListeners */
//    if (listenersSync != null) {
//      listenersSync.dispatchEventSynchronous(BUNDLEEVENTSYNC, event);
//    }
//    /* Dispatch BundleEvent to BundleListeners */
//    if (listenersAsync != null) {
//      listenersAsync.dispatchEventAsynchronous(BUNDLEEVENT, event);
//    }
//  }

public: ListenerQueue NewListenerQueue();

//  /**
//   * Top level event dispatcher for the framework.
//   *
//   * @param l
//   *            BundleContext for receiving bundle
//   * @param lo
//   *            BundleContext for receiving bundle
//   * @param action
//   *            Event class type
//   * @param object
//   *            ListenerQueue to populate
//   */
//  public void dispatchEvent(Object l, Object lo, int action, Object object) {
//    try {
//      BundleContextImpl context = (BundleContextImpl) l;
//      if (context.isValid()) /* if context still valid */{
//        ListenerQueue queue = (ListenerQueue) object;
//        switch (action) {
//          case BUNDLEEVENT : {
//            queue.queueListeners(context.bundleEvent.entrySet(), context);
//            break;
//          }
//          case BUNDLEEVENTSYNC : {
//            queue.queueListeners(context.bundleEventSync.entrySet(), context);
//            break;
//          }
//          case FRAMEWORKEVENT : {
//            queue.queueListeners(context.frameworkEvent.entrySet(), context);
//            break;
//          }
//          default : {
//            throw new InternalError();
//          }
//        }
//      }
//    } catch (Throwable t) {
//      if (Debug.DEBUG && Debug.DEBUG_GENERAL) {
//        Debug.println("Exception in Top level event dispatcher: " + t.getMessage()); //$NON-NLS-1$
//        Debug.printStackTrace(t);
//      }
//      // allow the adaptor to handle this unexpected error
//      adaptor.handleRuntimeError(t);
//      publisherror: {
//        if (action == FRAMEWORKEVENT) {
//          FrameworkEvent event = (FrameworkEvent) object;
//          if (event.getType() == FrameworkEvent.ERROR) {
//            break publisherror; /* avoid infinite loop */
//          }
//        }
//        BundleContextImpl context = (BundleContextImpl) l;
//        publishFrameworkEvent(FrameworkEvent.ERROR, context.bundle, t);
//      }
//    }
//  }
//
//  private void initializeContextFinder() {
//    Thread current = Thread.currentThread();
//    try {
//      ClassLoader parent = null;
//      // check property for specified parent
//      String type = FrameworkProperties.getProperty(PROP_CONTEXTCLASSLOADER_PARENT);
//      if (CONTEXTCLASSLOADER_PARENT_APP.equals(type))
//        parent = ClassLoader.getSystemClassLoader();
//      else if (CONTEXTCLASSLOADER_PARENT_BOOT.equals(type))
//        parent = null;
//      else if (CONTEXTCLASSLOADER_PARENT_FWK.equals(type))
//        parent = Framework.class.getClassLoader();
//      else if (CONTEXTCLASSLOADER_PARENT_EXT.equals(type)) {
//        ClassLoader appCL = ClassLoader.getSystemClassLoader();
//        if (appCL != null)
//          parent = appCL.getParent();
//      } else { // default is ccl (null or any other value will use ccl)
//        parent = current.getContextClassLoader();
//      }
//      contextFinder = new ContextFinder(parent);
//      current.setContextClassLoader(contextFinder);
//      return;
//    } catch (Exception e) {
//      FrameworkLogEntry entry = new FrameworkLogEntry(FrameworkAdaptor.FRAMEWORK_SYMBOLICNAME, FrameworkLogEntry.INFO, 0, NLS.bind(Msg.CANNOT_SET_CONTEXTFINDER, null), 0, e, null);
//      adaptor.getFrameworkLog().log(entry);
//    }
//
//  }
//
//  public static Field getField(Class clazz, Class type, boolean instance) {
//    Field[] fields = clazz.getDeclaredFields();
//    for (int i = 0; i < fields.length; i++) {
//      boolean isStatic = Modifier.isStatic(fields[i].getModifiers());
//      if (instance != isStatic && fields[i].getType().equals(type)) {
//        fields[i].setAccessible(true);
//        return fields[i];
//      }
//    }
//    return null;
//  }
//
//  private void installContentHandlerFactory(BundleContext context, FrameworkAdaptor frameworkAdaptor) {
//    ContentHandlerFactory chf = new ContentHandlerFactory(context, frameworkAdaptor);
//    try {
//      // first try the standard way
//      URLConnection.setContentHandlerFactory(chf);
//    } catch (Error err) {
//      // ok we failed now use more drastic means to set the factory
//      try {
//        forceContentHandlerFactory(chf);
//      } catch (Exception ex) {
//        // this is unexpected, log the exception and throw the original error
//        adaptor.getFrameworkLog().log(new FrameworkEvent(FrameworkEvent.ERROR, context.getBundle(), ex));
//        throw err;
//      }
//    }
//    contentHandlerFactory = chf;
//  }
//
//  private static void forceContentHandlerFactory(ContentHandlerFactory chf) throws Exception {
//    Field factoryField = getField(URLConnection.class, java.net.ContentHandlerFactory.class, false);
//    if (factoryField == null)
//      throw new Exception("Could not find ContentHandlerFactory field"); //$NON-NLS-1$
//    synchronized (URLConnection.class) {
//      java.net.ContentHandlerFactory factory = (java.net.ContentHandlerFactory) factoryField.get(null);
//      // doing a null check here just in case, but it would be really strange if it was null,
//      // because we failed to set the factory normally!!
//
//      if (factory != null) {
//        try {
//          factory.getClass().getMethod("isMultiplexing", null); //$NON-NLS-1$
//          Method register = factory.getClass().getMethod("register", new Class[] {Object.class}); //$NON-NLS-1$
//          register.invoke(factory, new Object[] {chf});
//        } catch (NoSuchMethodException e) {
//          // current factory does not support multiplexing, ok we'll wrap it
//          chf.setParentFactory(factory);
//          factory = chf;
//        }
//      }
//      // null out the field so that we can successfully call setContentHandlerFactory
//      factoryField.set(null, null);
//      // always attempt to clear the handlers cache
//      // This allows an optomization for the single framework use-case
//      resetContentHandlers();
//      URLConnection.setContentHandlerFactory(factory);
//    }
//  }
//
//  private void uninstallContentHandlerFactory() {
//    try {
//      Field factoryField = getField(URLConnection.class, java.net.ContentHandlerFactory.class, false);
//      if (factoryField == null)
//        return; // oh well, we tried.
//      synchronized (URLConnection.class) {
//        java.net.ContentHandlerFactory factory = (java.net.ContentHandlerFactory) factoryField.get(null);
//
//        if (factory == contentHandlerFactory) {
//          factory = (java.net.ContentHandlerFactory) contentHandlerFactory.designateSuccessor();
//        } else {
//          Method unregister = factory.getClass().getMethod("unregister", new Class[] {Object.class}); //$NON-NLS-1$
//          unregister.invoke(factory, new Object[] {contentHandlerFactory});
//        }
//        // null out the field so that we can successfully call setContentHandlerFactory
//        factoryField.set(null, null);
//        // always attempt to clear the handlers cache
//        // This allows an optomization for the single framework use-case
//        // Note that the call to setContentHandlerFactory below may clear this cache
//        // but we want to be sure to clear it here just incase the parent is null.
//        // In this case the call below would not occur.
//        // Also it appears most java libraries actually do not clear the cache
//        // when setContentHandlerFactory is called, go figure!!
//        resetContentHandlers();
//        if (factory != null)
//          URLConnection.setContentHandlerFactory(factory);
//      }
//    } catch (Exception e) {
//      // ignore and continue closing the framework
//    }
//  }
//
//  private static void resetContentHandlers() throws IllegalAccessException {
//    Field handlersField = getField(URLConnection.class, Hashtable.class, false);
//    if (handlersField != null) {
//      Hashtable handlers = (Hashtable) handlersField.get(null);
//      if (handlers != null)
//        handlers.clear();
//    }
//  }
//
//  private void installURLStreamHandlerFactory(BundleContext context, FrameworkAdaptor frameworkAdaptor) {
//    StreamHandlerFactory shf = new StreamHandlerFactory(context, frameworkAdaptor);
//    try {
//      // first try the standard way
//      URL.setURLStreamHandlerFactory(shf);
//    } catch (Error err) {
//      try {
//        // ok we failed now use more drastic means to set the factory
//        forceURLStreamHandlerFactory(shf);
//      } catch (Exception ex) {
//        adaptor.getFrameworkLog().log(new FrameworkEvent(FrameworkEvent.ERROR, context.getBundle(), ex));
//        throw err;
//      }
//    }
//    streamHandlerFactory = shf;
//  }
//
//  private static void forceURLStreamHandlerFactory(StreamHandlerFactory shf) throws Exception {
//    Field factoryField = getField(URL.class, URLStreamHandlerFactory.class, false);
//    if (factoryField == null)
//      throw new Exception("Could not find URLStreamHandlerFactory field"); //$NON-NLS-1$
//    // look for a lock to synchronize on
//    Object lock = getURLStreamHandlerFactoryLock();
//    synchronized (lock) {
//      URLStreamHandlerFactory factory = (URLStreamHandlerFactory) factoryField.get(null);
//      // doing a null check here just in case, but it would be really strange if it was null,
//      // because we failed to set the factory normally!!
//      if (factory != null) {
//        try {
//          factory.getClass().getMethod("isMultiplexing", null); //$NON-NLS-1$
//          Method register = factory.getClass().getMethod("register", new Class[] {Object.class}); //$NON-NLS-1$
//          register.invoke(factory, new Object[] {shf});
//        } catch (NoSuchMethodException e) {
//          // current factory does not support multiplexing, ok we'll wrap it
//          shf.setParentFactory(factory);
//          factory = shf;
//        }
//      }
//      factoryField.set(null, null);
//      // always attempt to clear the handlers cache
//      // This allows an optomization for the single framework use-case
//      resetURLStreamHandlers();
//      URL.setURLStreamHandlerFactory(factory);
//    }
//  }
//
//  private void uninstallURLStreamHandlerFactory() {
//    try {
//      Field factoryField = getField(URL.class, URLStreamHandlerFactory.class, false);
//      if (factoryField == null)
//        return; // oh well, we tried
//      Object lock = getURLStreamHandlerFactoryLock();
//      synchronized (lock) {
//        URLStreamHandlerFactory factory = (URLStreamHandlerFactory) factoryField.get(null);
//        if (factory == streamHandlerFactory) {
//          factory = (URLStreamHandlerFactory) streamHandlerFactory.designateSuccessor();
//        } else {
//          Method unregister = factory.getClass().getMethod("unregister", new Class[] {Object.class}); //$NON-NLS-1$
//          unregister.invoke(factory, new Object[] {streamHandlerFactory});
//        }
//        factoryField.set(null, null);
//        // always attempt to clear the handlers cache
//        // This allows an optomization for the single framework use-case
//        // Note that the call to setURLStreamHandlerFactory below may clear this cache
//        // but we want to be sure to clear it here just in case the parent is null.
//        // In this case the call below would not occur.
//        resetURLStreamHandlers();
//        if (factory != null)
//          URL.setURLStreamHandlerFactory(factory);
//      }
//    } catch (Exception e) {
//      // ignore and continue closing the framework
//    }
//  }
//
//  private static Object getURLStreamHandlerFactoryLock() throws IllegalAccessException {
//    Object lock;
//    try {
//      Field streamHandlerLockField = URL.class.getDeclaredField("streamHandlerLock"); //$NON-NLS-1$
//      streamHandlerLockField.setAccessible(true);
//      lock = streamHandlerLockField.get(null);
//    } catch (NoSuchFieldException noField) {
//      // could not find the lock, lets sync on the class object
//      lock = URL.class;
//    }
//    return lock;
//  }
//
//  private static void resetURLStreamHandlers() throws IllegalAccessException {
//    Field handlersField = getField(URL.class, Hashtable.class, false);
//    if (handlersField != null) {
//      Hashtable handlers = (Hashtable) handlersField.get(null);
//      if (handlers != null)
//        handlers.clear();
//    }
//  }
//
//  /*
//   * (non-Javadoc)
//   * @see java.lang.Runnable#run()
//   * This thread monitors the framework active status and terminates when the framework is
//   * shutdown.  This is needed to ensure the VM does not exist because of the lack of a
//   * non-daemon thread (bug 215730)
//   */
//  public void run() {
//    synchronized (this) {
//      while (active)
//        try {
//          this.wait(1000);
//        } catch (InterruptedException e) {
//          // do nothing
//        }
//    }
//  }
//
//  void setForcedRestart(boolean forcedRestart) {
//    this.forcedRestart = forcedRestart;
//  }
//
//  boolean isForcedRestart() {
//    return forcedRestart;
//  }
//
//  public FrameworkEvent waitForStop(long timeout) throws InterruptedException {
//    boolean waitForEver = timeout == 0;
//    long start = System.currentTimeMillis();
//    long timeLeft = timeout;
//    synchronized (this) {
//      FrameworkEvent[] event = shutdownEvent;
//      while (event != null && event[0] == null) {
//        this.wait(timeLeft);
//        if (!waitForEver) {
//          timeLeft = start + timeout - System.currentTimeMillis();
//          // break if we are passed the timeout
//          if (timeLeft <= 0)
//            break;
//        }
//      }
//      if (event == null || event[0] == null)
//        return new FrameworkEvent(FrameworkEvent.WAIT_TIMEDOUT, systemBundle, null);
//      return event[0];
//    }
//  }
//
//
//
//  SignedContentFactory getSignedContentFactory() {
//    ServiceTracker currentTracker = signedContentFactory;
//    return (SignedContentFactory) (currentTracker == null ? null : currentTracker.getService());
//  }
//
//  ContextFinder getContextFinder() {
//    return contextFinder;
//  }
};

}
}
}

#endif /* CHERRYFRAMEWORK_H_ */
