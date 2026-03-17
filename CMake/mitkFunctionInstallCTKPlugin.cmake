
#! \brief Install CTK plug-ins.
#!
#! Installs the given list of CTK plug-in targets in the given
#! <code>DESTINATION</code>. If an entry in <code>TARGETS</code>
#! is not a known target, a warning will be issued. This macro
#! can also install imported plug-in targets.
#!
#! \note This macro is usually not called directly. It is used in
#! mitk_create_module() and mitkFunctionInstallThirdPartyCTKPlugins().
#!
#! \param TARGETS (optional) A list of plug-ins (target names) to install.
#! \param DESTINATION (optional) The install destination. Defaults to "bin/".
function(mitkFunctionInstallCTKPlugin)

  cmake_parse_arguments(_INSTALL "" "DESTINATION;RUNTIME_DEPENDENCY_SET" "TARGETS" ${ARGN})

  if(NOT _INSTALL_DESTINATION)
    set(_INSTALL_DESTINATION "bin/")
  else()
    set(_INSTALL_DESTINATION "${_INSTALL_DESTINATION}/")
  endif()

  foreach(_install_target ${_INSTALL_TARGETS})
    if(TARGET ${_install_target})
      get_target_property(_is_imported ${_install_target} IMPORTED)
      if(_is_imported)
        get_target_property(_import_loc_debug ${_install_target} IMPORTED_LOCATION_DEBUG)
        get_target_property(_import_loc_release ${_install_target} IMPORTED_LOCATION_RELEASE)
        get_filename_component(_target_filename_debug "${_import_loc_debug}" NAME)
        get_filename_component(_target_filename_release "${_import_loc_release}" NAME)
        install(FILES ${_import_loc_debug}
                DESTINATION ${_INSTALL_DESTINATION}
                CONFIGURATIONS Debug)
        install(FILES ${_import_loc_release}
                DESTINATION ${_INSTALL_DESTINATION}
                CONFIGURATIONS Release)
        if(LINUX)
          if(_target_filename_debug)
            install(CODE "file(RPATH_SET
                               FILE \"\${CMAKE_INSTALL_PREFIX}/${_INSTALL_DESTINATION}/${_target_filename_debug}\"
                               NEW_RPATH \"\$ORIGIN/..\")")
          endif()
          if(_target_filename_release)
            install(CODE "file(RPATH_SET
                               FILE \"\${CMAKE_INSTALL_PREFIX}/${_INSTALL_DESTINATION}/${_target_filename_release}\"
                               NEW_RPATH \"\$ORIGIN/..\")")
          endif()
        elseif(APPLE)
          # file(RPATH_SET) only supports ELF/XCOFF, not Mach-O.
          # Use install_name_tool to add the install RPATH instead.
          # Stale build-tree RPATHs are harmless (dead references).
          if(_target_filename_debug)
            install(CODE "
              set(_file \"\${CMAKE_INSTALL_PREFIX}/${_INSTALL_DESTINATION}/${_target_filename_debug}\")
              if(EXISTS \"\${_file}\")
                execute_process(COMMAND install_name_tool -add_rpath \"@loader_path/..\" \"\${_file}\" ERROR_QUIET)
              endif()")
          endif()
          if(_target_filename_release)
            install(CODE "
              set(_file \"\${CMAKE_INSTALL_PREFIX}/${_INSTALL_DESTINATION}/${_target_filename_release}\")
              if(EXISTS \"\${_file}\")
                execute_process(COMMAND install_name_tool -add_rpath \"@loader_path/..\" \"\${_file}\" ERROR_QUIET)
              endif()")
          endif()
        endif()
      else()
        set(_depset_arg "")
        if(_INSTALL_RUNTIME_DEPENDENCY_SET)
          set(_depset_arg RUNTIME_DEPENDENCY_SET ${_INSTALL_RUNTIME_DEPENDENCY_SET})
        endif()
        install(TARGETS ${_install_target}
                ${_depset_arg}
                RUNTIME DESTINATION ${_INSTALL_DESTINATION}
                LIBRARY DESTINATION ${_INSTALL_DESTINATION}
                )
      endif()
    else()
      message(WARNING "Ignoring unknown plug-in target \"${_install_target}\" for installation.")
    endif()
  endforeach()

endfunction()
