SET(EVENT_CPP_FILES
  cherryBundleEvent.cpp
  cherryPlatformEvent.cpp
)

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
)

SET(SERVICE_CPP_FILES
  cherryIExtensionPointService.cpp
  cherryService.cpp
  cherryServiceRegistry.cpp
)

SET(CPP_FILES
  manifest.cpp
  cherryIBundleManifest.cpp
  cherryBundleLoader.cpp
  cherryObject.cpp
  cherryPlatform.cpp
  cherryPlatformException.cpp
  cherryPlugin.cpp
  
  # application
  application/cherryIApplication.cpp
  application/cherryStarter.cpp
)


foreach(file ${EVENT_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} event/${file})
endforeach(file ${EVENT_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

foreach(file ${SERVICE_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} service/${file})
endforeach(file ${SERVICE_CPP_FILES})