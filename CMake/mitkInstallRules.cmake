MITK_INSTALL(FILES "${MITK_SOURCE_DIR}/mitk.ico")
MITK_INSTALL(FILES "${MITK_SOURCE_DIR}/mitk.bmp")

# Install CTK Qt (designer) plugins
if(MITK_USE_CTK)
  if(EXISTS ${CTK_QTDESIGNERPLUGINS_DIR})
    set(_qtplugin_install_destinations)
    if(MACOSX_BUNDLE_NAMES)
      foreach(bundle_name ${MACOSX_BUNDLE_NAMES})
        list(APPEND _qtplugin_install_destinations
             ${bundle_name}.app/Contents/MacOS/${_install_DESTINATION}/plugins/designer)
      endforeach()
    else()
      list(APPEND _qtplugin_install_destinations bin/plugins/designer)
    endif()

    set(_ctk_qt_plugin_folder_release)
    set(_ctk_qt_plugin_folder_debug)
    if(NOT CMAKE_CFG_INTDIR STREQUAL ".")
      set(_ctk_qt_plugin_folder_release "Release/")
      set(_ctk_qt_plugin_folder_debug "Debug/")
    endif()

    foreach(_qtplugin_install_dir ${_qtplugin_install_destinations})
      install(DIRECTORY "${CTK_QTDESIGNERPLUGINS_DIR}/designer/${_ctk_qt_plugin_folder_release}"
              DESTINATION ${_qtplugin_install_dir}
              CONFIGURATIONS Release
              )
      install(DIRECTORY "${CTK_QTDESIGNERPLUGINS_DIR}/designer/${_ctk_qt_plugin_folder_debug}"
              DESTINATION ${_qtplugin_install_dir}
              CONFIGURATIONS Debug
              )
    endforeach()
  endif()
endif()

# related to MITK:T19679
if(MACOSX_BUNDLE_NAMES)
  foreach(bundle_name ${MACOSX_BUNDLE_NAMES})
    get_property(_qmake_location TARGET ${Qt5Core_QMAKE_EXECUTABLE}
                 PROPERTY IMPORT_LOCATION)
    get_filename_component(_qmake_path "${_qmake_location}" DIRECTORY)
    install(FILES "${_qmake_path}/../plugins/platforms/libqcocoa.dylib"
            DESTINATION "${bundle_name}.app/Contents/MacOS/platforms"
            CONFIGURATIONS Release)
    install(FILES "${_qmake_path}/../plugins/sqldrivers/libqsqlite.dylib"
            DESTINATION "${bundle_name}.app/Contents/MacOS/sqldrivers"
            CONFIGURATIONS Release)
    install(FILES "${_qmake_path}/../plugins/iconengines/libqsvgicon.dylib"
            DESTINATION "${bundle_name}.app/Contents/MacOS/iconengines"
            CONFIGURATIONS Release)
    # related to MITK:T19679-InstallQtWebEnginProcess
    if(MITK_USE_Qt5)
        get_filename_component(ABS_DIR_HELPERS "${_qmake_path}/../lib/QtWebEngineCore.framework/Helpers" REALPATH)
        install(DIRECTORY ${ABS_DIR_HELPERS}
                DESTINATION "${bundle_name}.app/Contents/Frameworks/QtWebEngineCore.framework/"
                CONFIGURATIONS Release)
    endif()
  endforeach()
endif()

if(MITK_USE_Qt5)
  get_property(_qmake_location TARGET ${Qt5Core_QMAKE_EXECUTABLE}
               PROPERTY IMPORT_LOCATION)
  get_filename_component(_qmake_path "${_qmake_location}" DIRECTORY)

  if(WIN32)
    set(_prefix "")
    set(_ext ".dll")
    set(_configs CONFIGURATIONS Release)
    install(FILES "${_qmake_path}/../plugins/platforms/${_prefix}qwindows${_ext}"
            DESTINATION "bin/plugins/platforms"
            ${_configs})
    MITK_INSTALL(FILES "${_qmake_path}/QtWebEngineProcess.exe")
  elseif(UNIX)
    set(_prefix ".lib")
    if(APPLE)
      set(_ext ".dylib")
      set(_configs CONFIGURATIONS Release)
      install(FILES "${_qmake_path}/../plugins/platforms/${_prefix}qcocoa${_ext}"
              DESTINATION "bin/plugins/platforms"
              ${_configs})
      get_filename_component(ABS_DIR_HELPERS "${_qmake_path}/../lib/QtWebEngineCore.framework/Helpers" REALPATH)
      MITK_INSTALL_HELPER_APP(EXECUTABLES "${ABS_DIR_HELPERS}/QtWebEngineProcess")
    else()
      set(_ext "so")
      set(_configs "")
      install(FILES "${_qmake_path}/../plugins/platforms/${_prefix}qxcb${_ext}"
              DESTINATION "bin/plugins/platforms"
              ${_configs})
      install(FILES "${_qmake_path}/../plugins/xcbglintegrations/${_prefix}qxcb-glx-integration${_ext}"
              DESTINATION "bin/plugins/xcbglintegrations"
              ${_configs})
      MITK_INSTALL_HELPER_APP(EXECUTABLES "${_qmake_path}/../libexec/QtWebEngineProcess")
    endif()
  endif()

  install(FILES "${_qmake_path}/../plugins/sqldrivers/${_prefix}qsqlite${_ext}"
          DESTINATION "bin/plugins/sqldrivers"
          ${_configs})
  install(FILES "${_qmake_path}/../plugins/imageformats/${_prefix}qsvg${_ext}"
          DESTINATION "bin/plugins/imageformats"
          ${_configs})
  install(FILES "${_qmake_path}/../plugins/iconengines/${_prefix}qsvgicon${_ext}"
          DESTINATION "bin/plugins/iconengines"
          ${_configs})

  install(DIRECTORY "${_qmake_path}/../resources/"
          DESTINATION "bin/resources/"
          ${_configs})
  install(DIRECTORY "${_qmake_path}/../translations/qtwebengine_locales/"
          DESTINATION "bin/translations/qtwebengine_locales/"
          ${_configs})
endif()

#install Matchpoint libs that are currently not auto detected
if(MITK_USE_MatchPoint)
  install(DIRECTORY "${MITK_EXTERNAL_PROJECT_PREFIX}/bin/"
            DESTINATION "bin"
            FILES_MATCHING PATTERN "MAPUtilities*")
  install(DIRECTORY "${MITK_EXTERNAL_PROJECT_PREFIX}/bin/"
            DESTINATION "bin"
            FILES_MATCHING PATTERN "MAPAlgorithms*")
endif()

if(MITK_USE_BetData)
  install(DIRECTORY "${BetData_DIR}"
            DESTINATION "bin"
            FILES_MATCHING PATTERN "*")
endif()
