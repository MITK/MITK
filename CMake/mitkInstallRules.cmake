# Install MITK icon and logo

MITK_INSTALL(FILES
  "${MITK_SOURCE_DIR}/mitk.ico"
  "${MITK_SOURCE_DIR}/mitk.bmp")

# Helper vars

if(WIN32)
  set(_prefix "")
  set(_ext ".dll")
elseif(UNIX)
  set(_prefix "lib")
  if(APPLE)
    set(_ext ".dylib")
  else()
    set(_ext ".so")
  endif()
endif()

# Install MITK executables including auto-load modules

get_property(_mitk_executable_targets GLOBAL PROPERTY MITK_EXECUTABLE_TARGETS)
if(_mitk_executable_targets)
  get_property(_mitk_module_targets GLOBAL PROPERTY MITK_MODULE_TARGETS)
  foreach(_mitk_module_target ${_mitk_module_targets})
    if(TARGET ${_mitk_module_target})
      get_target_property(_mitk_autoload_targets ${_mitk_module_target} MITK_AUTOLOAD_TARGETS)
      if (_mitk_autoload_targets)
        foreach(_mitk_autoload_target ${_mitk_autoload_targets})
          get_target_property(_mitk_autoload_directory ${_mitk_autoload_target} MITK_AUTOLOAD_DIRECTORY)
          if (_mitk_autoload_directory)
            if(WIN32)
              get_target_property(_target_location ${_mitk_autoload_target} RUNTIME_OUTPUT_DIRECTORY)
            else()
              get_target_property(_target_location ${_mitk_autoload_target} LIBRARY_OUTPUT_DIRECTORY)
            endif()
            if(NOT CMAKE_CFG_INTDIR STREQUAL ".")
              set(_target_location "${_target_location}/Release")
            endif()
            set(_mitk_autoload_target_filename "${_prefix}${_mitk_autoload_target}${_ext}")
            set(_mitk_autoload_target_filepath "${_target_location}/${_mitk_autoload_target_filename}")
            set(_install_DESTINATION "${_mitk_autoload_directory}")
            MITK_INSTALL(FILES ${_mitk_autoload_target_filepath})
            if(UNIX AND NOT APPLE)
              install(CODE "file(RPATH_REMOVE FILE \"\${CMAKE_INSTALL_PREFIX}/bin/${_mitk_autoload_directory}/${_mitk_autoload_target_filename}\")")
            endif()
          endif()
        endforeach()
      endif()
    endif()
  endforeach()

  set(_install_DESTINATION "")

  foreach(_mitk_executable_target ${_mitk_executable_targets})
    get_target_property(_no_install ${_mitk_executable_target} NO_INSTALL)
    if(_no_install)
      continue()
    endif()
    MITK_INSTALL_TARGETS(EXECUTABLES ${_mitk_executable_target} GLOB_PLUGINS)
    if(UNIX AND NOT APPLE)
      install(PROGRAMS "${MITK_SOURCE_DIR}/CMake/RunInstalledApp.sh" DESTINATION "." RENAME "${_mitk_executable_target}.sh")
    elseif(WIN32)
      get_target_property(_win32_exec ${_mitk_executable_target} WIN32_EXECUTABLE)
      if(_win32_exec)
        install(PROGRAMS "${MITK_SOURCE_DIR}/CMake/RunInstalledWin32App.bat" DESTINATION "." RENAME "${_mitk_executable_target}.bat")
      else()
        install(PROGRAMS "${MITK_SOURCE_DIR}/CMake/RunInstalledApp.bat" DESTINATION "." RENAME "${_mitk_executable_target}.bat")
      endif()
    endif()
  endforeach()
endif()

# Install PythonQt

if(MITK_USE_Python3 AND PythonQt_DIR)
  set(_python_qt_lib "${PythonQt_DIR}/")
  if(WIN32)
    set(_python_qt_lib "${_python_qt_lib}bin")
  else()
    set(_python_qt_lib "${_python_qt_lib}lib")
  endif()
  set(_python_qt_lib "${_python_qt_lib}/${_prefix}PythonQt${_ext}")
  MITK_INSTALL(FILES ${_python_qt_lib})
endif()

# Install Qt plugins

if(MITK_USE_Qt5)
  get_property(_qmake_location TARGET ${Qt5Core_QMAKE_EXECUTABLE} PROPERTY IMPORT_LOCATION)
  get_filename_component(_qmake_path ${_qmake_location} DIRECTORY)

  set(_install_DESTINATION "plugins/sqldrivers")
  MITK_INSTALL(FILES "${_qmake_path}/../plugins/sqldrivers/${_prefix}qsqlite${_ext}")

  set(_install_DESTINATION "plugins/imageformats")
  MITK_INSTALL(FILES "${_qmake_path}/../plugins/imageformats/${_prefix}qsvg${_ext}")

  set(_install_DESTINATION "plugins/iconengines")
  MITK_INSTALL(FILES "${_qmake_path}/../plugins/iconengines/${_prefix}qsvgicon${_ext}")

  # Install platform-specific Qt plugins

  set(_install_DESTINATION "plugins/platforms")

  if(WIN32)
    MITK_INSTALL(FILES "${_qmake_path}/../plugins/platforms/qwindows.dll")
  elseif(APPLE)
    MITK_INSTALL(FILES "${_qmake_path}/../plugins/platforms/libqcocoa.dylib")
  elseif(UNIX)
    MITK_INSTALL(FILES "${_qmake_path}/../plugins/platforms/libqxcb.so")

    set(_install_DESTINATION "plugins/xcbglintegrations")
    MITK_INSTALL(FILES "${_qmake_path}/../plugins/xcbglintegrations/libqxcb-glx-integration.so")
  endif()

  # Install platform-specific Qt styles

  set(_install_DESTINATION "plugins/styles")

  if(WIN32)
    MITK_INSTALL(FILES "${_qmake_path}/../plugins/styles/qwindowsvistastyle.dll")
  elseif(APPLE)
    MITK_INSTALL(FILES "${_qmake_path}/../plugins/styles/libqmacstyle.dylib")
  endif()

  # Install Qt WebEngine

  if(APPLE)
    set(_install_DESTINATION "../Frameworks/QtWebEngineCore.framework")

    get_filename_component(_real_path "${_qmake_path}/../lib/QtWebEngineCore.framework/Helpers" REALPATH)
    MITK_INSTALL(DIRECTORY ${_real_path} USE_SOURCE_PERMISSIONS)

    # Translations are included in the Resources directory of
    # QtWebEngineCore.framework and are installed by default.
  else()
    set(_install_DESTINATION "")

    if(WIN32)
      MITK_INSTALL(PROGRAMS "${_qmake_path}/QtWebEngineProcess.exe")
    elseif(UNIX)
      MITK_INSTALL(PROGRAMS "${_qmake_path}/../libexec/QtWebEngineProcess")
    endif()

    # make sure resources and translations exist and try system location as well
    if(EXISTS "${_qmake_path}/../resources")
      MITK_INSTALL(DIRECTORY "${_qmake_path}/../resources")
    elseif(EXISTS "/usr/share/qt5/resources")
      MITK_INSTALL(DIRECTORY "/usr/share/qt5/resources")
    else()
      message(WARNING "No webengine resources found!")
    endif()

    set(_install_DESTINATION "translations")
    if(EXISTS "${_qmake_path}/../translations/qtwebengine_locales")
      MITK_INSTALL(DIRECTORY "${_qmake_path}/../translations/qtwebengine_locales")
    elseif(EXISTS "/usr/share/qt5/translations/qtwebengine_locales")
      MITK_INSTALL(DIRECTORY "/usr/share/qt5/translations/qtwebengine_locales")
    else()
      message(WARNING "No webengine translations found!")
    endif()

   endif()
endif()

set(_install_DESTINATION "")

# Install MatchPoint binaries that are not auto-detected
if(MITK_USE_MatchPoint)
  MITK_INSTALL(DIRECTORY "${MITK_EXTERNAL_PROJECT_PREFIX}/bin/" FILES_MATCHING PATTERN "MapUtilities*")
  MITK_INSTALL(DIRECTORY "${MITK_EXTERNAL_PROJECT_PREFIX}/bin/" FILES_MATCHING PATTERN "MapAlgorithms*")
endif()

# IMPORTANT: Restore default install destination! Do not edit this file beyond this line!
set(_install_DESTINATION "")
