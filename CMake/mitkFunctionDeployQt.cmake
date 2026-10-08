#
# Deploy Qt runtime dependencies for a target.
#
# This installs Qt plugins, shared libraries, and qt.conf. Called centrally from
# mitkInstallRules
# AFTER runtime dependencies have been deployed, so that windeployqt can trace
# transitive Qt dependencies from MITK DLLs already present in bin/.
#
function(mitkFunctionDeployQt _target)
  set(_exclude_plugin_types
    generic
    networkinformation
    position
    qmltooling
    tls
  )

  set(_exclude_plugins
    qsqlibase
    qsqlmimer
    qsqlmysql
    qsqloci
    qsqlodbc
    qsqlpsql
  )

  set(_include_plugins
    qsqlite
  )

  get_target_property(_is_bundle ${_target} MACOSX_BUNDLE)

  if(APPLE AND _is_bundle)
    # macdeployqt rewrites the library references of the bundle executable
    # only. The other executables installed into the bundle, such as the
    # command-line apps, have to be named explicitly, or they keep references
    # to the libraries of the build host. Naming any makes macdeployqt write
    # all references of the bundle relative to @loader_path instead of
    # @executable_path, so they also resolve when another executable, such
    # as the bundled Python, loads the libraries.
    set(_additional_executables "")
    get_property(_executable_targets GLOBAL PROPERTY MITK_EXECUTABLE_TARGETS)
    foreach(_executable_target IN LISTS _executable_targets)
      if(_executable_target STREQUAL _target)
        continue()
      endif()
      get_target_property(_no_install ${_executable_target} NO_INSTALL)
      get_target_property(_is_other_bundle ${_executable_target} MACOSX_BUNDLE)
      if(_no_install OR _is_other_bundle)
        continue()
      endif()
      list(APPEND _additional_executables
        "\"$<TARGET_FILE_NAME:${_target}>.app/Contents/MacOS/$<TARGET_FILE_NAME:${_executable_target}>\"")
    endforeach()
    list(JOIN _additional_executables " " _additional_executables)

    qt_generate_deploy_script(
      TARGET ${_target}
      OUTPUT_SCRIPT _deploy_script
      CONTENT "
qt_deploy_runtime_dependencies(
  EXECUTABLE \"$<TARGET_FILE_NAME:${_target}>.app\"
  ADDITIONAL_EXECUTABLES ${_additional_executables}
  NO_TRANSLATIONS
)
"
    )
  else()
    # For Windows/Linux, deploy Qt into bin/ with plugins in bin/plugins/
    set(_win_deploy_tool_options "")

    if(WIN32)
      set(_win_deploy_tool_options "DEPLOY_TOOL_OPTIONS --no-opengl-sw")
      if(OPENSSL_INCLUDE_DIR)
        get_filename_component(_openssl_root "${OPENSSL_INCLUDE_DIR}" DIRECTORY)
        string(APPEND _win_deploy_tool_options " --openssl-root \"${_openssl_root}\"")
      endif()
    endif()

    # On Linux, override QT_DEPLOY_* variables so that everything goes under
    # bin/ instead of the default lib/, libexec/, plugins/, translations/ at
    # the prefix root.
    set(_deploy_prefix_overrides "")
    if(LINUX)
      set(_deploy_prefix_overrides "
set(QT_DEPLOY_BIN_DIR \"bin\")
set(QT_DEPLOY_LIB_DIR \"bin\")
set(QT_DEPLOY_LIBEXEC_DIR \"bin\")
set(QT_DEPLOY_PLUGINS_DIR \"bin/plugins\")
set(QT_DEPLOY_QML_DIR \"bin/qml\")
set(QT_DEPLOY_TRANSLATIONS_DIR \"bin/translations\")
set(QT_DEPLOY_DATA_DIR \"bin\")
")
    endif()

    list(JOIN _exclude_plugin_types " " _exclude_plugin_types)
    list(JOIN _exclude_plugins " " _exclude_plugins)
    list(JOIN _include_plugins " " _include_plugins)

    qt_generate_deploy_script(
      TARGET ${_target}
      OUTPUT_SCRIPT _deploy_script
      CONTENT "
${_deploy_prefix_overrides}
qt_deploy_runtime_dependencies(
  EXECUTABLE \"$<TARGET_FILE:${_target}>\"
  BIN_DIR \"bin\"
  LIB_DIR \"bin\"
  PLUGINS_DIR \"bin/plugins\"
  QML_DIR \"bin/qml\"
  NO_TRANSLATIONS
  NO_COMPILER_RUNTIME
  EXCLUDE_PLUGIN_TYPES ${_exclude_plugin_types}
  EXCLUDE_PLUGINS ${_exclude_plugins}
  INCLUDE_PLUGINS ${_include_plugins}
  ${_win_deploy_tool_options}
)
"
    )
  endif()
  install(SCRIPT ${_deploy_script})

  # macdeployqt ignores the plugin selection, and the generic deployment of Qt
  # before 6.12 also ignores the exclusion by name, so remove the excluded
  # plugins afterwards. A deployed plugin whose dependencies are not bundled
  # fails to load on other machines.
  if((APPLE AND _is_bundle) OR LINUX)
    if(APPLE)
      set(_plugins_dir "$<TARGET_FILE_NAME:${_target}>.app/Contents/PlugIns")
    else()
      set(_plugins_dir "bin/plugins")
    endif()
    install(CODE "
      set(_plugins_dir \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${_plugins_dir}\")
      foreach(_type ${_exclude_plugin_types})
        file(REMOVE_RECURSE \"\${_plugins_dir}/\${_type}\")
      endforeach()
      foreach(_plugin ${_exclude_plugins})
        file(GLOB _files \"\${_plugins_dir}/*/lib\${_plugin}.*\")
        if(_files)
          file(REMOVE \${_files})
        endif()
      endforeach()
    ")
  endif()

  # On Linux, write qt.conf with a relative prefix so the deployed bin/ layout
  # stays relocatable: Qt then resolves plugins at bin/plugins/, QML at
  # bin/qml/, etc. relative to bin/. The Qt deploy script can otherwise leave
  # an absolute staging path as the prefix.
  if(LINUX)
    install(CODE "
      set(_qt_conf \"\${CMAKE_INSTALL_PREFIX}/bin/qt.conf\")
      message(STATUS \"MITK_DEPLOY: Writing \${_qt_conf}\")
      file(WRITE \"\${_qt_conf}\" \"[Paths]\nPrefix = .\n\")
    ")
  endif()

  if(WIN32)
    # windeployqt places translations at the prefix root.
    # Move them into bin/ to keep everything in one directory.
    install(CODE "
      foreach(_qt_dir translations)
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
  endif()
endfunction()
