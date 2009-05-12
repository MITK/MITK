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

SET(SRC_CPP_FILES
  cherryException.cpp
  cherryIBundleManifest.cpp
  cherryBundleLoader.cpp
  cherryDebugUtil.cpp
  cherryObject.cpp
  cherryObjects.cpp
  cherryPlatform.cpp
  cherryPlatformException.cpp
  cherryPlugin.cpp
  cherryStackTrace.cpp
  
  # application
  application/cherryIApplication.cpp
  application/cherryStarter.cpp
)

SET(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${EVENT_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/event/${file})
endforeach(file ${EVENT_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

foreach(file ${SERVICE_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/service/${file})
endforeach(file ${SERVICE_CPP_FILES})