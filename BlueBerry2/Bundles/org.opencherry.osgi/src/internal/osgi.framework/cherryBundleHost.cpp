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

#include "cherryBundleHost.h"

namespace cherry {
namespace internal {
namespace osgi {

  BundleHost::Pointer CreateBundle(BundleData bundledata,
      SmartPointer<Framework> framework, bool setBundle) throw(BundleException) {
    AbstractBundle result;
    if ((bundledata.getType() & BundleData.TYPE_COMPOSITEBUNDLE) > 0)
      result = new CompositeImpl(bundledata, framework);
    else if ((bundledata.getType() & BundleData.TYPE_SURROGATEBUNDLE) > 0)
      result = new SurrogateImpl(bundledata, framework);
    else
      result = new BundleHost(bundledata, framework);
    if (setBundle)
      bundledata.setBundle(result);
    return result;
  }

  void Load() {
    if (Debug.DEBUG && Debug.DEBUG_GENERAL) {
      if ((state & (INSTALLED)) == 0) {
        Debug.println("Bundle.load called when state != INSTALLED: " + this); //$NON-NLS-1$
        Debug.printStackTrace(new Exception("Stack trace")); //$NON-NLS-1$
      }
      if (proxy != null) {
        Debug.println("Bundle.load called when proxy != null: " + this); //$NON-NLS-1$
        Debug.printStackTrace(new Exception("Stack trace")); //$NON-NLS-1$
      }
    }

    if (framework.isActive()) {
      SecurityManager sm = System.getSecurityManager();
      if (sm != null && framework.securityAdmin != null) {
        domain = framework.securityAdmin.createProtectionDomain(this);
      }
    }
    proxy = null;
  }

  bool Reload(SmartPointer<BundleHost> newBundle) {
    if (Debug.DEBUG && Debug.DEBUG_GENERAL) {
      if ((state & (INSTALLED | RESOLVED)) == 0) {
        Debug.println("Bundle.reload called when state != INSTALLED | RESOLVED: " + this); //$NON-NLS-1$
        Debug.printStackTrace(new Exception("Stack trace")); //$NON-NLS-1$
      }
    }

    boolean exporting = false;

    if (framework.isActive()) {
      if (state == RESOLVED) {
        BundleLoaderProxy curProxy = getLoaderProxy();
        exporting = curProxy.inUse();
        if (exporting) {
          // add the bundle data to the list of removals
          framework.packageAdmin.addRemovalPending(bundledata);
          // make sure the BundleLoader is created.
          curProxy.getBundleLoader().createClassLoader();
        } else
          BundleLoader.closeBundleLoader(proxy);
        state = INSTALLED;
        proxy = null;
        fragments = null;
      }

    } else {
      /* close the outgoing jarfile */
      try {
        this.bundledata.close();
      } catch (IOException e) {
        // Do Nothing
      }
    }
    this.bundledata = newBundle.bundledata;
    this.bundledata.setBundle(this);
    // create a new domain for the bundle because its signers/symbolic-name may have changed
    if (framework.isActive() && System.getSecurityManager() != null && framework.securityAdmin != null)
      domain = framework.securityAdmin.createProtectionDomain(this);
    return (exporting);
  }

  void Refresh() {
    if (Debug.DEBUG && Debug.DEBUG_GENERAL) {
      if ((state & (UNINSTALLED | INSTALLED | RESOLVED)) == 0) {
        Debug.println("Bundle.reload called when state != UNINSTALLED | INSTALLED | RESOLVED: " + this); //$NON-NLS-1$
        Debug.printStackTrace(new Exception("Stack trace")); //$NON-NLS-1$
      }
    }
    if (state == RESOLVED) {
      BundleLoader.closeBundleLoader(proxy);
      proxy = null;
      fragments = null;
      state = INSTALLED;
      // Do not publish UNRESOLVED event here.  This is done by caller
      // to resolve if appropriate.
    }
    manifestLocalization = null;
  }

  bool Unload() {
    if (Debug.DEBUG && Debug.DEBUG_GENERAL) {
      if ((state & (UNINSTALLED | INSTALLED | RESOLVED)) == 0) {
        Debug.println("Bundle.unload called when state != UNINSTALLED | INSTALLED | RESOLVED: " + this); //$NON-NLS-1$
        Debug.printStackTrace(new Exception("Stack trace")); //$NON-NLS-1$
      }
    }

    boolean exporting = false;

    if (framework.isActive()) {
      if (state == RESOLVED) {
        BundleLoaderProxy curProxy = getLoaderProxy();
        exporting = curProxy.inUse();
        if (exporting) {
          // add the bundle data to the list of removals
          framework.packageAdmin.addRemovalPending(bundledata);
          // make sure the BundleLoader is created.
          curProxy.getBundleLoader().createClassLoader();
        } else
          BundleLoader.closeBundleLoader(proxy);

        state = INSTALLED;
        proxy = null;
        fragments = null;
        domain = null;
      }
    }
    if (!exporting) {
      try {
        this.bundledata.close();
      } catch (IOException e) { // Do Nothing.
      }
    }

    return (exporting);
  }

  void Close() {
    if (Debug.DEBUG && Debug.DEBUG_GENERAL) {
      if ((state & (INSTALLED)) == 0) {
        Debug.println("Bundle.close called when state != INSTALLED: " + this); //$NON-NLS-1$
        Debug.printStackTrace(new Exception("Stack trace")); //$NON-NLS-1$
      }
    }
    state = UNINSTALLED;
  }

  SmartPointer<BundleActivator> LoadBundleActivator() throw(BundleException) {
    /* load Bundle's BundleActivator if it has one */
    String activatorClassName = bundledata.getActivator();
    if (activatorClassName != null) {
      try {
        Class activatorClass = loadClass(activatorClassName, false);
        /* Create the activator for the bundle */
        return (BundleActivator) (activatorClass.newInstance());
      } catch (Throwable t) {
        if (Debug.DEBUG && Debug.DEBUG_GENERAL) {
          Debug.printStackTrace(t);
        }
        throw new BundleException(NLS.bind(Msg.BUNDLE_INVALID_ACTIVATOR_EXCEPTION, activatorClassName, bundledata.getSymbolicName()), BundleException.ACTIVATOR_ERROR, t);
      }
    }
    return (null);
  }

  void StartWorker(int options) throw(BundleException) {
    if ((options & START_TRANSIENT) == 0) {
      setStatus(Constants.BUNDLE_STARTED, true);
      setStatus(Constants.BUNDLE_ACTIVATION_POLICY, (options & START_ACTIVATION_POLICY) != 0);
      if (Debug.DEBUG && Debug.MONITOR_ACTIVATION)
        new Exception("A persistent start has been called on bundle: " + getBundleData()).printStackTrace(); //$NON-NLS-1$
    }
    if (!framework.active || (state & ACTIVE) != 0)
      return;

    if (state == INSTALLED) {
      if (!framework.packageAdmin.resolveBundles(new Bundle[] {this}))
        throw getResolutionFailureException();
    }

    if (getStartLevel() > framework.startLevelManager.getStartLevel()) {
      if ((options & START_TRANSIENT) != 0) {
        // throw exception if this is a transient start
        String msg = NLS.bind(Msg.BUNDLE_TRANSIENT_START_ERROR, this);
        // Use a StatusException to indicate to the lazy starter that this should result in a warning
        throw new BundleException(msg, BundleException.INVALID_OPERATION, new BundleStatusException(msg, StatusException.CODE_WARNING, this));
      }
      return;
    }
    if ((options & START_ACTIVATION_POLICY) != 0 && (bundledata.getStatus() & Constants.BUNDLE_LAZY_START) != 0) {
      // the bundle must use the activation policy here.
      if ((state & RESOLVED) != 0) {
        // now we must publish the LAZY_ACTIVATION event and return
        state = STARTING;
        // release the state change lock before sending lazy activation event (bug 258659)
        completeStateChange();
        framework.publishBundleEvent(BundleEvent.LAZY_ACTIVATION, this);
      }
      return;
    }

    if (Debug.DEBUG && Debug.DEBUG_GENERAL) {
      Debug.println("Bundle: Active sl = " + framework.startLevelManager.getStartLevel() + "; Bundle " + getBundleId() + " sl = " + getStartLevel()); //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$
    }

    state = STARTING;
    framework.publishBundleEvent(BundleEvent.STARTING, this);
    context = getContext();
    //STARTUP TIMING Start here
    long start = 0;
    if (Debug.DEBUG) {
      BundleWatcher bundleStats = framework.adaptor.getBundleWatcher();
      if (bundleStats != null)
        bundleStats.watchBundle(this, BundleWatcher.START_ACTIVATION);
      if (Debug.DEBUG_BUNDLE_TIME) {
        start = System.currentTimeMillis();
        System.out.println("Starting " + getSymbolicName()); //$NON-NLS-1$
      }
    }
    try {
      context.start();
      startHook();
      if (framework.active) {
        state = ACTIVE;

        if (Debug.DEBUG && Debug.DEBUG_GENERAL) {
          Debug.println("->started " + this); //$NON-NLS-1$
        }
        // release the state change lock before sending lazy activation event (bug 258659)
        completeStateChange();
        framework.publishBundleEvent(BundleEvent.STARTED, this);
      }

    } catch (BundleException e) {
      // we must fire the stopping event
      state = STOPPING;
      framework.publishBundleEvent(BundleEvent.STOPPING, this);

      stopHook();
      context.close();
      context = null;

      state = RESOLVED;
      // if this is a lazy start bundle that fails to start then
      // we must fire the stopped event
      framework.publishBundleEvent(BundleEvent.STOPPED, this);
      throw e;
    } finally {
      if (Debug.DEBUG) {
        BundleWatcher bundleStats = framework.adaptor.getBundleWatcher();
        if (bundleStats != null)
          bundleStats.watchBundle(this, BundleWatcher.END_ACTIVATION);
        if (Debug.DEBUG_BUNDLE_TIME)
          System.out.println("End starting " + getSymbolicName() + " " + (System.currentTimeMillis() - start)); //$NON-NLS-1$ //$NON-NLS-2$
      }
    }

    if (state == UNINSTALLED) {
      context.close();
      context = null;
      throw new BundleException(NLS.bind(Msg.BUNDLE_UNINSTALLED_EXCEPTION, getBundleData().getLocation()), BundleException.STATECHANGE_ERROR);
    }
  }

 void StartHook() throw(BundleException) {
    // do nothing by default
  }

  bool ReadyToResume() {
    // Return false if the bundle is not at the correct start-level
    if (getStartLevel() > framework.startLevelManager.getStartLevel())
      return false;
    int status = bundledata.getStatus();
    // Return false if the bundle is not persistently marked for start
    if ((status & Constants.BUNDLE_STARTED) == 0)
      return false;
    if ((status & Constants.BUNDLE_ACTIVATION_POLICY) == 0 || (status & Constants.BUNDLE_LAZY_START) == 0)
      return true;
    if (!isResolved())
      // should never transition from UNRESOLVED -> STARTING
      return false;
    // now we can publish the LAZY_ACTIVATION event
    state = STARTING;
    // release the state change lock before sending lazy activation event (bug 258659)
    completeStateChange();
    framework.publishBundleEvent(BundleEvent.LAZY_ACTIVATION, this);
    return false;
  }

  SmartPointer<BundleContextImpl> CreateContext() {
    return (new BundleContextImpl(this));
  }

  synchronized SmartPointer<BundleContextImpl> GetContext() {
    if (context == null) {
      // only create the context if we are starting, active or stopping
      // this is so that SCR can get the context for lazy-start bundles
      if ((state & (STARTING | ACTIVE | STOPPING)) != 0)
        context = createContext();
    }
    return (context);
  }

  void StopWorker(int options) throw(BundleException) {
    if ((options & STOP_TRANSIENT) == 0) {
      setStatus(Constants.BUNDLE_STARTED, false);
      setStatus(Constants.BUNDLE_ACTIVATION_POLICY, false);
      if (Debug.DEBUG && Debug.MONITOR_ACTIVATION)
        new Exception("A persistent start has been called on bundle: " + getBundleData()).printStackTrace(); //$NON-NLS-1$
    }
    if (framework.active) {
      if ((state & (STOPPING | RESOLVED | INSTALLED)) != 0) {
        return;
      }
      if (Debug.DEBUG) {
        BundleWatcher bundleStats = framework.adaptor.getBundleWatcher();
        if (bundleStats != null)
          bundleStats.watchBundle(this, BundleWatcher.START_DEACTIVATION);
      }
      state = STOPPING;
      framework.publishBundleEvent(BundleEvent.STOPPING, this);
      try {
        // context may be null if a lazy-start bundle is STARTING
        if (context != null)
          context.stop();
      } finally {
        stopHook();
        if (context != null) {
          context.close();
          context = null;
        }

        checkValid();

        state = RESOLVED;

        if (Debug.DEBUG && Debug.DEBUG_GENERAL) {
          Debug.println("->stopped " + this); //$NON-NLS-1$
        }

        framework.publishBundleEvent(BundleEvent.STOPPED, this);
        if (Debug.DEBUG) {
          BundleWatcher bundleStats = framework.adaptor.getBundleWatcher();
          if (bundleStats != null)
            bundleStats.watchBundle(this, BundleWatcher.END_DEACTIVATION);
        }
      }
    }
  }

  void StopHook() throw(BundleException) {
    // do nothing
  }

  bool IsActive() {
    return ((state & (ACTIVE | STARTING)) != 0);
  }

  void Resume() throw(BundleException) {
    if (state == UNINSTALLED) {
      return;
    }
    beginStateChange();
    try {
      if (readyToResume())
        startWorker(START_TRANSIENT);
    } finally {
      completeStateChange();
    }
  }

  void SetStatus(int mask, bool state) {
    try {
      AccessController.doPrivileged(new PrivilegedExceptionAction() {
        public Object run() throws BundleException, IOException {
          int status = bundledata.getStatus();
          boolean test = ((status & mask) != 0);
          if (test != state) {
            bundledata.setStatus(state ? (status | mask) : (status & ~mask));
            bundledata.save();
          }
          return null;
        }
      });
    } catch (PrivilegedActionException pae) {
      framework.publishFrameworkEvent(FrameworkEvent.ERROR, this, pae.getException());
    }
  }

  void Suspend(bool lock) throw(BundleException) {
    if (state == UNINSTALLED) {
      return;
    }
    beginStateChange();
    try {
      stopWorker(STOP_TRANSIENT);
    } finally {
      if (!lock) {
        completeStateChange();
      }
    }
  }

  void UpdateWorker(PrivilegedExceptionAction action) throw(BundleException) {
    int previousState = 0;
    if (!isFragment())
      previousState = state;
    if ((previousState & (ACTIVE | STARTING)) != 0) {
      try {
        stopWorker(STOP_TRANSIENT);
      } catch (BundleException e) {
        framework.publishFrameworkEvent(FrameworkEvent.ERROR, this, e);
        if ((state & (ACTIVE | STARTING)) != 0) /* if the bundle is still active */{
          throw e;
        }
      }
    }
    try {
      AccessController.doPrivileged(action);
      framework.publishBundleEvent(BundleEvent.UPDATED, this);
    } catch (PrivilegedActionException pae) {
      if (pae.getException() instanceof RuntimeException)
        throw (RuntimeException) pae.getException();
      throw (BundleException) pae.getException();
    } finally {
      if ((previousState & (ACTIVE | STARTING)) != 0) {
        try {
          startWorker(START_TRANSIENT | ((previousState & STARTING) != 0 ? START_ACTIVATION_POLICY : 0));
        } catch (BundleException e) {
          framework.publishFrameworkEvent(FrameworkEvent.ERROR, this, e);
        }
      }
    }
  }

  void UpdateWorkerPrivileged(URLConnection source, AccessControlContext callerContext) throw(BundleException) {
    AbstractBundle oldBundle = AbstractBundle.createBundle(bundledata, framework, false);
    boolean reloaded = false;
    BundleOperation storage = framework.adaptor.updateBundle(this.bundledata, source);
    BundleRepository bundles = framework.getBundles();
    try {
      BundleData newBundleData = storage.begin();
      // Must call framework createBundle to check execution environment.
      final AbstractBundle newBundle = framework.createAndVerifyBundle(newBundleData, false);
      boolean exporting;
      int st = getState();
      synchronized (bundles) {
        String oldBSN = this.getSymbolicName();
        exporting = reload(newBundle);
        // update this to flush the old BSN/version etc.
        bundles.update(oldBSN, this);
        manifestLocalization = null;
      }
      // indicate we have loaded from the new version of the bundle
      reloaded = true;
      if (System.getSecurityManager() != null) {
        final boolean extension = (bundledata.getType() & (BundleData.TYPE_BOOTCLASSPATH_EXTENSION | BundleData.TYPE_FRAMEWORK_EXTENSION | BundleData.TYPE_EXTCLASSPATH_EXTENSION)) != 0;
        // must check for AllPermission before allow a bundle extension to be updated
        if (extension && !hasPermission(new AllPermission()))
          throw new BundleException(Msg.BUNDLE_EXTENSION_PERMISSION, BundleException.SECURITY_ERROR, new SecurityException(Msg.BUNDLE_EXTENSION_PERMISSION));
        try {
          AccessController.doPrivileged(new PrivilegedExceptionAction() {
            public Object run() throws Exception {
              framework.checkAdminPermission(newBundle, AdminPermission.LIFECYCLE);
              if (extension) // need special permission to update extension bundles
                framework.checkAdminPermission(newBundle, AdminPermission.EXTENSIONLIFECYCLE);
              return null;
            }
          }, callerContext);
        } catch (PrivilegedActionException e) {
          throw e.getException();
        }
      }
      // send out unresolved events outside synch block (defect #80610)
      if (st == RESOLVED)
        framework.publishBundleEvent(BundleEvent.UNRESOLVED, this);
      storage.commit(exporting);
    } catch (Throwable t) {
      try {
        storage.undo();
        if (reloaded)
        /*
         * if we loaded from the new version of the
         * bundle
         */{
          synchronized (bundles) {
            String oldBSN = this.getSymbolicName();
            reload(oldBundle);
            // update this to flush the new BSN/version back to the old one etc.
            bundles.update(oldBSN, this);
          }
        }
      } catch (BundleException ee) {
        /* if we fail to revert then we are in big trouble */
        framework.publishFrameworkEvent(FrameworkEvent.ERROR, this, ee);
      }
      if (t instanceof SecurityException)
        throw (SecurityException) t;
      if (t instanceof BundleException)
        throw (BundleException) t;
      throw new BundleException(t.getMessage(), t);
    }
  }

  void UninstallWorker(PrivilegedExceptionAction action) throw(BundleException) {
    boolean bundleActive = false;
    if (!isFragment())
      bundleActive = (state & (ACTIVE | STARTING)) != 0;
    if (bundleActive) {
      try {
        stopWorker(STOP_TRANSIENT);
      } catch (BundleException e) {
        framework.publishFrameworkEvent(FrameworkEvent.ERROR, this, e);
      }
    }
    try {
      AccessController.doPrivileged(action);
    } catch (PrivilegedActionException pae) {
      if (bundleActive) /* if we stopped the bundle */{
        try {
          startWorker(START_TRANSIENT);
        } catch (BundleException e) {
          /*
           * if we fail to start the original bundle then we are in
           * big trouble
           */
          framework.publishFrameworkEvent(FrameworkEvent.ERROR, this, e);
        }
      }
      throw (BundleException) pae.getException();
    }
    framework.publishBundleEvent(BundleEvent.UNINSTALLED, this);
  }

  void UninstallWorkerPrivileged() throw(BundleException) {
    if (Debug.DEBUG) {
      BundleWatcher bundleStats = framework.adaptor.getBundleWatcher();
      if (bundleStats != null)
        bundleStats.watchBundle(this, BundleWatcher.START_UNINSTALLING);
    }
    boolean unloaded = false;
    //cache the bundle's headers
    getHeaders();
    BundleOperation storage = framework.adaptor.uninstallBundle(this.bundledata);
    BundleRepository bundles = framework.getBundles();
    try {
      storage.begin();
      boolean exporting;
      int st = getState();
      synchronized (bundles) {
        bundles.remove(this); /* remove before calling unload */
        exporting = unload();
      }
      // send out unresolved events outside synch block (defect #80610)
      if (st == RESOLVED)
        framework.publishBundleEvent(BundleEvent.UNRESOLVED, this);
      unloaded = true;
      storage.commit(exporting);
      close();
    } catch (BundleException e) {
      try {
        storage.undo();
        if (unloaded) /* if we unloaded the bundle */{
          synchronized (bundles) {
            load(); /* reload the bundle */
            bundles.add(this);
          }
        }
      } catch (BundleException ee) {
        /*
         * if we fail to load the original bundle then we are in big
         * trouble
         */
        framework.publishFrameworkEvent(FrameworkEvent.ERROR, this, ee);
      }
      throw e;
    } finally {
      if (Debug.DEBUG) {
        BundleWatcher bundleStats = framework.adaptor.getBundleWatcher();
        if (bundleStats != null)
          bundleStats.watchBundle(this, BundleWatcher.END_UNINSTALLING);
      }
    }
  }

  void BeginStateChange() throw(BundleException) {
    synchronized (statechangeLock) {
      boolean doubleFault = false;
      while (true) {
        if (stateChanging == null) {
          stateChanging = Thread.currentThread();
          return;
        }
        if (doubleFault || (stateChanging == Thread.currentThread())) {
          throw new BundleException(NLS.bind(Msg.BUNDLE_STATE_CHANGE_EXCEPTION, getBundleData().getLocation(), stateChanging.getName()), BundleException.STATECHANGE_ERROR, new BundleStatusException(null, StatusException.CODE_WARNING, stateChanging));
        }
        try {
          long start = 0;
          if (Debug.DEBUG && Debug.DEBUG_GENERAL) {
            Debug.println(" Waiting for state to change in bundle " + this); //$NON-NLS-1$
            start = System.currentTimeMillis();
          }
          statechangeLock.wait(5000);
          /*
           * wait for other thread to
           * finish changing state
           */
          if (Debug.DEBUG && Debug.DEBUG_GENERAL) {
            long end = System.currentTimeMillis();
            if (end - start > 0)
              System.out.println("Waiting... : " + getSymbolicName() + ' ' + (end - start)); //$NON-NLS-1$
          }
        } catch (InterruptedException e) {
          //Nothing to do
        }
        doubleFault = true;
      }
    }
  }

  void CompleteStateChange() {
    synchronized (statechangeLock) {
      if (stateChanging == Thread.currentThread()) {
        stateChanging = null;
        statechangeLock.notify();
        /*
         * notify one waiting thread that the
         * state change is complete
         */
      }
    }
  }

  void CheckValid() {
    if (state == UNINSTALLED) {
      throw new IllegalStateException(NLS.bind(Msg.BUNDLE_UNINSTALLED_EXCEPTION, getBundleData().getLocation()));
    }
  }

  void Resolve() {
    if (Debug.DEBUG && Debug.DEBUG_GENERAL) {
      if ((state & (INSTALLED)) == 0) {
        Debug.println("Bundle.resolve called when state != INSTALLED: " + this); //$NON-NLS-1$
        Debug.printStackTrace(new Exception("Stack trace")); //$NON-NLS-1$
      }
    }
    if (state == INSTALLED) {
      state = RESOLVED;
      // Do not publish RESOLVED event here.  This is done by caller
      // to resolve if appropriate.
    }
  }

  BundleHost(BundleData bundledata, SmartPointer<Framework> framework) throw(BundleException) {
    state = INSTALLED;
    stateChanging = null;
    this.bundledata = bundledata;
    this.framework = framework;
    context = null;
    fragments = null;
  }

  std::istream* GetResource(const std::string& name) const throw(IllegalStateException) {
    BundleLoader loader = null;
    try {
      framework.checkAdminPermission(this, AdminPermission.RESOURCE);
    } catch (SecurityException ee) {
      return null;
    }
    loader = checkLoader();
    if (loader == null)
      return null;
    return (loader.findResource(name));
  }

  std::vector<SmartPointer<ServiceReference> > GetRegisteredServices() {
    checkValid();

    if (context == null) {
      return null;
    }

    return context.getFramework().getServiceRegistry().getRegisteredServices(context);
  }

  std::vector<SmartPointer<ServiceReference> > GetServicesInUse() {
    checkValid();

    if (context == null) {
      return null;
    }

    return context.getFramework().getServiceRegistry().getServicesInUse(context);
  }

  int GetState() {
    return (state);
  }

  SmartPointer<Framework> GetFramework() {
    return framework;
  }

  bool IsResolved() {
    return (state & (INSTALLED | UNINSTALLED)) == 0;
  }

  void Start(int options = 0) throw(BundleException, IllegalStateException) {
    framework.checkAdminPermission(this, AdminPermission.EXECUTE);
    checkValid();
    beginStateChange();
    try {
      startWorker(options);
    } finally {
      completeStateChange();
    }
  }

  void Stop(int options = 0) throw(BundleException, IllegalStateException) {
    framework.checkAdminPermission(this, AdminPermission.EXECUTE);
    checkValid();
    beginStateChange();
    try {
      stopWorker(options);
    } finally {
      completeStateChange();
    }
  }

  void Update(std::istream * const in = 0) throw(BundleException) {
    if (Debug.DEBUG && Debug.DEBUG_GENERAL) {
      Debug.println("update location " + bundledata.getLocation()); //$NON-NLS-1$
      Debug.println("   from: " + in); //$NON-NLS-1$
    }
    framework.checkAdminPermission(this, AdminPermission.LIFECYCLE);
    if ((bundledata.getType() & (BundleData.TYPE_BOOTCLASSPATH_EXTENSION | BundleData.TYPE_FRAMEWORK_EXTENSION | BundleData.TYPE_EXTCLASSPATH_EXTENSION)) != 0)
      // need special permission to update extensions
      framework.checkAdminPermission(this, AdminPermission.EXTENSIONLIFECYCLE);
    checkValid();
    beginStateChange();
    try {
      final AccessControlContext callerContext = AccessController.getContext();
      //note AdminPermission is checked again after updated bundle is loaded
      updateWorker(new PrivilegedExceptionAction() {
        public Object run() throws BundleException {
          /* compute the update location */
          URLConnection source = null;
          if (in == null) {
            String updateLocation = (String) bundledata.getManifest().get(Constants.BUNDLE_UPDATELOCATION);
            if (updateLocation == null)
              updateLocation = bundledata.getLocation();
            if (Debug.DEBUG && Debug.DEBUG_GENERAL)
              Debug.println("   from location: " + updateLocation); //$NON-NLS-1$
            /* Map the update location to a URLConnection */
            source = framework.adaptor.mapLocationToURLConnection(updateLocation);
          } else {
            /* Map the InputStream to a URLConnection */
            source = new BundleSource(in);
          }
          /* call the worker */
          updateWorkerPrivileged(source, callerContext);
          return null;
        }
      });
    } finally {
      completeStateChange();
    }
  }

  void Uninstall() throw(BundleException, IllegalStateException) {
    if (Debug.DEBUG && Debug.DEBUG_GENERAL) {
      Debug.println("uninstall location: " + bundledata.getLocation()); //$NON-NLS-1$
    }
    framework.checkAdminPermission(this, AdminPermission.LIFECYCLE);
    if ((bundledata.getType() & (BundleData.TYPE_BOOTCLASSPATH_EXTENSION | BundleData.TYPE_FRAMEWORK_EXTENSION | BundleData.TYPE_EXTCLASSPATH_EXTENSION)) != 0)
      // need special permission to uninstall extensions
      framework.checkAdminPermission(this, AdminPermission.EXTENSIONLIFECYCLE);
    checkValid();
    beginStateChange();
    try {
      uninstallWorker(new PrivilegedExceptionAction() {
        public Object run() throws BundleException {
          uninstallWorkerPrivileged();
          return null;
        }
      });
    } finally {
      completeStateChange();
    }
  }

  SmartPointer<Dictionary> GetHeaders(const std::string& localeString = "") {
    framework.checkAdminPermission(this, AdminPermission.METADATA);
    ManifestLocalization localization;
    try {
      localization = getManifestLocalization();
    } catch (BundleException e) {
      framework.publishFrameworkEvent(FrameworkEvent.ERROR, this, e);
      // return an empty dictinary.
      return new Hashtable();
    }
    if (localeString == null)
      localeString = Locale.getDefault().toString();
    return localization.getHeaders(localeString);
  }

  long GetBundleId() {
    return (bundledata.getBundleID());
  }

  std::string GetLocation() {
    framework.checkAdminPermission(this, AdminPermission.METADATA);
    return (bundledata.getLocation());
  }

  std::string ToString() const {
    String name = bundledata.getSymbolicName();
    if (name == null)
      name = "unknown"; //$NON-NLS-1$
    return (name + '_' + bundledata.getVersion() + " [" + getBundleId() + "]"); //$NON-NLS-1$ //$NON-NLS-2$
  }

  bool operator<(const Object * obj) const {
    int slcomp = getStartLevel() - ((AbstractBundle) obj).getStartLevel();
    if (slcomp != 0) {
      return slcomp;
    }
    long idcomp = getBundleId() - ((AbstractBundle) obj).getBundleId();
    return (idcomp < 0L) ? -1 : ((idcomp > 0L) ? 1 : 0);
  }

  std::string GetSymbolicName() {
    return bundledata.getSymbolicName();
  }

  long GetLastModified() {
    return bundledata.getLastModified();
  }

  SmartPointer<BundleData> GetBundleData() {
    return bundledata;
  }

  Version GetVersion() {
    return bundledata.getVersion();
  }

  SmartPointer<BundleDescription> GetBundleDescription() {
    return framework.adaptor.getState().getBundle(getBundleId());
  }

  SmartPointer<BundleContext> GetBundleContext() {
    framework.checkAdminPermission(this, AdminPermission.CONTEXT);
    return getContext();
  }

  BundleException GetResolutionFailureException() {
    BundleDescription bundleDescription = getBundleDescription();
    if (bundleDescription == null)
      return new BundleException(Msg.BUNDLE_UNRESOLVED_EXCEPTION, BundleException.RESOLVE_ERROR);
    // just a sanity check - this would be an inconsistency between the framework and the state
    if (bundleDescription.isResolved())
      return new BundleException(Msg.BUNDLE_UNRESOLVED_STATE_CONFLICT, BundleException.RESOLVE_ERROR);
    return getResolverError(bundleDescription);
  }

  std::size_t HashCode() const {
    long id = getBundleId();
    return (int) (id ^ (id >>> 32));
  }

  bool operator==(const Object* other) const {
    return getBundleId() == ((AbstractBundle) other).getBundleId();
  }

   bool TestStateChanging(Poco::Thread* thread) {
    return stateChanging == thread;
  }

  Poco::Thread* GetStateChanging() {
    return stateChanging;
  }

  BundleException GetResolverError(SmartPointer<BundleDescription> bundleDesc) {
    ResolverError[] errors = framework.adaptor.getState().getResolverErrors(bundleDesc);
    if (errors == null || errors.length == 0)
      return new BundleException(Msg.BUNDLE_UNRESOLVED_EXCEPTION, BundleException.RESOLVE_ERROR);
    StringBuffer message = new StringBuffer();
    int errorType = BundleException.RESOLVE_ERROR;
    for (int i = 0; i < errors.length; i++) {
      if ((errors[i].getType() & ResolverError.INVALID_NATIVECODE_PATHS) != 0)
        errorType = BundleException.NATIVECODE_ERROR;
      message.append(errors[i].toString());
      if (i < errors.length - 1)
        message.append(", "); //$NON-NLS-1$
    }
    return new BundleException(NLS.bind(Msg.BUNDLE_UNRESOLVED_UNSATISFIED_CONSTRAINT_EXCEPTION, message.toString()), errorType);
  }

  class BundleStatusException : public Poco::Exception {

  private:

    Object::Pointer status;

  public:

    BundleStatusException(const std::string& msg, int code = 0, Object::Pointer status = Object::Pointer(0))
     : Poco::Exception(msg, code), status(status)
    {

    }

    BundleStatusException(const BundleStatusException& exc)
     : Poco::Exception(exc), status(exc.status)
     {

     }

    ~BundleStatusException() throw() {}

    BundleStatusException& operator = (const BundleStatusException& exc)
    {
      Poco::Exception::operator=(exc);
      this->status = exc.status;
      return *this;
    }

    const char* name() const throw()
    {
      return "Bundle status exception";
    }

    const char* className() const throw()
    {
      return typeid(*this).name();
    }

    Poco::Exception* clone() const
    {
      return new BundleStatusException(*this);
    }

    void rethrow() const
    {
      throw *this;
    }

    Object::Pointer GetStatus() const {
      return status;
    }

  };

}
}
}
