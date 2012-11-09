
#! \brief Install CTK plug-ins.
#!
#! Installs the given list of CTK plug-in targets in the given
#! <code>DESTINATION</code>. If an entry in <code>TARGETS</code>
#! is not a known target, a warning will be issued. This macro
#! can also install imported plug-in targets.
#!
#! \note This macro is usually not called directly. It is used in
#! MACRO_CREATE_CTK_PLUGIN() and FunctionInstallThirdPartyCTKPlugins().
#!
#! \param TARGETS (optional) A list of plug-ins (target names) to install.
#! \param DESTINATION (optional) The install destination. Defaults to "bin/".
macro(MACRO_INSTALL_CTK_PLUGIN)

  MACRO_PARSE_ARGUMENTS(_INSTALL "TARGETS;DESTINATION" "" ${ARGN})

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
        if(UNIX AND NOT APPLE)
          if(_target_filename_debug)
            install(CODE "file(RPATH_REMOVE
                               FILE \"\${CMAKE_INSTALL_PREFIX}/${_INSTALL_DESTINATION}/${_target_filename_debug}\")")
          endif()
          if(_target_filename_release)
            install(CODE "file(RPATH_REMOVE
                               FILE \"\${CMAKE_INSTALL_PREFIX}/${_INSTALL_DESTINATION}/${_target_filename_release}\")")
          endif()
        endif()
      else()
        install(TARGETS ${_install_target}
                RUNTIME DESTINATION ${_INSTALL_DESTINATION}
                LIBRARY DESTINATION ${_INSTALL_DESTINATION}
                #ARCHIVE DESTINATION ${_INSTALL_DESTINATION}
                )
      endif()
    else()
      message(WARNING "Ignoring unknown plug-in target \"${_install_target}\" for installation.")
    endif()
  endforeach()

endmacro()
