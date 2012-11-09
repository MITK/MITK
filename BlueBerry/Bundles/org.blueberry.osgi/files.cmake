# Files which should be processed by Qts moc
set(MOC_H_FILES
  src/internal/berryCTKPluginActivator.h
  src/internal/berryExtensionPointService.h
)

set(CACHED_RESOURCE_FILES
  plugin.xml
)

set(EVENT_CPP_FILES
  berryBundleEvent.cpp
  berryPlatformEvent.cpp
)

set(INTERNAL_CPP_FILES
  berryBundle.cpp
  berryBundleContext.cpp
  berryBundleDirectory.cpp
  berryBundleManifest.cpp
  berryCodeCache.cpp
  berryCTKPluginActivator.cpp
  berryDefaultActivator.cpp
  berryConfigurationElement.cpp
  berryExtension.cpp
  berryExtensionPoint.cpp
  berryExtensionPointService.cpp
  berryInternalPlatform.cpp
  berryPlatformLogChannel.cpp
  berryProvisioningInfo.cpp
  berrySystemBundle.cpp
  berrySystemBundleActivator.cpp
  berrySystemBundleManifest.cpp
)

set(SERVICE_CPP_FILES
  berryIExtensionPointService.cpp
  berryService.cpp
  berryServiceRegistry.cpp
)

set(SRC_CPP_FILES
  berryDebugBreakpointManager.cpp
  berryException.cpp
  berryExtensionType.cpp
  berryIBundleManifest.cpp
  berryIDebugObjectListener.cpp
  berryBundleLoader.cpp
  berryDebugUtil.cpp
  berryObject.cpp
  berryObjects.cpp
  berryPlatform.cpp
  berryPlatformException.cpp
  berryPlugin.cpp
  berryStackTrace.cpp

  # application
  application/berryIApplication.cpp
  application/berryStarter.cpp
)

set(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${EVENT_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/event/${file})
endforeach(file ${EVENT_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

foreach(file ${SERVICE_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/service/${file})
endforeach(file ${SERVICE_CPP_FILES})
