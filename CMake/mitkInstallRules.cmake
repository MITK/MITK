# Install MITK icon and logo

MITK_INSTALL(FILES
  "${MITK_SOURCE_DIR}/mitk.ico"
  "${MITK_SOURCE_DIR}/mitk.bmp")

# Install Qt plugins

if(MITK_USE_Qt5)
  get_property(_qmake_location TARGET ${Qt5Core_QMAKE_EXECUTABLE} PROPERTY IMPORT_LOCATION)
  get_filename_component(_qmake_path ${_qmake_location} DIRECTORY)

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
    MITK_INSTALL(FILES
      "${_qmake_path}/../plugins/platforms/libqxcb.so"
      "${_qmake_path}/../plugins/platforms/libqxcb-glx-integration.so")
  endif()

  # Install Qt WebEngine

  set(_install_DESTINATION "")

  if(NOT APPLE)
    if(WIN32)
      MITK_INSTALL(FILES "${_qmake_path}/QtWebEngineProcess.exe")
    elseif(UNIX)
      MITK_INSTALL(FILES "${_qmake_path}/../libexec/QtWebEngineProcess")
    endif()

    set(_install_DESTINATION "resources")
    MITK_INSTALL(DIRECTORY "${_qmake_path}/../resources/")

    set(_install_DESTINATION "translations/qtwebengine_locales")
    MITK_INSTALL(DIRECTORY "${_qmake_path}/../translations/qtwebengine_locales/")
  endif()
endif()

# Install CTK Qt designer plugins

if(MITK_USE_CTK AND EXISTS ${CTK_DESIGNERPLUGINS_DIR})
  set(_install_DESTINATION "plugins/designer")
  if(NOT CMAKE_CFG_INTDIR STREQUAL ".")
    set(_release_dir "Release/")
  else()
    set(_release_dir "")
  endif()
  MITK_INSTALL(DIRECTORY "${CTK_DESIGNERPLUGINS_DIR}/designer/${_release_dir}")
endif()
