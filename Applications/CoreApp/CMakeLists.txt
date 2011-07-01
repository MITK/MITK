FIND_PACKAGE(BlueBerry REQUIRED)

INCLUDE(${QT_USE_FILE})

INCLUDE_DIRECTORIES(
  ${BLUEBERRY_PLUGIN_SOURCE_DIRS}
  ${org_blueberry_osgi_INCLUDE_DIRS}
  ${Poco_INCLUDE_DIRS}
  ${mbilog_INCLUDE_DIRS}
)

LINK_DIRECTORIES(${Poco_LIBRARY_DIRS})

IF(MITK_SHOW_CONSOLE_WINDOW)
  ADD_EXECUTABLE(CoreApp MACOSX_BUNDLE CoreApp.cpp)
ELSE(MITK_SHOW_CONSOLE_WINDOW)
  ADD_EXECUTABLE(CoreApp MACOSX_BUNDLE WIN32 CoreApp.cpp)
ENDIF(MITK_SHOW_CONSOLE_WINDOW)

IF (WIN32)
  FIND_PACKAGE(Qt4)
  TARGET_LINK_LIBRARIES(CoreApp ${QT_QTCORE_LIBRARY} ${QT_QTMAIN_LIBRARY})
ENDIF(WIN32)

TARGET_LINK_LIBRARIES(CoreApp 
                      optimized PocoFoundation      debug PocoFoundationd 
                      optimized PocoUtil            debug PocoUtild 
                      org_blueberry_osgi)

# subproject support
ADD_DEPENDENCIES(MITK-CoreUI CoreApp)
IF(MITK_ENABLE_GUI_TESTING)
  ADD_DEPENDENCIES(MITK-CoreUI solstice)
ENDIF()

SET(_plugin_deps 
  ${BLUEBERRY_ENABLED_PLUGINS} 
  ${MITK_CORE_ENABLED_PLUGINS}
  )

IF(_plugin_deps)
  # Make sure all enabled plug-ins are up to date 
  ADD_DEPENDENCIES(CoreApp ${_plugin_deps})
ENDIF()

SET(BLUEBERRY_PLUGIN_CACHE_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugin_cache")

CONFIGURE_FILE(CoreApp.ini
               ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/CoreApp.ini)

IF(WIN32)
  FOREACH(COREAPP_BUILD_TYPE debug release)
    mitkFunctionCreateWindowsBatchScript(startCoreApp.bat.in
      ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/startCoreApp_${COREAPP_BUILD_TYPE}.bat
      ${COREAPP_BUILD_TYPE})
  ENDFOREACH()
ENDIF(WIN32)

MITK_INSTALL_TARGETS(EXECUTABLES CoreApp GLOB_PLUGINS )
mitkFunctionInstallProvisioningFiles(${BLUEBERRY_PLUGIN_PROVISIONING_FILE} ${MITK_COREAPP_PROVISIONING_FILE})

IF(UNIX AND NOT APPLE)
  INSTALL(PROGRAMS "${MITK_SOURCE_DIR}/CMake/RunInstalledApp.sh" DESTINATION "." RENAME CoreApp.sh)
ENDIF()
SET(MITK_CPACK_PACKAGE_EXECUTABLES ${MITK_CPACK_PACKAGE_EXECUTABLES} "CoreApp;MITK - CoreApp Application" CACHE INTERNAL "Collecting windows shortcuts to executables")

