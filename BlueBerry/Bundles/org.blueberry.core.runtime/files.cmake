set(MOC_H_FILES
  src/berryPlugin.h

  src/internal/berryCTKPluginListener.h
  src/internal/berryCTKPluginActivator.h
  src/internal/berryExtensionRegistry.h
  src/internal/berryPreferencesService.h
)

set(CACHED_RESOURCE_FILES
  plugin.xml
)

set(SRC_CPP_FILES
  berryBackingStoreException.cpp
  berryContributorFactory.cpp
  berryCoreException.cpp
  berryDebugBreakpointManager.cpp
  berryDebugUtil.cpp
  berryException.cpp
  berryExtensionType.cpp
  berryIAdaptable.cpp
  berryIAdapterManager.cpp
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
  berryObjectString.cpp
  berryObjectStringMap.cpp
  berryOperationCanceledException.cpp
  berryPlatform.cpp
  berryPlatformException.cpp
  berryPlatformObject.cpp
  berryPlugin.cpp
  berrySafeRunner.cpp
  berrySmartPointer.h
  berryStatus.cpp
  berryWeakPointer.h

  # application
  application/berryIApplication.cpp
  application/berryStarter.cpp

  # registry
  registry/berryIConfigurationElement.cpp
  registry/berryIContributor.cpp
  registry/berryIExecutableExtension.cpp
  registry/berryIExecutableExtensionFactory.cpp
  registry/berryIExtension.cpp
  registry/berryIExtensionPoint.cpp
  registry/berryIExtensionRegistry.cpp
  registry/berryInvalidRegistryObjectException.cpp
  registry/berryIRegistryEventListener.cpp
  registry/berryRegistryConstants.cpp

)

set(INTERNAL_CPP_FILES
  berryAbstractPreferencesStorage.cpp
  berryCombinedEventDelta.cpp
  berryConfigurationElement.h
  berryConfigurationElement.cpp
  berryConfigurationElementAttribute.cpp
  berryConfigurationElementDescription.cpp
  berryConfigurationElementHandle.cpp
  berryCTKPluginActivator.cpp
  berryCTKPluginListener.cpp
  berryCTKPluginUtils.cpp
  berryExtension.cpp
  berryExtensionHandle.cpp
  berryExtensionPoint.cpp
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
