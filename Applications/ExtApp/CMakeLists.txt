SET(MITK_USE_OPENCHERRY 1)
SET(MITK_USE_EXT 1)
FIND_PACKAGE(MITK)

# TODO: this can be removed after resolving factory issues, it's only necessary for the registration call in main()
MITK_USE_MODULE(MitkExt)

INCLUDE_DIRECTORIES(
  ${OPENCHERRY_PLUGIN_SOURCE_DIRS} 
  ${org.opencherry.osgi_BIN_DIR} # needed for generated cherryConfig.h 
  ${Poco_INCLUDE_DIRS}
  ${ALL_INCLUDE_DIRECTORIES}
)
LINK_DIRECTORIES(${Poco_LIBRARY_DIRS})
  
ADD_EXECUTABLE(ExtApp ExtApp.cpp)

TARGET_LINK_LIBRARIES(ExtApp 
                      optimized PocoFoundation      debug PocoFoundationd 
                      optimized PocoUtil            debug PocoUtild 
                      optimized org_opencherry_osgi debug org_opencherry_osgi${OPENCHERRY_DEBUG_POSTFIX}
                      ${ALL_LIBRARIES})

					  
SET(EXTAPP_REQUIRED_BUNDLES
org.mitk.gui.qt.datamanager
org.mitk.gui.qt.imagenavigator
)
FOREACH(EXTAPP_REQUIRED_BUNDLE ${EXTAPP_REQUIRED_BUNDLES})
  SET(MITK_BUILD_${EXTAPP_REQUIRED_BUNDLE} "ON" CACHE BOOL "Build ${EXTAPP_REQUIRED_BUNDLE} Plugin" FORCE)
  STRING(REPLACE "." "_" EXTAPP_REQUIRED_BUNDLE_PROJECT ${EXTAPP_REQUIRED_BUNDLE})
  SET(MITK_MODULES_ENABLED_PLUGINS ${MITK_MODULES_ENABLED_PLUGINS} ${EXTAPP_REQUIRED_BUNDLE_PROJECT})
ENDFOREACH()

#MESSAGE(STATUS ${MITK_MODULES_ENABLED_PLUGINS})
					  
SET(_plugin_deps ${OPENCHERRY_ENABLED_PLUGINS} ${MITK_CORE_ENABLED_PLUGINS} ${MITK_MODULES_ENABLED_PLUGINS})

IF(_plugin_deps)
  # Make sure all enabled plug-ins are up to date
  ADD_DEPENDENCIES(ExtApp ${_plugin_deps})
ENDIF()

SET(OPENCHERRY_PLUGIN_CACHE_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugin_cache")

CONFIGURE_FILE(ExtApp.ini
               ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ExtApp.ini)

IF(WIN32)
  SET(EXTAPP_BUILD_TYPE debug)
  CONFIGURE_FILE(startExtApp.bat.in
               ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/startExtApp_${EXTAPP_BUILD_TYPE}.bat @ONLY)
  SET(EXTAPP_BUILD_TYPE release)
  CONFIGURE_FILE(startExtApp.bat.in
               ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/startExtApp_${EXTAPP_BUILD_TYPE}.bat @ONLY)
ENDIF(WIN32)

MITK_INSTALL_TARGETS(ExtApp)


