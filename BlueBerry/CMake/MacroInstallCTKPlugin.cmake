MACRO(MACRO_INSTALL_CTK_PLUGIN)

  MACRO_PARSE_ARGUMENTS(_INSTALL "TARGETS;DESTINATION" "" ${ARGN})

  IF(NOT _INSTALL_DESTINATION)
    SET(_INSTALL_DESTINATION "bin/")
  ELSE()
    SET(_INSTALL_DESTINATION "${_INSTALL_DESTINATION}/")
  ENDIF()
  
  FOREACH(_install_target ${_INSTALL_TARGETS})
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
  ENDFOREACH()
  
ENDMACRO()
