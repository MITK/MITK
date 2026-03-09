# Install MITK icon and logo

if(WIN32)
  install(FILES "${MITK_SOURCE_DIR}/mitk.ico" "${MITK_SOURCE_DIR}/mitk.bmp"
    DESTINATION bin)
endif()

# Install MITK modules, auto-load modules, and executables

get_property(_mitk_executable_targets GLOBAL PROPERTY MITK_EXECUTABLE_TARGETS)
if(_mitk_executable_targets)
  # Install CppMicroServices - it uses usMacroCreateModule() (not mitk_create_module())
  # so it is not tracked in MITK_MODULE_TARGETS, and US_NO_INSTALL disables its own
  # install rules.
  if(TARGET CppMicroServices)
    install(TARGETS CppMicroServices
      RUNTIME_DEPENDENCY_SET mitk_deps
      RUNTIME DESTINATION bin
      LIBRARY DESTINATION bin
      PUBLIC_HEADER DESTINATION include/CppMicroServices EXCLUDE_FROM_ALL
      PRIVATE_HEADER DESTINATION include/CppMicroServices EXCLUDE_FROM_ALL)
  endif()

  # Install MITK shared library modules to bin/ and add to the dependency set.
  # Without this, modules would not be installed - install(RUNTIME_DEPENDENCY_SET)
  # only installs transitive dependencies, not the set members themselves.
  get_property(_mitk_module_targets GLOBAL PROPERTY MITK_MODULE_TARGETS)
  foreach(_mitk_module_target ${_mitk_module_targets})
    if(TARGET ${_mitk_module_target})
      get_target_property(_target_type ${_mitk_module_target} TYPE)
      get_target_property(_is_autoload ${_mitk_module_target} MITK_AUTOLOAD_DIRECTORY)
      if(_target_type STREQUAL "SHARED_LIBRARY" AND NOT _is_autoload)
        install(TARGETS ${_mitk_module_target}
          RUNTIME_DEPENDENCY_SET mitk_deps
          RUNTIME DESTINATION bin
          LIBRARY DESTINATION bin)
      endif()

      # Install auto-load modules to their designated subdirectories
      get_target_property(_mitk_autoload_targets ${_mitk_module_target} MITK_AUTOLOAD_TARGETS)
      if(_mitk_autoload_targets)
        foreach(_mitk_autoload_target ${_mitk_autoload_targets})
          get_target_property(_mitk_autoload_directory ${_mitk_autoload_target} MITK_AUTOLOAD_DIRECTORY)
          if(_mitk_autoload_directory)
            install(TARGETS ${_mitk_autoload_target}
              RUNTIME_DEPENDENCY_SET mitk_deps
              RUNTIME DESTINATION bin/${_mitk_autoload_directory}
              LIBRARY DESTINATION bin/${_mitk_autoload_directory})
            if(LINUX)
              install(CODE "file(RPATH_REMOVE FILE \"\${CMAKE_INSTALL_PREFIX}/bin/${_mitk_autoload_directory}/$<TARGET_FILE_NAME:${_mitk_autoload_target}>\")")
            endif()
          endif()
        endforeach()
      endif()
    endif()
  endforeach()

  foreach(_mitk_executable_target ${_mitk_executable_targets})
    get_target_property(_no_install ${_mitk_executable_target} NO_INSTALL)
    if(_no_install)
      continue()
    endif()
    MITK_INSTALL_TARGETS(EXECUTABLES ${_mitk_executable_target})
    get_target_property(_command_line_app ${_mitk_executable_target} COMMAND_LINE_APP)
    if(_command_line_app)
      set(_source "RunInstalledCmdLineApp")
      set(_destination "apps")
    else()
      set(_source "RunInstalledApp")
      set(_destination ".")
    endif()
    get_target_property(_deploy_qt ${_mitk_executable_target} MITK_DEPLOY_QT)
    if(LINUX)
      install(PROGRAMS "${MITK_SOURCE_DIR}/CMake/${_source}.sh" DESTINATION "${_destination}" RENAME "${_mitk_executable_target}.sh")
    elseif(WIN32)
      if(_deploy_qt)
        install(PROGRAMS "${MITK_SOURCE_DIR}/CMake/RunInstalledWin32App.bat" DESTINATION "${_destination}" RENAME "${_mitk_executable_target}.bat")
      else()
        install(PROGRAMS "${MITK_SOURCE_DIR}/CMake/${_source}.bat" DESTINATION "${_destination}" RENAME "${_mitk_executable_target}.bat")
      endif()
    endif()
  endforeach()
endif()

# Install Python3 with MITK Python module

if(MITK_USE_Python3)
  if(APPLE)
    set(_python_dest "../Frameworks/Python.framework")
  else()
    set(_python_dest "python")
  endif()

  install(DIRECTORY "${MITK_BINARY_DIR}/python/"
    DESTINATION ${_python_dest}
    USE_SOURCE_PERMISSIONS)

  file(RELATIVE_PATH _rel_sitearch "${Python3_ROOT_DIR}" "${Python3_SITEARCH}")
  install(TARGETS mitk_python_bindings
    RUNTIME_DEPENDENCY_SET mitk_deps
    RUNTIME DESTINATION ${_python_dest}/${_rel_sitearch}/mitk
    LIBRARY DESTINATION ${_python_dest}/${_rel_sitearch}/mitk)
endif()

#-----------------------------------------------------------------------------
# Resolve and install ALL runtime dependencies in one pass.
#
# Every target that called install(TARGETS ... RUNTIME_DEPENDENCY_SET mitk_deps)
# has been recorded above. This single call resolves their transitive shared
# library dependencies and installs them to bin/.
#-----------------------------------------------------------------------------

mitkFunctionGetLibrarySearchPaths(_search_dirs Release RELEASE)

install(RUNTIME_DEPENDENCY_SET mitk_deps
  PRE_EXCLUDE_REGEXES
    "^api-ms-"
    "^ext-ms-"
    "^msvcp[0-9]+"
    "^vcruntime[0-9]+"
    "^concrt[0-9]+"
    "^vcomp[0-9]+"
    "^ucrtbase"
  POST_EXCLUDE_REGEXES
    "[/\\\\][Ww][Ii][Nn][Dd][Oo][Ww][Ss][/\\\\]"
    "^/usr/lib"
    "^/lib"
    "^/System"
    "python3[0-9]+[.]"
    ".*/plugins/.*"
  DIRECTORIES ${_search_dirs}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION bin
  FRAMEWORK DESTINATION bin
)

#-----------------------------------------------------------------------------
# Deploy Qt runtime dependencies (plugins, qt.conf, WebEngine resources).
#
# This runs AFTER install(RUNTIME_DEPENDENCY_SET) so that windeployqt can see
# all MITK DLLs in bin/ and correctly trace their transitive Qt dependencies.
#-----------------------------------------------------------------------------

if(MITK_USE_Qt6 AND _mitk_executable_targets)
  foreach(_mitk_executable_target ${_mitk_executable_targets})
    get_target_property(_no_install ${_mitk_executable_target} NO_INSTALL)
    if(_no_install)
      continue()
    endif()
    get_target_property(_deploy_qt ${_mitk_executable_target} MITK_DEPLOY_QT)
    if(_deploy_qt)
      _mitk_deploy_qt(${_mitk_executable_target})
    endif()
  endforeach()
endif()
