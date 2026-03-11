#
# Deploy Qt runtime dependencies for a target.
#
# This installs Qt plugins, shared libraries, qt.conf, and platform-specific
# resources (e.g. QtWebEngine on Linux). Called centrally from mitkInstallRules
# AFTER runtime dependencies have been deployed, so that windeployqt can trace
# transitive Qt dependencies from MITK DLLs already present in bin/.
#
function(mitkFunctionDeployQt _target)
  get_target_property(_is_bundle ${_target} MACOSX_BUNDLE)

  if(APPLE AND _is_bundle)
    # For macOS bundles, Qt handles the .app bundle layout
    qt_generate_deploy_app_script(
      TARGET ${_target}
      OUTPUT_SCRIPT _deploy_script
      NO_TRANSLATIONS
      NO_COMPILER_RUNTIME
    )
  else()
    # For Windows/Linux, deploy Qt into bin/ with plugins in bin/plugins/
    set(_deploy_tool_options "")
    if(WIN32)
      set(_deploy_tool_options "DEPLOY_TOOL_OPTIONS \"--no-opengl-sw\"")
      if(OPENSSL_INCLUDE_DIR)
        get_filename_component(_openssl_root "${OPENSSL_INCLUDE_DIR}" DIRECTORY)
        string(APPEND _deploy_tool_options " \"--openssl-root\" \"${_openssl_root}\"")
      endif()
    endif()
    qt_generate_deploy_script(
      TARGET ${_target}
      OUTPUT_SCRIPT _deploy_script
      CONTENT "
qt_deploy_runtime_dependencies(
  EXECUTABLE \"bin/$<TARGET_FILE_NAME:${_target}>\"
  BIN_DIR \"bin\"
  LIB_DIR \"bin\"
  PLUGINS_DIR \"bin/plugins\"
  QML_DIR \"bin/qml\"
  NO_TRANSLATIONS
  NO_COMPILER_RUNTIME
  ${_deploy_tool_options}
)
"
    )
  endif()
  install(SCRIPT ${_deploy_script})

  # windeployqt deploys some files (WebEngine resources, translations)
  # relative to the install prefix root rather than bin/. Move them
  # into bin/ to keep everything in one directory and remove the
  # root-level duplicates.
  install(CODE "
    foreach(_qt_dir qml resources translations)
      set(_root_dir \"\${CMAKE_INSTALL_PREFIX}/\${_qt_dir}\")
      if(IS_DIRECTORY \"\${_root_dir}\")
        message(STATUS \"MITK_DEPLOY: Moving root-level \${_qt_dir}/ to bin/\${_qt_dir}/\")
        file(MAKE_DIRECTORY \"\${CMAKE_INSTALL_PREFIX}/bin/\${_qt_dir}\")
        file(GLOB _items \"\${_root_dir}/*\")
        foreach(_item \${_items})
          get_filename_component(_name \"\${_item}\" NAME)
          set(_dest \"\${CMAKE_INSTALL_PREFIX}/bin/\${_qt_dir}/\${_name}\")
          if(NOT EXISTS \"\${_dest}\")
            file(RENAME \"\${_item}\" \"\${_dest}\")
          endif()
        endforeach()
        file(REMOVE_RECURSE \"\${_root_dir}\")
      endif()
    endforeach()
  ")

  # On Linux, qt_deploy_runtime_dependencies() only traces shared library
  # dependencies. QtWebEngine additionally requires a helper executable,
  # data files, and locale files that must be deployed manually.
  if(LINUX AND TARGET Qt6::WebEngineCore)
    install(CODE "
      set(_webengine_process \"${QT_INSTALL_LIBEXECS}/QtWebEngineProcess\")
      if(EXISTS \"\${_webengine_process}\")
        message(STATUS \"MITK_DEPLOY: Installing QtWebEngineProcess\")
        file(INSTALL \"\${_webengine_process}\"
          DESTINATION \"\${CMAKE_INSTALL_PREFIX}/bin\"
          FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        )
      endif()

      set(_webengine_resources \"${QT_INSTALL_DATA}/resources\")
      if(IS_DIRECTORY \"\${_webengine_resources}\")
        message(STATUS \"MITK_DEPLOY: Installing QtWebEngine resources\")
        file(INSTALL \"\${_webengine_resources}/\"
          DESTINATION \"\${CMAKE_INSTALL_PREFIX}/bin/resources\"
        )
      endif()

      set(_webengine_locales \"${QT_INSTALL_TRANSLATIONS}/qtwebengine_locales\")
      if(IS_DIRECTORY \"\${_webengine_locales}\")
        message(STATUS \"MITK_DEPLOY: Installing QtWebEngine locale en-US.pak\")
        file(INSTALL \"\${_webengine_locales}/en-US.pak\"
          DESTINATION \"\${CMAKE_INSTALL_PREFIX}/bin/translations/qtwebengine_locales\"
        )
      endif()
    ")
  endif()
endfunction()
