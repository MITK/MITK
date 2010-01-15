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
    # no targets specified. get the main target from the plug-ins manifest
    MACRO_PARSE_MANIFEST("${_plugin_dir}/META-INF/MANIFEST.MF")
	STRING(REPLACE "." "_" _INSTALL_TARGETS ${BUNDLE-SYMBOLICNAME})
  ENDIF()
 
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
		  
  INSTALL(TARGETS ${_INSTALL_TARGETS}
                  RUNTIME DESTINATION ${_plugin_install_dir}/bin
				  LIBRARY DESTINATION ${_plugin_install_dir}/lib
				  ARCHIVE DESTINATION ${_plugin_install_dir}/lib
		 )

ENDMACRO()