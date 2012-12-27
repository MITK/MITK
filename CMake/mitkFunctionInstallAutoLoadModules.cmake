
#! \brief Install auto-load modules needed by plug-ins.
#!
#! Uses the MITK_AUTOLOAD_TARGETS target property of the plug-in
#! targets to extract a list of auto-load target names which will
#! be installed in the given <code>DESTINATION</code> with an appropriate
#! sub-directory appended. If an entry in <code>TARGETS</code>
#! is not a known target, a warning will be issued. This macro
#! can also work on imported plug-in targets.
#!
#! \note This macro is usually not called directly. It is indirectly
#! called by a call to FunctionCreateBlueBerryApplication().
#!
#! \param PLUGINS A list of plug-in targets from which installable audo-load modules are derived.
#! \param DESTINATION The install destination, e.g. "bin".
function(mitkFunctionInstallAutoLoadModules)

  MACRO_PARSE_ARGUMENTS(_INSTALL "PLUGINS;DESTINATION" "" ${ARGN})

  if(NOT _INSTALL_PLUGINS)
    message(SEND_ERROR "PLUGINS argument is required")
  endif()

  if(NOT _INSTALL_DESTINATION)
    message(SEND_ERROR "DESTINATION argument is required")
  endif()

  foreach(_install_plugin ${_INSTALL_PLUGINS})
    if(TARGET ${_install_plugin})
      get_target_property(_autoload_targets ${_install_plugin} MITK_AUTOLOAD_TARGETS)
      if (_autoload_targets)
        foreach(_autoload_target ${_autoload_targets})
          get_target_property(_autoload_subdir ${_autoload_target} MITK_AUTOLOAD_DIRECTORY)
          if(NOT _autoload_subdir)
            message(WARNING "Target ${_autoload_target} does not seem to be an auto-load module. Skipping installation.")
          else(NOT _${_autoload_target}_installed)

            set(_module_install_dir ${_INSTALL_DESTINATION}/${_autoload_subdir})

            get_target_property(_is_imported ${_autoload_target} IMPORTED)
            if(_is_imported)
              get_target_property(_target_loc_debug ${_autoload_target} IMPORTED_LOCATION_DEBUG)
              get_target_property(_target_loc_release ${_autoload_target} IMPORTED_LOCATION_RELEASE)
            else()
              # Since we need to use install(FILE ...) we need to get the absolute path to the
              # module. This is a bit tricky and we only support Debug and Release configurations
              # on multi-configuration build systems.
              if(WIN32)
                get_target_property(_target_loc_debug ${_autoload_target} RUNTIME_OUTPUT_DIRECTORY)
              else()
                get_target_property(_target_loc_debug ${_autoload_target} LIBRARY_OUTPUT_DIRECTORY)
              endif()
              set(_target_loc_release ${_target_loc_debug})
              if(NOT CMAKE_CFG_INTDIR STREQUAL ".")
                set(_target_loc_debug "${_target_loc_debug}/Debug")
                set(_target_loc_release "${_target_loc_release}/Release")
              endif()
              set(_target_loc_debug ${_target_loc_debug}/${CMAKE_SHARED_LIBRARY_PREFIX}${_autoload_target}${CMAKE_SHARED_LIBRARY_SUFFIX})
              set(_target_loc_release ${_target_loc_release}/${CMAKE_SHARED_LIBRARY_PREFIX}${_autoload_target}${CMAKE_SHARED_LIBRARY_SUFFIX})
            endif()

            get_filename_component(_target_filename_debug "${_target_loc_debug}" NAME)
            get_filename_component(_target_filename_release "${_target_loc_release}" NAME)
            install(FILES ${_target_loc_debug}
                    DESTINATION ${_module_install_dir}
                    CONFIGURATIONS Debug)
            install(FILES ${_target_loc_release}
                    DESTINATION ${_module_install_dir}
                    CONFIGURATIONS Release)

            set(_${_autoload_target}_installed 1)

            if(UNIX AND NOT APPLE)
              if(_target_filename_debug)
                install(CODE "file(RPATH_REMOVE
                                   FILE \"\${CMAKE_INSTALL_PREFIX}/${_module_install_dir}/${_target_filename_debug}\")")
              endif()
              if(_target_filename_release)
                install(CODE "file(RPATH_REMOVE
                                   FILE \"\${CMAKE_INSTALL_PREFIX}/${_module_install_dir}/${_target_filename_release}\")")
              endif()
            endif()
          endif()
        endforeach()
      endif()
    else()
      message(WARNING "Ignoring unknown target \"${_install_target}\" for installation.")
    endif()
  endforeach()

endfunction()

function(BlueBerryApplicationInstallHook)

  MACRO_PARSE_ARGUMENTS(_INSTALL "APP_NAME;PLUGINS" "" ${ARGN})

  set(_destination bin)
  if(APPLE)
    set(_destination ${_INSTALL_APP_NAME}.app/Contents/MacOS)
  endif()

  mitkFunctionInstallAutoLoadModules(
    PLUGINS ${_INSTALL_PLUGINS}
    DESTINATION ${_destination}
   )

endfunction()
