# Install the given plug-in directory to the desintation directory
#
# MACRO_INSTALL_PLUGIN(plugindir [TARGETS target...] [DESTINATION dest_dir])
#
MACRO(MACRO_INSTALL_PLUGIN _plugin_dir)

  MACRO_PARSE_ARGUMENTS(_INSTALL "TARGETS;DESTINATION" "" ${ARGN})
  
  # Convert _plugin_dir into an absolute path
  IF(NOT IS_ABSOLUTE _plugin_dir)
    SET(_plugin_dir "${CMAKE_CURRENT_SOURCE_DIR}/${_plugin_dir}")
  ENDIF()
  
  # Check if target names have been specified
  IF(NOT _INSTALL_TARGETS)
    # no targets specified. get the main target from the plug-ins manifest if it exists
    IF(EXISTS "${_plugin_dir}/META-INF/MANIFEST.MF")
      MACRO_PARSE_MANIFEST("${_plugin_dir}/META-INF/MANIFEST.MF")
      STRING(REPLACE "." "_" _INSTALL_TARGETS ${BUNDLE-SYMBOLICNAME})
    ENDIF()
  ENDIF()
 
  # Only continue if _INSTALL_TARGETS is set
  IF(_INSTALL_TARGETS)

  IF(NOT _INSTALL_DESTINATION)
    SET(_INSTALL_DESTINATION "bin/")
  ELSE()
    SET(_INSTALL_DESTINATION "${_INSTALL_DESTINATION}/")
  ENDIF()
  
  STRING(REGEX REPLACE ".*/(.+)/?$" "\\1" _toplevel_plugindir ${_plugin_dir})
  SET(_plugin_install_dir "${_INSTALL_DESTINATION}${_toplevel_plugindir}")
  
  # Install the directory without shared libraries
  INSTALL(DIRECTORY ${_plugin_dir}  DESTINATION ${_INSTALL_DESTINATION} CONFIGURATIONS Debug 
          PATTERN "Release/*" EXCLUDE 
      PATTERN "bin/*" EXCLUDE
      PATTERN "lib/*" EXCLUDE)
      
  INSTALL(DIRECTORY ${_plugin_dir} DESTINATION ${_INSTALL_DESTINATION} CONFIGURATIONS Release 
          PATTERN "Debug/*" EXCLUDE
          PATTERN "bin/*" EXCLUDE
      PATTERN "lib/*" EXCLUDE)
      
  SET(_target_install_rpath ${CMAKE_INSTALL_RPATH})
  FOREACH(_dep ${_plugin_dependencies})
    SET(_linklib_path "${${_dep}_OUT_DIR}")
    IF(_linklib_path)
      IF(BLUEBERRY_INSTALL_RPATH_RELATIVE)
        #MESSAGE("replace ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} with \".\" in ${_linklib_path} ")
        STRING(REPLACE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" "." _linklib_path "${_linklib_path}")
      ELSE()
        STRING(REPLACE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" "${CMAKE_INSTALL_PREFIX}/bin" _linklib_path "${_linklib_path}")
      ENDIF()
      LIST(APPEND _target_install_rpath "${_linklib_path}/bin")
    ENDIF()
  ENDFOREACH()
  SET_TARGET_PROPERTIES(${_INSTALL_TARGETS}
                        PROPERTIES INSTALL_RPATH "${_target_install_rpath}")
  
  FOREACH(_install_target ${_INSTALL_TARGETS})
    GET_TARGET_PROPERTY(_is_imported ${_install_target} IMPORTED)
    IF(_is_imported)
      GET_TARGET_PROPERTY(_import_loc_debug ${_install_target} IMPORTED_LOCATION_DEBUG)
      GET_TARGET_PROPERTY(_import_loc_release ${_install_target} IMPORTED_LOCATION_RELEASE)
      INSTALL(FILES ${_import_loc_debug}
              DESTINATION ${_plugin_install_dir}/bin
              CONFIGURATIONS Debug)
      INSTALL(FILES ${_import_loc_release}
              DESTINATION ${_plugin_install_dir}/bin
              CONFIGURATIONS Release)
    ELSE()
      INSTALL(TARGETS ${_INSTALL_TARGETS}
              RUNTIME DESTINATION ${_plugin_install_dir}/bin
              LIBRARY DESTINATION ${_plugin_install_dir}/bin
              #ARCHIVE DESTINATION ${_plugin_install_dir}/bin
              )
    ENDIF()
  ENDFOREACH()

  ENDIF() # _INSTALL_TARGETS
  
ENDMACRO()
