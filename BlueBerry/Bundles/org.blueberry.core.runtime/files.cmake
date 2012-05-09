set(MOC_H_FILES
  src/internal/berryCTKPluginListener_p.h
  src/internal/berryCTKPluginActivator.h
  src/internal/berryExtensionPointService.h
  src/internal/berryPreferencesService.h
)

set(CACHED_RESOURCE_FILES
  plugin.xml
)

set(SRC_CPP_FILES
  berryBackingStoreException.cpp
  berryBundleLoader.cpp
  berryDebugBreakpointManager.cpp
  berryDebugUtil.cpp
  berryException.cpp
  berryExtensionType.cpp
  berryIAdaptable.cpp
  berryIAdapterManager.cpp
  berryIBundleManifest.cpp
  berryIDebugObjectListener.cpp
  berryIPreferences.cpp
  berryIBerryPreferences.cpp
  berryIPreferencesService.cpp
  berryLog.cpp
  berryMultiStatus.cpp
  berryObject.cpp
  berryObjectGeneric.h
  berryObjectList.h
  berryObjects.cpp
  berryObjectString.h
  berryOperationCanceledException.cpp
  berryPlatform.cpp
  berryPlatformException.cpp
  berryPlatformObject.cpp
  berryPlugin.cpp
  berryRuntime.cpp
  berrySafeRunner.cpp
  berryStackTrace.cpp
  berryStatus.cpp

  # application
  application/berryIApplication.cpp
  application/berryStarter.cpp

  # registry
  registry/berryIConfigurationElement.h
  registry/berryIConfigurationElementLegacy.h
  registry/berryIExecutableExtension.h
  registry/berryIExtension.h
  registry/berryIExtensionPoint.h
  registry/berryIExtensionPointService.cpp
  registry/berryIExtensionRegistry.h

  # service
  service/berryService.cpp
  service/berryServiceRegistry.cpp
)

set(INTERNAL_CPP_FILES
  berryAbstractPreferencesStorage.cpp
  berryBundle.cpp
  berryBundleContext.cpp
  berryBundleDirectory.cpp
  berryBundleManifest.cpp
  berryCodeCache.cpp
  berryCTKPluginActivator.cpp
  berryCTKPluginListener.cpp
  berryDefaultActivator.cpp
  #berryConfigurationElement.cpp
  #berryExtension.cpp
  #berryExtensionPoint.cpp
  #berryExtensionPointService.cpp
  berryHandle.cpp
  berryInternalPlatform.cpp
  berryIRuntimeConstants.cpp
  berryPreferences.cpp
  berryPreferencesService.cpp
  berryProvisioningInfo.cpp
  berrySystemBundle.cpp
  berrySystemBundleActivator.cpp
  berrySystemBundleManifest.cpp
  berryXMLPreferencesStorage.cpp
)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
