FIND_PACKAGE(BlueBerry REQUIRED)

# TODO: this can be removed after resolving factory issues, it's only necessary for the registration call in main()
MITK_USE_MODULE(MitkExt)

INCLUDE_DIRECTORIES(
  ${BLUEBERRY_PLUGIN_SOURCE_DIRS} 
  ${org.blueberry.osgi_BIN_DIR} # needed for generated berryConfig.h 
  ${Poco_INCLUDE_DIRS}
  ${ALL_INCLUDE_DIRECTORIES}
)
LINK_DIRECTORIES(${Poco_LIBRARY_DIRS}
                 ${BLUEBERRY_PLUGINS_OUTPUT_DIR}/org.blueberry.osgi/bin
                )
  
IF(MITK_SHOW_CONSOLE_WINDOW)
  ADD_EXECUTABLE(ExtApp MACOSX_BUNDLE ExtApp.cpp)
ELSE(MITK_SHOW_CONSOLE_WINDOW)
  ADD_EXECUTABLE(ExtApp MACOSX_BUNDLE WIN32 ExtApp.cpp)
ENDIF(MITK_SHOW_CONSOLE_WINDOW)

IF (WIN32)
  FIND_PACKAGE(Qt4)
  TARGET_LINK_LIBRARIES(ExtApp ${QT_QTCORE_LIBRARY} ${QT_QTMAIN_LIBRARY})
ENDIF(WIN32)

TARGET_LINK_LIBRARIES(ExtApp 
                      optimized PocoFoundation      debug PocoFoundationd 
                      optimized PocoUtil            debug PocoUtild 
                      optimized org_blueberry_osgi debug org_blueberry_osgi${BLUEBERRY_DEBUG_POSTFIX}
                      ${ALL_LIBRARIES})

SET_TARGET_PROPERTIES(ExtApp PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_RPATH};${CMAKE_INSTALL_RPATH}/BlueBerry/org.blueberry.osgi/bin")

SET(_plugin_deps ${BLUEBERRY_ENABLED_PLUGINS} ${MITK_CORE_ENABLED_PLUGINS} ${MITK_MODULES_ENABLED_PLUGINS})

IF(_plugin_deps)
  # Make sure all enabled plug-ins are up to date
  ADD_DEPENDENCIES(ExtApp ${_plugin_deps})
ENDIF()

SET(BLUEBERRY_PLUGIN_CACHE_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugin_cache")

CONFIGURE_FILE(ExtApp.ini
               ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ExtApp.ini)

IF(WIN32)
  FOREACH(EXTAPP_BUILD_TYPE debug release)
    mitkFunctionCreateWindowsBatchScript(startExtApp.bat.in
      ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/startExtApp_${EXTAPP_BUILD_TYPE}.bat
      ${EXTAPP_BUILD_TYPE})
  ENDFOREACH()
ELSE(WIN32)
  CONFIGURE_FILE(startExtApp.sh.in startExtApp.sh)
  FILE(COPY ${CMAKE_CURRENT_BINARY_DIR}/startExtApp.sh DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE)
ENDIF(WIN32)

MITK_INSTALL_TARGETS(EXECUTABLES ExtApp GLOB_PLUGINS )

IF(UNIX AND MITK_INSTALL_RPATH_RELATIVE AND NOT APPLE)
  INSTALL(PROGRAMS "${MITK_SOURCE_DIR}/CMake/RunInstalledApp.sh" DESTINATION "." RENAME ExtApp.sh)
ENDIF()
SET(MITK_CPACK_PACKAGE_EXECUTABLES ${MITK_CPACK_PACKAGE_EXECUTABLES} "ExtApp;MITK - ExtApp Application" CACHE INTERNAL "Collecting windows shortcuts to executables")
