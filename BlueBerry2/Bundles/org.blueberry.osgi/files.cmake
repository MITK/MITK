SET(INTERNAL_CPP_FILES
  berryBundle.cpp
  berryBundleContext.cpp
  berryBundleDirectory.cpp
  berryBundleManifest.cpp
  berryCodeCache.cpp
  berryDefaultActivator.cpp
  berryConfigurationElement.cpp
  berryExtension.cpp
  berryExtensionPoint.cpp
  berryExtensionPointService.cpp
  berryInternalPlatform.cpp
  berryPlatformLogChannel.cpp
  berrySystemBundle.cpp
  berrySystemBundleActivator.cpp
  
  # osgi.framework
  osgi.framework/eventmgr/berryEventManager.cpp
  osgi.framework/eventmgr/berryListenerQueue.cpp
  #osgi.framework/berryBundleContextImpl.cpp
  #osgi.framework/berryBundleHost.cpp
  osgi.framework/berryFilterImpl.cpp
  osgi.framework/berryFramework.cpp
  
  # osgi.serviceregistry
  osgi.serviceregistry/berryFilteredServiceListener.cpp
  osgi.serviceregistry/berryServiceProperties.cpp
  osgi.serviceregistry/berryServiceReferenceImpl.cpp
  osgi.serviceregistry/berryServiceRegistrationImpl.cpp
  osgi.serviceregistry/berryServiceRegistry.cpp
  osgi.serviceregistry/berryServiceUse.cpp
  
  # supplement
  supplement/berryStackTrace.cpp
)

SET(SRC_CPP_FILES
  
  # event
  event/berryPlatformEvent.cpp
  
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
  
  berryBundleLoader.cpp
  berryDebugUtil.cpp
  berryPlatform.cpp
  berryPlugin.cpp
  
  #BlueBerryAdapter
  BlueBerryAdapter/runtime/berryStarter.cpp
  
  #service
  service/berryIExtensionPointService.cpp
  service/berryService.cpp
  service/berryServiceRegistry.cpp
  
  # supplement/framework.debug
  supplement/framework.debug/berryDebug.cpp
  
)

SET(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
