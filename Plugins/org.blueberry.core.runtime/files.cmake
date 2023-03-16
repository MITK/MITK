set(MOC_H_FILES
  src/berryPlugin.h

  src/internal/berryApplicationContainer.h
  src/internal/berryApplicationDescriptor.h
  src/internal/berryApplicationHandle.h
  src/internal/berryCTKPluginListener.h
  src/internal/berryCTKPluginActivator.h
  src/internal/berryErrorApplication.h
  src/internal/berryExtensionRegistry.h
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
  berryIProduct.cpp
  berryIProductProvider.h
  berryListenerList.h
  berryLog.cpp
  berryMultiStatus.cpp
  berryObject.cpp
  berryObjectGeneric.h
  berryObjectList.h
  berryObjects.cpp
  berryObjectString.cpp
  berryObjectStringMap.cpp
  berryObjectTypeInfo.cpp
  berryOperationCanceledException.cpp
  berryPlatform.cpp
  berryPlatformException.cpp
  berryPlatformObject.cpp
  berryPlugin.cpp
  berryReflection.cpp
  berrySafeRunner.cpp
  berrySmartPointer.h
  berryStatus.cpp
  berryWeakPointer.h

  # application
  application/berryIApplication.cpp
  application/berryIApplicationContext.cpp

  dynamichelpers/berryExtensionTracker.cpp
  dynamichelpers/berryIExtensionChangeHandler.cpp
  dynamichelpers/berryIExtensionTracker.cpp

  # registry
  registry/berryIConfigurationElement.cpp
  registry/berryIContributor.cpp
  registry/berryIExecutableExtension.cpp
  registry/berryIExecutableExtensionFactory.cpp
  registry/berryIExtension.cpp
  registry/berryIExtensionPoint.cpp
  registry/berryIExtensionPointFilter.cpp
  registry/berryIExtensionRegistry.cpp
  registry/berryInvalidRegistryObjectException.cpp
  registry/berryIRegistryEventListener.cpp
  registry/berryRegistryConstants.cpp

)

set(INTERNAL_CPP_FILES
  berryApplicationContainer.cpp
  berryApplicationDescriptor.cpp
  berryApplicationHandle.cpp
  berryCombinedEventDelta.cpp
  berryConfigurationElement.h
  berryConfigurationElement.cpp
  berryConfigurationElementAttribute.cpp
  berryConfigurationElementDescription.cpp
  berryConfigurationElementHandle.cpp
  berryCTKPluginActivator.cpp
  berryCTKPluginListener.cpp
  berryCTKPluginUtils.cpp
  berryErrorApplication.cpp
  berryExtension.cpp
  berryExtensionHandle.cpp
  berryExtensionPoint.cpp
  berryExtensionPointHandle.cpp
  berryExtensionRegistry.cpp
  berryExtensionsParser.cpp
  berryHandle.cpp
  berryIBranding.cpp
  berryInternalPlatform.cpp
  berryIObjectManager.cpp
  berryIRuntimeConstants.cpp
  berryLogImpl.cpp
  berryKeyedElement.cpp
  berryKeyedHashSet.cpp
  berryMainApplicationLauncher.cpp
  berryProduct.cpp
  berryProductExtensionBranding.cpp
  berryProviderExtensionBranding.cpp
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
  berrySimpleExtensionPointFilter.cpp
  berryTemporaryObjectManager.cpp
  berryThirdLevelConfigurationElementHandle.cpp
)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
