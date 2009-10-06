SET(MITK_USE_OPENCHERRY 1)
FIND_PACKAGE(MITK)

INCLUDE_DIRECTORIES(
  ${OPENCHERRY_PLUGIN_SOURCE_DIRS}
  ${org.opencherry.osgi_BIN_DIR} # needed for generated cherryConfig.h
  ${Poco_INCLUDE_DIRS}
)
LINK_DIRECTORIES(${Poco_LIBRARY_DIRS})
  
ADD_EXECUTABLE(CoreApp CoreApp.cpp)

TARGET_LINK_LIBRARIES(CoreApp 
                      optimized PocoFoundation      debug PocoFoundationd 
                      optimized PocoUtil            debug PocoUtild 
                      optimized org_opencherry_osgi debug org_opencherry_osgi${OPENCHERRY_DEBUG_POSTFIX})

SET(_plugin_deps ${OPENCHERRY_ENABLED_PLUGINS} ${MITK_CORE_ENABLED_PLUGINS})

IF(_plugin_deps)
  # Make sure all enabled plug-ins are up to date 
  ADD_DEPENDENCIES(CoreApp ${_plugin_deps})
ENDIF()

SET(OPENCHERRY_PLUGIN_CACHE_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugin_cache")

CONFIGURE_FILE(CoreApp.ini
               ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/CoreApp.ini)

IF(WIN32)
  SET(COREAPP_BUILD_TYPE debug)
  CONFIGURE_FILE(startCoreApp.bat.in
               ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/startCoreApp_${COREAPP_BUILD_TYPE}.bat @ONLY)
  SET(COREAPP_BUILD_TYPE release)
  CONFIGURE_FILE(startCoreApp.bat.in
               ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/startCoreApp_${COREAPP_BUILD_TYPE}.bat @ONLY)
ENDIF(WIN32)

MITK_INSTALL_TARGETS(CoreApp)

