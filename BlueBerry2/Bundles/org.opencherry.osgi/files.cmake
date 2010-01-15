SET(INTERNAL_CPP_FILES
  cherryBundle.cpp
  cherryBundleContext.cpp
  cherryBundleDirectory.cpp
  cherryBundleManifest.cpp
  cherryCodeCache.cpp
  cherryDefaultActivator.cpp
  cherryConfigurationElement.cpp
  cherryExtension.cpp
  cherryExtensionPoint.cpp
  cherryExtensionPointService.cpp
  cherryInternalPlatform.cpp
  cherryPlatformLogChannel.cpp
  cherrySystemBundle.cpp
  cherrySystemBundleActivator.cpp
  
  # osgi.framework
  osgi.framework/eventmgr/cherryEventManager.cpp
  osgi.framework/eventmgr/cherryListenerQueue.cpp
  #osgi.framework/cherryBundleContextImpl.cpp
  #osgi.framework/cherryBundleHost.cpp
  osgi.framework/cherryFilterImpl.cpp
  osgi.framework/cherryFramework.cpp
  
  # osgi.serviceregistry
  osgi.serviceregistry/cherryFilteredServiceListener.cpp
  osgi.serviceregistry/cherryServiceProperties.cpp
  osgi.serviceregistry/cherryServiceReferenceImpl.cpp
  osgi.serviceregistry/cherryServiceRegistrationImpl.cpp
  osgi.serviceregistry/cherryServiceRegistry.cpp
  osgi.serviceregistry/cherryServiceUse.cpp
  
  # supplement
  supplement/cherryStackTrace.cpp
)

SET(SRC_CPP_FILES
  
  # event
  event/cherryPlatformEvent.cpp
  
  # osgi/framework
  osgi/framework/BundleEvent.cpp
  osgi/framework/Constants.cpp
  osgi/framework/Exceptions.cpp
  osgi/framework/IBundleManifest.cpp
  osgi/framework/Object.cpp
  osgi/framework/Objects.cpp
  osgi/framework/ServiceEvent.cpp
  osgi/framework/Version.cpp
  
  # osgi/util
  osgi/util/Headers.cpp
  
  cherryBundleLoader.cpp
  cherryDebugUtil.cpp
  cherryPlatform.cpp
  cherryPlugin.cpp
  
  #openCherryAdapter
  openCherryAdapter/runtime/cherryStarter.cpp
  
  #service
  service/cherryIExtensionPointService.cpp
  service/cherryService.cpp
  service/cherryServiceRegistry.cpp
  
  # supplement/framework.debug
  supplement/framework.debug/cherryDebug.cpp
  
)

SET(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
