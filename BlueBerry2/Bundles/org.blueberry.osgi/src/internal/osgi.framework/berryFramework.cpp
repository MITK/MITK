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

#include "berryFramework.h"

namespace berry {
namespace osgi {
namespace internal {

Framework::Framework(/*FrameworkAdaptor adaptor*/) {
    Initialize(/*adaptor*/);
  }

void Framework::Initialize(/*FrameworkAdaptor adaptor*/) {
//    if (Profile.PROFILE && Profile.STARTUP)
//      Profile.logEnter("Framework.initialze()", null); //$NON-NLS-1$
//    long start = System.currentTimeMillis();
//    this.adaptor = adaptor;
//    delegateHooks = adaptor instanceof BaseAdaptor ? ((BaseAdaptor) adaptor).getHookRegistry().getClassLoaderDelegateHooks() : null;
//    active = false;
//    installSecurityManager();
//    if (Debug.DEBUG && Debug.DEBUG_SECURITY) {
//      Debug.println("SecurityManager: " + System.getSecurityManager()); //$NON-NLS-1$
//      Debug.println("ProtectionDomain of Framework.class: \n" + this.getClass().getProtectionDomain()); //$NON-NLS-1$
//    }
//    setNLSFrameworkLog();
//    // initialize ContextFinder
//    initializeContextFinder();
//    /* initialize the adaptor */
//    adaptor.initialize(this);
//    if (Profile.PROFILE && Profile.STARTUP)
//      Profile.logTime("Framework.initialze()", "adapter initialized"); //$NON-NLS-1$//$NON-NLS-2$
//    try {
//      adaptor.initializeStorage();
//    } catch (IOException e) /* fatal error */{
//      e.printStackTrace();
//      throw new RuntimeException(e.getMessage());
//    }
//    if (Profile.PROFILE && Profile.STARTUP)
//      Profile.logTime("Framework.initialze()", "adapter storage initialized"); //$NON-NLS-1$//$NON-NLS-2$
//    /*
//     * This must be done before calling any of the framework getProperty
//     * methods.
//     */
//    initializeProperties(adaptor.getProperties());
//    /* initialize admin objects */
//    packageAdmin = new PackageAdminImpl(this);
//    try {
//      // always create security admin even with security off
//      securityAdmin = new SecurityAdmin(null, this, adaptor.getPermissionStorage());
//    } catch (IOException e) /* fatal error */{
//      e.printStackTrace();
//      throw new RuntimeException(e.getMessage());
//    }
//    if (Profile.PROFILE && Profile.STARTUP)
//      Profile.logTime("Framework.initialze()", "done init props & new PermissionAdminImpl"); //$NON-NLS-1$//$NON-NLS-2$
//    startLevelManager = new StartLevelManager(this);
    /* create the event manager and top level event dispatchers */
    eventManager = new EventManager("Framework Event Dispatcher"); //$NON-NLS-1$
//    bundleEvent = new CopyOnWriteIdentityMap();
//    bundleEventSync = new CopyOnWriteIdentityMap();
//    frameworkEvent = new CopyOnWriteIdentityMap();
//    if (Profile.PROFILE && Profile.STARTUP)
//      Profile.logTime("Framework.initialze()", "done new EventManager"); //$NON-NLS-1$ //$NON-NLS-2$
//    /* create the service registry */
//    serviceRegistry = new ServiceRegistry(this);
//    // Initialize the installLock; there is no way of knowing
//    // what the initial size should be, at most it will be the number
//    // of threads trying to install a bundle (probably a very low number).
//    installLock = new Hashtable(10);
//    /* create the system bundle */
//    createSystemBundle();
//    loadVMProfile(); // load VM profile after the system bundle has been created
//    setBootDelegation(); //set boot delegation property after system exports have been set
//    if (Profile.PROFILE && Profile.STARTUP)
//      Profile.logTime("Framework.initialze()", "done createSystemBundle"); //$NON-NLS-1$ //$NON-NLS-2$
//    /* install URLStreamHandlerFactory */
//    installURLStreamHandlerFactory(systemBundle.context, adaptor);
//    /* install ContentHandlerFactory for OSGi URLStreamHandler support */
//    installContentHandlerFactory(systemBundle.context, adaptor);
//    if (Profile.PROFILE && Profile.STARTUP)
//      Profile.logTime("Framework.initialze()", "done new URLStream/Content HandlerFactory"); //$NON-NLS-1$//$NON-NLS-2$
//    /* create bundle objects for all installed bundles. */
//    BundleData[] bundleDatas = adaptor.getInstalledBundles();
//    bundles = new BundleRepository(bundleDatas == null ? 10 : bundleDatas.length + 1);
//    /* add the system bundle to the Bundle Repository */
//    bundles.add(systemBundle);
//    if (bundleDatas != null) {
//      for (int i = 0; i < bundleDatas.length; i++) {
//        try {
//          AbstractBundle bundle = AbstractBundle.createBundle(bundleDatas[i], this, true);
//          bundles.add(bundle);
//        } catch (BundleException be) {
//          // This is not a fatal error. Publish the framework event.
//          publishFrameworkEvent(FrameworkEvent.ERROR, systemBundle, be);
//        }
//      }
//    }
//    if (Debug.DEBUG && Debug.DEBUG_GENERAL)
//      System.out.println("Initialize the framework: " + (System.currentTimeMillis() - start)); //$NON-NLS-1$
//    if (Profile.PROFILE && Profile.STARTUP)
//      Profile.logExit("Framework.initialize()"); //$NON-NLS-1$
  }

ListenerQueue Framework::NewListenerQueue() {
    return ListenerQueue(eventManager);
  }

}
}
}
