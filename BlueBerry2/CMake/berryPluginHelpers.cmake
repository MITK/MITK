# _MACRO_CREATE_PLUGIN_NAME(output_name
#                           INPUT dir
#                           [BASEDIR basedir]
#                           )
#
# sets output_name to the plugin-name
# the plugin-name is computed by removing basedir from dir and
# replacing slashes with points
#
# Example: BASEDIR=/root
#          INPUT=/root/org/blueberry/ui.qt
#          output_name=org.blueberry.ui.qt
#
MACRO(_MACRO_CREATE_PLUGIN_NAME output_name)

  MACRO_PARSE_ARGUMENTS(_CREATE_NAME "INPUT;BASEDIR" "" ${ARGN})

  STRING(REPLACE "${_CREATE_NAME_BASEDIR}/" "" _name_with_slashes ${_CREATE_NAME_INPUT})
  STRING(REPLACE "/" "." _name_with_points ${_name_with_slashes})

  SET(${output_name} ${_name_with_points})

ENDMACRO()

MACRO(_MACRO_REQUIRED_BUNDLES_LIST output_list input_file)

  SET(_file "${input_file}")
  FILE(STRINGS "${_file}" _mfline LENGTH_MINIMUM 1 REGEX " *Require-Bundle *: *[^ ]+")
  STRING(REGEX REPLACE "^[^:]*[:](.*)" "\\1" _reqb_input "${_mfline}")

  SET(_reqb_dependencies )
  IF(_reqb_input)
    STRING(REPLACE "," ";" _reqb_dependencies ${_reqb_input})
  ENDIF(_reqb_input)

  FOREACH(_dep ${_reqb_dependencies})
    # remove leading and trailing whitespace
    STRING(STRIP ${_dep} _trimeddep)
    LIST(APPEND ${output_list} ${_trimeddep})
  ENDFOREACH(_dep ${_reqb_dependencies})
ENDMACRO()



# _MACRO_SETUP_PLUGIN_DEPENDENCIES(
#      _explicit_libs
#      PLUGIN_NAME _plugin_name
#      )
#
MACRO(_MACRO_SETUP_PLUGIN_DEPENDENCIES _explicit_libs)

  SET(_plugin_src_dir ${${BUNDLE-SYMBOLICNAME}_SRC_DIR})
  SET(_plugin_dependencies ${REQUIRE-BUNDLE})
  SET(_bundle_id ${BUNDLE-SYMBOLICNAME})
  
  FOREACH(_dep ${_plugin_dependencies})
    STRING(REPLACE . _ _dep_target ${_dep})
    SET(${_explicit_libs} ${${_explicit_libs}} ${_dep_target})
  ENDFOREACH()

  SET(_plugin_old_dependencies "")
  SET(_dep_error 0)
  WHILE(NOT "${_plugin_dependencies}" STREQUAL "${_plugin_old_dependencies}")
    SET(_plugin_old_dependencies ${_plugin_dependencies})
    
    FOREACH(_dep ${_plugin_dependencies})

        SET(_plugin_src_dir ${${_dep}_SRC_DIR})
        
        IF(NOT _plugin_src_dir)
          MESSAGE(SEND_ERROR "Plug-in dependency \"${_dep}\" does not seem to exist.")
          SET(_dep_error 1)
          BREAK()
        ENDIF()

       #_MACRO_REQUIRED_BUNDLES_LIST(_plugin_dependencies "${_plugin_src_dir}/META-INF/MANIFEST.MF")
       INCLUDE(${${_dep}_BIN_DIR}/RequireBundle.cmake)
       LIST(APPEND _plugin_dependencies ${REQUIRE-BUNDLE})

    ENDFOREACH()
    
    LIST(REMOVE_DUPLICATES _plugin_dependencies)
    LIST(SORT _plugin_dependencies)
    
  ENDWHILE()

  IF(NOT _dep_error)
      # iterator over all dependencies
      SET(_plugins_turned_off )
      SET(PLUGIN_LINK_LIBRARIES )
      FOREACH(_dep ${_plugin_dependencies})
    
        # collect the ids of plugins we will have to build
        IF(NOT _BUILD_${_dep})
          LIST(APPEND _plugins_turned_off ${_dep})
        ENDIF()
    
        # set include and link directories
        SET(_dep_src_dir ${${_dep}_SRC_DIR})
        SET(_dep_out_dir ${${_dep}_OUT_DIR})
    SET(_dep_bin_dir ${${_dep}_BIN_DIR})
        
        LINK_DIRECTORIES(${_dep_out_dir}/bin)
        INCLUDE_DIRECTORIES(${_dep_src_dir}/src)
    INCLUDE_DIRECTORIES(${_dep_bin_dir})
    
        IF(EXISTS ${_dep_src_dir}/includes.cmake)
          INCLUDE(${_dep_src_dir}/includes.cmake)
          FOREACH(_added_incldir ${ADDITIONAL_INCLUDE_DIRECTORIES})
            INCLUDE_DIRECTORIES(${_dep_src_dir}/${_added_incldir})
          ENDFOREACH(_added_incldir ${ADDITIONAL_INCLUDE_DIRECTORIES})
        ENDIF(EXISTS ${_dep_src_dir}/includes.cmake)
    
        STRING(REPLACE . _ _dep_target ${_dep})
        LIST(APPEND PLUGIN_LINK_LIBRARIES ${_dep_target})
    
      ENDFOREACH(_dep ${_plugin_dependencies})
      
      IF(_plugins_turned_off)
        MESSAGE(SEND_ERROR "Unmet dependencies: The plugin ${_bundle_id} depends on the following plugins:\n${_plugins_turned_off}.\nSwitch them on in order to build ${_FIND_DEPS_PLUGIN_NAME}.")
      ENDIF(_plugins_turned_off)
  ENDIF()

ENDMACRO()

MACRO(_MACRO_ENABLE_QT4_PLUGINS)

  SET(_enable_qt_plugin ${ARGV0})
  STRING(REGEX MATCH ".*\\.qt.*" _qt_matched ${_enable_qt_plugin})
  IF(_qt_matched)
    IF(QT4_FOUND)
      SET(ENABLE_PLUGIN 1)
    ELSE()
      SET(ENABLE_PLUGIN 0)
    ENDIF()
  ENDIF()

ENDMACRO()

MACRO(_MACRO_CREATE_ENABLE_PLUGIN_CODE)
    SET(_enable_plugins_file "${CMAKE_CURRENT_BINARY_DIR}/berryEnablePlugin.cmake")
    SET(_enable_plugins_filecontent "SET(_enable_bundle 1)")
    FOREACH(_macro_name ${ARGN})
      SET(_enable_plugins_filecontent "${_enable_plugins_filecontent}
      IF(_enable_bundle)
        SET(ENABLE_PLUGIN 1)
        ${_macro_name}(\${BUNDLE-SYMBOLICNAME})
        IF(NOT ENABLE_PLUGIN)
          SET(_enable_bundle 0)
        ENDIF()
      ENDIF()")
    ENDFOREACH()
    CONFIGURE_FILE("${BLUEBERRY_SOURCE_DIR}/CMake/berryEnablePlugin.cmake.in" "${_enable_plugins_file}" @ONLY)
ENDMACRO()


