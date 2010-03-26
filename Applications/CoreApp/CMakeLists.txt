SET(MITK_USE_BLUEBERRY 1)
FIND_PACKAGE(MITK)

INCLUDE_DIRECTORIES(
  ${BLUEBERRY_PLUGIN_SOURCE_DIRS}
  ${org.blueberry.osgi_BIN_DIR} # needed for generated berryConfig.h
  ${Poco_INCLUDE_DIRS}
)
LINK_DIRECTORIES(${Poco_LIBRARY_DIRS})
  
ADD_EXECUTABLE(CoreApp CoreApp.cpp)

TARGET_LINK_LIBRARIES(CoreApp 
                      optimized PocoFoundation      debug PocoFoundationd 
                      optimized PocoUtil            debug PocoUtild 
                      optimized org_blueberry_osgi debug org_blueberry_osgi${BLUEBERRY_DEBUG_POSTFIX})

SET(_plugin_deps 
  ${BLUEBERRY_ENABLED_PLUGINS} 
  ${MITK_CORE_ENABLED_PLUGINS}
  ${MITK_CORE_ENABLED_TEST_PLUGINS}
  )

IF(_plugin_deps)
  # Make sure all enabled plug-ins are up to date 
  ADD_DEPENDENCIES(CoreApp ${_plugin_deps})
ENDIF()

SET(BLUEBERRY_PLUGIN_CACHE_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugin_cache")

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

