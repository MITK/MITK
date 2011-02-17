PROJECT(org_blueberry_osgi)

SET(PLUGIN_export_directive "BERRY_OSGI")

INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/files.cmake)
SET(PLUGIN_SRCS ${CPP_FILES})

# Files which should be processed by Qts moc
SET(PLUGIN_MOC_SRCS
  src/internal/berryCTKPluginActivator.h
  src/internal/berryExtensionPointService.h
)

# Qt Designer files which should be processed by Qts uic
SET(PLUGIN_UI_FORMS
)

# QRC Files which should be compiled into the plugin
SET(PLUGIN_resources
)

SET(PLUGIN_cached_resources
  plugin.xml
)

#Compute the plugin dependencies
ctkFunctionGetTargetLibraries(PLUGIN_target_libraries)

ctkMacroBuildPlugin(
  NAME ${PROJECT_NAME}
  INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/src
  EXPORT_DIRECTIVE ${PLUGIN_export_directive}
  SRCS ${PLUGIN_SRCS}
  MOC_SRCS ${PLUGIN_MOC_SRCS}
  UI_FORMS ${PLUGIN_UI_FORMS}
  RESOURCES ${PLUGIN_resources}
  CACHED_RESOURCEFILES ${PLUGIN_cached_resources}
  TARGET_LIBRARIES ${PLUGIN_target_libraries}
)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} optimized PocoUtil debug PocoUtild optimized PocoXML debug PocoXMLd mbilog)

ADD_EXECUTABLE(${OSGI_APP} MACOSX_BUNDLE "src/application/berryMain.cpp")
# ADD_EXECUTABLE(${OSGI_APP} "src/application/berryMain.cpp")
TARGET_LINK_LIBRARIES(${OSGI_APP} ${PROJECT_NAME} mbilog)
IF (BLUEBERRY_ENABLED_TEST_PLUGINS)
  ADD_DEPENDENCIES(${OSGI_APP} ${BLUEBERRY_ENABLED_TEST_PLUGINS})
  ADD_DEPENDENCIES(BlueBerry ${OSGI_APP})
  SET_PROPERTY(TARGET ${OSGI_APP} APPEND PROPERTY LABELS BlueBerry)
ENDIF()

SET(BLUEBERRY_PLUGIN_CACHE_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugin_cache")

CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/src/berryConfig.h.in" "${CMAKE_CURRENT_BINARY_DIR}/berryConfig.h" @ONLY)

CONFIGURE_FILE(src/application/solstice.ini
               ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${OSGI_APP}.ini)

#SET_TARGET_PROPERTIES(${OSGI_APP} PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_RPATH};${CMAKE_INSTALL_PREFIX}/bin/BlueBerry/org.blueberry.osgi/bin")

# MITK_INSTALL_TARGETS(EXECUTABLES ${OSGI_APP} GLOB_PLUGINS )
# MITK_INSTALL_TARGETS(EXECUTABLES ${OSGI_APP} LIBRARY_DIRS ${CMAKE_INSTALL_PREFIX}/bin)
# MITK_INSTALL_TARGETS(EXECUTABLES ${OSGI_APP} LIBRARY_DIRS ${CMAKE_INSTALL_PREFIX}/bin PLUGINS ${CMAKE_INSTALL_PREFIX}/bin/BlueBerry/org.blueberry.osgi/bin/liborg_blueberry_osgid.dylib)

