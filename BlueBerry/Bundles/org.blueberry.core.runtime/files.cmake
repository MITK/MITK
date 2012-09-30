set(MOC_H_FILES
  src/berryPlugin.h

  src/internal/berryCTKPluginListener_p.h
  src/internal/berryCTKPluginActivator.h
  #src/internal/berryExtensionPointService.h
  src/internal/berryPreferencesService.h
)

set(CACHED_RESOURCE_FILES
  plugin.xml
)

set(SRC_CPP_FILES
  berryBackingStoreException.cpp
  #berryBundleLoader.cpp
  berryCoreException.cpp
  berryDebugBreakpointManager.cpp
  berryDebugUtil.cpp
  berryException.cpp
  berryExtensionType.cpp
  berryIAdaptable.cpp
  berryIAdapterManager.cpp
  #berryIBundleManifest.cpp
  berryIDebugObjectListener.cpp
  berryILog.cpp
  berryIPreferences.cpp
  berryIBerryPreferences.cpp
  berryIPreferencesService.cpp
  berryListenerList.h
  berryLog.cpp
  berryMultiStatus.cpp
  berryObject.cpp
  berryObjectGeneric.h
  berryObjectList.h
  berryObjects.cpp
  berryObjectString.h
  berryObjectStringMap.cpp
  berryOperationCanceledException.cpp
  berryPlatform.cpp
  berryPlatformException.cpp
  berryPlatformObject.cpp
  berryPlugin.cpp
  berryRuntime.cpp
  berrySafeRunner.cpp
  berrySmartPointer.h
  berryStackTrace.cpp
  berryStatus.cpp
  berryWeakPointer.h

  # application
  application/berryIApplication.cpp
  application/berryStarter.cpp

  # registry
  registry/berryIConfigurationElement.cpp
  registry/berryIConfigurationElementLegacy.h
  registry/berryIContributor.cpp
  registry/berryIExecutableExtension.cpp
  registry/berryIExecutableExtensionFactory.cpp
  registry/berryIExtension.cpp
  registry/berryIExtensionPoint.cpp
  registry/berryIExtensionPointService.cpp
  registry/berryIExtensionRegistry.cpp
  registry/berryInvalidRegistryObjectException.cpp
  registry/berryIRegistryEventListener.cpp
  registry/berryRegistryConstants.cpp

  # service
  service/berryService.cpp
  service/berryServiceRegistry.cpp
)

set(INTERNAL_CPP_FILES
  berryAbstractPreferencesStorage.cpp
  #berryBundle.cpp
  #berryBundleContext.cpp
  #berryBundleDirectory.cpp
  #berryBundleManifest.cpp
  #berryCodeCache.cpp
  berryCombinedEventDelta.cpp
  berryConfigurationElement.h
  berryConfigurationElement.cpp
  berryConfigurationElementAttribute.cpp
  berryConfigurationElementDescription.cpp
  berryConfigurationElementHandle.cpp
  berryCTKPluginActivator.cpp
  berryCTKPluginListener.cpp
  #berryDefaultActivator.cpp
  berryExtension.cpp
  #berryExtensionDelta.cpp
  berryExtensionHandle.cpp
  berryExtensionPoint.cpp
  #berryExtensionPointService.cpp
  berryExtensionPointHandle.cpp
  berryExtensionRegistry.cpp
  berryExtensionsParser.cpp
  berryHandle.cpp
  berryInternalPlatform.cpp
  berryIObjectManager.cpp
  berryIRuntimeConstants.cpp
  berryLogImpl.cpp
  berryKeyedElement.cpp
  berryKeyedHashSet.cpp
  berryPreferences.cpp
  berryPreferencesService.cpp
  berryProvisioningInfo.cpp
  berryRegistryContribution.cpp
  berryRegistryContributor.cpp
  #berryRegistryDelta.cpp
  berryRegistryObjectFactory.cpp
  berryRegistryIndexChildren.cpp
  berryRegistryIndexElement.cpp
  berryRegistryMessages.cpp
  berryRegistryObject.cpp
  berryRegistryObjectManager.cpp
  berryRegistryObjectReferenceMap.cpp
  berryRegistryProperties.cpp
  berryRegistryStrategy.cpp
  berryRegistryTimestamp.cpp
  berryRegistrySupport.cpp
  #berrySystemBundle.cpp
  #berrySystemBundleActivator.cpp
  #berrySystemBundleManifest.cpp
  berryTemporaryObjectManager.cpp
  berryThirdLevelConfigurationElementHandle.cpp
  berryXMLPreferencesStorage.cpp
)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
