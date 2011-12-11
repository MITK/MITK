
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
MACRO(MACRO_INSTALL_CTK_PLUGIN)

  MACRO_PARSE_ARGUMENTS(_INSTALL "TARGETS;DESTINATION" "" ${ARGN})

  IF(NOT _INSTALL_DESTINATION)
    SET(_INSTALL_DESTINATION "bin/")
  ELSE()
    SET(_INSTALL_DESTINATION "${_INSTALL_DESTINATION}/")
  ENDIF()
  
  FOREACH(_install_target ${_INSTALL_TARGETS})
    IF(TARGET ${_install_target})
      GET_TARGET_PROPERTY(_is_imported ${_install_target} IMPORTED)
      IF(_is_imported)
        GET_TARGET_PROPERTY(_import_loc_debug ${_install_target} IMPORTED_LOCATION_DEBUG)
        GET_TARGET_PROPERTY(_import_loc_release ${_install_target} IMPORTED_LOCATION_RELEASE)
        GET_FILENAME_COMPONENT(_target_filename_debug "${_import_loc_debug}" NAME)
        GET_FILENAME_COMPONENT(_target_filename_release "${_import_loc_release}" NAME)
        INSTALL(FILES ${_import_loc_debug}
                DESTINATION ${_INSTALL_DESTINATION}
                CONFIGURATIONS Debug)
        INSTALL(FILES ${_import_loc_release}
                DESTINATION ${_INSTALL_DESTINATION}
                CONFIGURATIONS Release)
        IF(UNIX AND NOT APPLE)
          IF(_target_filename_debug)
            INSTALL(CODE "FILE(RPATH_REMOVE
                               FILE \"\${CMAKE_INSTALL_PREFIX}/${_INSTALL_DESTINATION}/${_target_filename_debug}\")")
          ENDIF()
          IF(_target_filename_release)
            INSTALL(CODE "FILE(RPATH_REMOVE
                               FILE \"\${CMAKE_INSTALL_PREFIX}/${_INSTALL_DESTINATION}/${_target_filename_release}\")")
          ENDIF()
        ENDIF()
      ELSE()
        INSTALL(TARGETS ${_install_target}
                RUNTIME DESTINATION ${_INSTALL_DESTINATION}
                LIBRARY DESTINATION ${_INSTALL_DESTINATION}
                #ARCHIVE DESTINATION ${_INSTALL_DESTINATION}
                )
      ENDIF()
    ELSE()
      MESSAGE(WARNING "Ignoring unknown plug-in target \"${_install_target}\" for installation.")
    ENDIF()
  ENDFOREACH()
  
ENDMACRO()
