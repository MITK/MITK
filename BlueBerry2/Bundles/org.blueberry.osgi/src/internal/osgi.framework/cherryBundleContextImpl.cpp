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

#include "cherryBundleContextImpl.h"

namespace cherry {
namespace osgi {
namespace internal {

 BundleContextImpl::BundleContextImpl(SmartPointer<BundleHost> bundle) {
    this.bundle = bundle;
    valid = true;
    framework = bundle.framework;
    bundleEvent = null;
    bundleEventSync = null;
    frameworkEvent = null;
    eventDispatcher = new BundleEventDispatcher();
    synchronized (contextLock) {
      servicesInUse = null;
    }
    activator = null;
  }

  void BundleContextImpl::Close() {
    valid = false; /* invalidate context */

    final ServiceRegistry registry = framework.getServiceRegistry();

    registry.removeAllServiceListeners(this);
    synchronized (framework.frameworkEvent) {
      if (frameworkEvent != null) {
        framework.frameworkEvent.remove(this);
        frameworkEvent = null;
      }
    }
    synchronized (framework.bundleEvent) {
      if (bundleEvent != null) {
        framework.bundleEvent.remove(this);
        bundleEvent = null;
      }
    }
    synchronized (framework.bundleEventSync) {
      if (bundleEventSync != null) {
        framework.bundleEventSync.remove(this);
        bundleEventSync = null;
      }
    }

    /* service's registered by the bundle, if any, are unregistered. */
    registry.unregisterServices(this);

    /* service's used by the bundle, if any, are released. */
    registry.releaseServicesInUse(this);

    synchronized (contextLock) {
      servicesInUse = null;
    }
  }

  bool BundleContextImpl::IsValid() const {
    return valid;
  }

  void BundleContextImpl::Start() throw(BundleException) {
    activator = bundle.loadBundleActivator();

    if (activator != null) {
      try {
        startActivator(activator);
      } catch (BundleException be) {
        activator = null;
        throw be;
      }
    }

    /* activator completed successfully. We must use this
     same activator object when we stop this bundle. */
  }

  void BundleContextImpl::StartActivator(BundleActivator* bundleActivator) throw(BundleException) {
    if (Profile.PROFILE && Profile.STARTUP)
      Profile.logEnter("BundleContextImpl.startActivator()", null); //$NON-NLS-1$
    try {
      AccessController.doPrivileged(new PrivilegedExceptionAction() {
        public Object run() throws Exception {
          if (bundleActivator != null) {
            if (Profile.PROFILE && Profile.STARTUP)
              Profile.logTime("BundleContextImpl.startActivator()", "calling " + bundle.getLocation() + " bundle activator"); //$NON-NLS-1$//$NON-NLS-2$//$NON-NLS-3$
            // make sure the context class loader is set correctly
            Object previousTCCL = setContextFinder();
            /* Start the bundle synchronously */
            try {
              bundleActivator.start(BundleContextImpl.this);
            } finally {
              if (previousTCCL != Boolean.FALSE)
                Thread.currentThread().setContextClassLoader((ClassLoader) previousTCCL);
            }
            if (Profile.PROFILE && Profile.STARTUP)
              Profile.logTime("BundleContextImpl.startActivator()", "returned from " + bundle.getLocation() + " bundle activator"); //$NON-NLS-1$//$NON-NLS-2$//$NON-NLS-3$
          }
          return null;
        }
      });
    } catch (Throwable t) {
      if (t instanceof PrivilegedActionException) {
        t = ((PrivilegedActionException) t).getException();
      }

      if (Debug.DEBUG && Debug.DEBUG_GENERAL) {
        Debug.printStackTrace(t);
      }

      String clazz = null;
      clazz = bundleActivator.getClass().getName();

      throw new BundleException(NLS.bind(Msg.BUNDLE_ACTIVATOR_EXCEPTION, new Object[] {clazz, "start", bundle.getSymbolicName() == null ? "" + bundle.getBundleId() : bundle.getSymbolicName()}), BundleException.ACTIVATOR_ERROR, t); //$NON-NLS-1$ //$NON-NLS-2$
    } finally {
      if (Profile.PROFILE && Profile.STARTUP)
        Profile.logExit("BundleContextImpl.startActivator()"); //$NON-NLS-1$
    }
  }

  void BundleContextImpl::Stop() throw(BundleException) {
    try {
      AccessController.doPrivileged(new PrivilegedExceptionAction() {
        public Object run() throws Exception {
          if (activator != null) {
            // make sure the context class loader is set correctly
            Object previousTCCL = setContextFinder();
            try {
              /* Stop the bundle synchronously */
              activator.stop(BundleContextImpl.this);
            } finally {
              if (previousTCCL != Boolean.FALSE)
                Thread.currentThread().setContextClassLoader((ClassLoader) previousTCCL);
            }
          }
          return null;
        }
      });
    } catch (Throwable t) {
      if (t instanceof PrivilegedActionException) {
        t = ((PrivilegedActionException) t).getException();
      }

      if (Debug.DEBUG && Debug.DEBUG_GENERAL) {
        Debug.printStackTrace(t);
      }

      String clazz = (activator == null) ? "" : activator.getClass().getName(); //$NON-NLS-1$

      throw new BundleException(NLS.bind(Msg.BUNDLE_ACTIVATOR_EXCEPTION, new Object[] {clazz, "stop", bundle.getSymbolicName() == null ? "" + bundle.getBundleId() : bundle.getSymbolicName()}), BundleException.ACTIVATOR_ERROR, t); //$NON-NLS-1$ //$NON-NLS-2$
    } finally {
      activator = null;
    }
  }

  std::string BundleContextImpl::GetProperty(const std::string& key) {
    SecurityManager sm = System.getSecurityManager();

    if (sm != null) {
      sm.checkPropertyAccess(key);
    }

    return (framework.getProperty(key));
  }

  SmartPointer<Bundle> BundleContextImpl::GetBundle() {
    checkValid();

    return getBundleImpl();
  }

  SmartPointer<BundleHost> BundleContextImpl::GetBundleImpl() {
    return bundle;
  }

  //Bundle BundleContextImpl::InstallBundle(String location) throws BundleException {
  //  return installBundle(location, null);
  //}

//  Bundle BundleContextImpl::InstallBundle(String location, InputStream in) throws BundleException {
//    checkValid();
//    //note AdminPermission is checked after bundle is loaded
//    return framework.installBundle(location, in);
//  }

  SmartPointer<Bundle> BundleContextImpl::GetBundle(long id) {
    return (framework.getBundle(id));
  }

  SmartPointer<BundleHost> BundleContextImpl::GetBundleByLocation(const std::string& location) {
    return (framework.getBundleByLocation(location));
  }

  std::vector<SmartPointer<Bundle> > BundleContextImpl::GetBundles() {
    return framework.getAllBundles();
  }

//  void BundleContextImpl::AddServiceListener(SmartPointer<ServiceListener> listener, const std::string& filter) throw(InvalidSyntaxException) {
//    checkValid();
//
//    if (listener == null) {
//      throw new IllegalArgumentException();
//    }
//    framework.getServiceRegistry().addServiceListener(this, listener, filter);
//  }

//  void BundleContextImpl::AddServiceListener(SmartPointer<ServiceListener> listener) {
//    try {
//      addServiceListener(listener, null);
//    } catch (InvalidSyntaxException e) {
//      if (Debug.DEBUG && Debug.DEBUG_GENERAL) {
//        Debug.println("InvalidSyntaxException w/ null filter" + e.getMessage()); //$NON-NLS-1$
//        Debug.printStackTrace(e);
//      }
//    }
//  }

//  void BundleContextImpl::RemoveServiceListener(SmartPointer<ServiceListener> listener) {
//    checkValid();
//
//    if (listener == null) {
//      throw new IllegalArgumentException();
//    }
//    framework.getServiceRegistry().removeServiceListener(this, listener);
//  }

//  void BundleContextImpl::AddBundleListener(SmartPointer<BundleListener> listener) {
//    checkValid();
//    if (listener == null) {
//      throw new IllegalArgumentException();
//    }
//
//    if (Debug.DEBUG && Debug.DEBUG_EVENTS) {
//      String listenerName = listener.getClass().getName() + "@" + Integer.toHexString(System.identityHashCode(listener)); //$NON-NLS-1$
//      Debug.println("addBundleListener[" + bundle + "](" + listenerName + ")"); //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$
//    }
//
//    if (listener instanceof SynchronousBundleListener) {
//      framework.checkAdminPermission(getBundle(), AdminPermission.LISTENER);
//      synchronized (framework.bundleEventSync) {
//        checkValid();
//        if (bundleEventSync == null) {
//          bundleEventSync = new CopyOnWriteIdentityMap();
//          framework.bundleEventSync.put(this, this);
//        }
//
//        bundleEventSync.put(listener, listener);
//      }
//    } else {
//      synchronized (framework.bundleEvent) {
//        checkValid();
//        if (bundleEvent == null) {
//          bundleEvent = new CopyOnWriteIdentityMap();
//          framework.bundleEvent.put(this, this);
//        }
//
//        bundleEvent.put(listener, listener);
//      }
//    }
//  }

//  void BundleContextImpl::RemoveBundleListener(SmartPointer<BundleListener> listener) {
//    checkValid();
//    if (listener == null) {
//      throw new IllegalArgumentException();
//    }
//
//    if (Debug.DEBUG && Debug.DEBUG_EVENTS) {
//      String listenerName = listener.getClass().getName() + "@" + Integer.toHexString(System.identityHashCode(listener)); //$NON-NLS-1$
//      Debug.println("removeBundleListener[" + bundle + "](" + listenerName + ")"); //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$
//    }
//
//    if (listener instanceof SynchronousBundleListener) {
//      framework.checkAdminPermission(getBundle(), AdminPermission.LISTENER);
//
//      synchronized (framework.bundleEventSync) {
//        if (bundleEventSync != null) {
//          bundleEventSync.remove(listener);
//        }
//      }
//    } else {
//      synchronized (framework.bundleEvent) {
//        if (bundleEvent != null) {
//          bundleEvent.remove(listener);
//        }
//      }
//    }
//  }

//  void BundleContextImpl::AddFrameworkListener(SmartPointer<FrameworkListener> listener) {
//    checkValid();
//    if (listener == null) {
//      throw new IllegalArgumentException();
//    }
//
//    if (Debug.DEBUG && Debug.DEBUG_EVENTS) {
//      String listenerName = listener.getClass().getName() + "@" + Integer.toHexString(System.identityHashCode(listener)); //$NON-NLS-1$
//      Debug.println("addFrameworkListener[" + bundle + "](" + listenerName + ")"); //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$
//    }
//
//    synchronized (framework.frameworkEvent) {
//      checkValid();
//      if (frameworkEvent == null) {
//        frameworkEvent = new CopyOnWriteIdentityMap();
//        framework.frameworkEvent.put(this, this);
//      }
//
//      frameworkEvent.put(listener, listener);
//    }
//  }

//  void BundleContextImpl::RemoveFrameworkListener(SmartPointer<FrameworkListener> listener) {
//    checkValid();
//    if (listener == null) {
//      throw new IllegalArgumentException();
//    }
//
//    if (Debug.DEBUG && Debug.DEBUG_EVENTS) {
//      String listenerName = listener.getClass().getName() + "@" + Integer.toHexString(System.identityHashCode(listener)); //$NON-NLS-1$
//      Debug.println("removeFrameworkListener[" + bundle + "](" + listenerName + ")"); //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$
//    }
//
//    synchronized (framework.frameworkEvent) {
//      if (frameworkEvent != null) {
//        frameworkEvent.remove(listener);
//      }
//    }
//  }

  SmartPointer<ServiceRegistration> BundleContextImpl::RegisterService(const std::vector<std::string>& clazzes, Object::Pointer service,
      const Dictionary& properties) throw(IllegalArgumentException, IllegalStateException) {
    checkValid();

    return framework.getServiceRegistry().registerService(this, clazzes, service, properties);
  }

  SmartPointer<ServiceRegistration> BundleContextImpl::RegisterService(const std::string& clazz, Object::Pointer service, const Dictionary& properties) throw(IllegalArgumentException, IllegalStateException) {
    String[] clazzes = new String[] {clazz};

    return registerService(clazzes, service, properties);
  }

  std::vector<SmartPointer<ServiceReference> > BundleContextImpl::GetServiceReferences(const std::string& clazz, const std::string& filter) throw(InvalidSyntaxException) {
    checkValid();
    return framework.getServiceRegistry().getServiceReferences(this, clazz, filter, false);
  }

  std::vector<SmartPointer<ServiceReference> > BundleContextImpl::GetAllServiceReferences(const std::string& clazz, const std::string& filter) throw(InvalidSyntaxException) {
    checkValid();
    return framework.getServiceRegistry().getServiceReferences(this, clazz, filter, true);
  }

  SmartPointer<ServiceReference> BundleContextImpl::GetServiceReference(const std::string& clazz) {
    checkValid();

    return framework.getServiceRegistry().getServiceReference(this, clazz);
  }

   Object::Pointer BundleContextImpl::GetService(SmartPointer<ServiceReference> reference) {
    checkValid();
    if (reference == null)
      throw new NullPointerException("A null service reference is not allowed."); //$NON-NLS-1$
    synchronized (contextLock) {
      if (servicesInUse == null)
        // Cannot predict how many services a bundle will use, start with a small table.
        servicesInUse = new HashMap(10);
    }

    return framework.getServiceRegistry().getService(this, (ServiceReferenceImpl) reference);
  }

  bool BundleContextImpl::UngetService(SmartPointer<ServiceReference> reference) {
    checkValid();

    return framework.getServiceRegistry().ungetService(this, (ServiceReferenceImpl) reference);
  }

  Poco::File BundleContextImpl::GetDataFile(const std::string& filename) {
    checkValid();

    return (framework.getDataFile(bundle, filename));
  }

  ServiceUseMap& BundleContextImpl::GetServicesInUseMap() {
    synchronized (contextLock) {
      return servicesInUse;
    }
  }

  void BundleContextImpl::BundleEventDispatcher::DispatchEvent(Object::Pointer originalListener, Object::Pointer l, int action, Object::Pointer object) {
    // save the bundle ref to a local variable
    // to avoid interference from another thread closing this context
    AbstractBundle tmpBundle = bundle;
    Object previousTCCL = setContextFinder();
    try {
      if (isValid()) /* if context still valid */{
        switch (action) {
          case Framework.BUNDLEEVENT :
          case Framework.BUNDLEEVENTSYNC : {
            BundleListener listener = (BundleListener) l;

            if (Debug.DEBUG && Debug.DEBUG_EVENTS) {
              String listenerName = listener.getClass().getName() + "@" + Integer.toHexString(System.identityHashCode(listener)); //$NON-NLS-1$
              Debug.println("dispatchBundleEvent[" + tmpBundle + "](" + listenerName + ")"); //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$
            }

            BundleEvent event = (BundleEvent) object;
            switch (event.getType()) {
              case Framework.BATCHEVENT_BEGIN : {
                if (listener instanceof BatchBundleListener)
                  ((BatchBundleListener) listener).batchBegin();
                break;
              }
              case Framework.BATCHEVENT_END : {
                if (listener instanceof BatchBundleListener)
                  ((BatchBundleListener) listener).batchEnd();
                break;
              }
              default : {
                listener.bundleChanged((BundleEvent) object);
              }
            }
            break;
          }

          case ServiceRegistry.SERVICEEVENT : {
            ServiceEvent event = (ServiceEvent) object;

            ServiceListener listener = (ServiceListener) l;
            if (Debug.DEBUG && Debug.DEBUG_EVENTS) {
              String listenerName = listener.getClass().getName() + "@" + Integer.toHexString(System.identityHashCode(listener)); //$NON-NLS-1$
              Debug.println("dispatchServiceEvent[" + tmpBundle + "](" + listenerName + ")"); //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$
            }
            listener.serviceChanged(event);

            break;
          }

          case Framework.FRAMEWORKEVENT : {
            FrameworkListener listener = (FrameworkListener) l;

            if (Debug.DEBUG && Debug.DEBUG_EVENTS) {
              String listenerName = listener.getClass().getName() + "@" + Integer.toHexString(System.identityHashCode(listener)); //$NON-NLS-1$
              Debug.println("dispatchFrameworkEvent[" + tmpBundle + "](" + listenerName + ")"); //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$
            }

            listener.frameworkEvent((FrameworkEvent) object);
            break;
          }
          default : {
            throw new InternalError();
          }
        }
      }
    } catch (Throwable t) {
      if (Debug.DEBUG && Debug.DEBUG_GENERAL) {
        Debug.println("Exception in bottom level event dispatcher: " + t.getMessage()); //$NON-NLS-1$
        Debug.printStackTrace(t);
      }
      // allow the adaptor to handle this unexpected error
      framework.adaptor.handleRuntimeError(t);
      publisherror: {
        if (action == Framework.FRAMEWORKEVENT) {
          FrameworkEvent event = (FrameworkEvent) object;
          if (event.getType() == FrameworkEvent.ERROR) {
            break publisherror; // avoid infinite loop
          }
        }

        framework.publishFrameworkEvent(FrameworkEvent.ERROR, tmpBundle, t);
      }
    } finally {
      if (previousTCCL != Boolean.FALSE)
        Thread.currentThread().setContextClassLoader((ClassLoader) previousTCCL);
    }
  }

  SmartPointer<Filter> BundleContextImpl::CreateFilter(const std::string& filter) const throw(InvalidSyntaxException) {
    checkValid();

    return FilterImpl.newInstance(filter);
  }

  void BundleContextImpl::CheckValid() const {
    if (!isValid()) {
      throw new IllegalStateException(Msg.BUNDLE_CONTEXT_INVALID_EXCEPTION);
    }
  }

//  Framework BundleContextImpl::GetFramework() {
//    return framework;
//  }

}
}
}
