# Recursively searches for plugins (i.e. directories containing a
# META-INF/MANIFEST.MF file). The macro adds the found directories
# to cmake and stores the plugin-name and absolute path in an
# internal cache variable. 
#
# MACRO_COLLECT_PLUGINS(OUTPUT_DIR plugin_output_dir
#                       [CACHE_PLUGIN_SOURCE_DIRS cache_src_dirs]
#                       [CACHE_PLUGIN_BINARY_DIRS cache_bin_dirs]
#                       [BUNDLE_LIST_NAME bundle_list_name]
#                       [FORCE_BUILD_ALL]
#                       )
#
# plugin_output_dir is the common binary output directory for all 
# plugins found. It is available as PLUGINS_OUTPUT_BASE_DIR
# and used by the MACRO_CREATE_PLUGIN macro.
#
# CACHE_PLUGIN_SOURCE_DIRS and CACHE_PLUGIN_BINARY_DIRS
# names of CMake cache variables where the base plugin source
# and binary directories will be appended. These variables
# can late be used to configure your applications .ini file.
#
# BUNDLE_LIST_NAME
# The name of the generated cmake file containing the BUILD_<plugin-id>
# variables. If not set, "BundleList" is used.
 
# FORCE_BUILD_ALL if set, the BUILD_pluginname variables are ignored and all
# plugins under this directory are build
#
# The variable PLUGINS_SOURCE_BASE_DIR is available after this macro completes
#
MACRO(MACRO_COLLECT_PLUGINS)

MACRO_PARSE_ARGUMENTS(_COLLECT "OUTPUT_DIR;CACHE_PLUGIN_SOURCE_DIRS;CACHE_PLUGIN_BINARY_DIRS;BUNDLE_LIST_NAME" "FORCE_BUILD_ALL" ${ARGN})

IF(NOT _COLLECT_ADD_DIR)
  SET(_COLLECT_ADD_DIR 1)
ENDIF(NOT _COLLECT_ADD_DIR)

IF(NOT _COLLECT_BUNDLE_LIST_NAME)
  SET(_COLLECT_BUNDLE_LIST_NAME "BundleList")
ENDIF(NOT _COLLECT_BUNDLE_LIST_NAME)
 
SET(PLUGINS_OUTPUT_BASE_DIR ${_COLLECT_OUTPUT_DIR})

SET(CMAKE_DEBUG_POSTFIX ${OPENCHERRY_DEBUG_POSTFIX})

 
#MESSAGE(SEND_ERROR "_COLLECT_OUTPUT_DIR: ${_COLLECT_OUTPUT_DIR}")
#MESSAGE(SEND_ERROR "_COLLECT_ADD_DIR: ${_COLLECT_ADD_DIR}")
#MESSAGE(SEND_ERROR "_COLLECT_FORCE_BUILD_ALL: ${_COLLECT_FORCE_BUILD_ALL}")

SET(PLUGINS_SOURCE_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

IF(_COLLECT_CACHE_PLUGIN_SOURCE_DIRS)  
  SET(${_COLLECT_CACHE_PLUGIN_SOURCE_DIRS} ${${_COLLECT_CACHE_PLUGIN_SOURCE_DIRS}} "${PLUGINS_SOURCE_BASE_DIR}"
      CACHE INTERNAL "List of base plugin source directories" FORCE)
ENDIF(_COLLECT_CACHE_PLUGIN_SOURCE_DIRS)

IF(_COLLECT_CACHE_PLUGIN_BINARY_DIRS)  
  SET(${_COLLECT_CACHE_PLUGIN_BINARY_DIRS} ${${_COLLECT_CACHE_PLUGIN_BINARY_DIRS}} "${_COLLECT_OUTPUT_DIR}"
      CACHE INTERNAL "List of base plugin source directories" FORCE)
ENDIF(_COLLECT_CACHE_PLUGIN_BINARY_DIRS)
  
SET(_plugins_to_build )
FILE(GLOB all_dirs *)
FOREACH(dir_entry ${all_dirs})
  IF(EXISTS "${dir_entry}/META-INF/MANIFEST.MF")
    #SET(plugin_dirs ${plugin_dirs} ${dir_entry})
    MACRO_PARSE_MANIFEST("${dir_entry}/META-INF/MANIFEST.MF")
    IF(BUNDLE-SYMBOLICNAME)
      SET(${BUNDLE-SYMBOLICNAME}_SRC_DIR "${dir_entry}")
      SET(${BUNDLE-SYMBOLICNAME}_BIN_DIR "${_COLLECT_OUTPUT_DIR}/${BUNDLE-SYMBOLICNAME}")
      # write the variable in .cmake file, so external projects have access to them
      SET(OPENCHERRY_BUNDLE_VARIABLES "${OPENCHERRY_BUNDLE_VARIABLES}
SET(${BUNDLE-SYMBOLICNAME}_SRC_DIR \"${dir_entry}\")
SET(${BUNDLE-SYMBOLICNAME}_BIN_DIR \"${_COLLECT_OUTPUT_DIR}/${BUNDLE-SYMBOLICNAME}\")")
      
      OPTION("BUILD_${BUNDLE-SYMBOLICNAME}" "Build ${BUNDLE-SYMBOLICNAME} Plugin" ON)
      IF(BUILD_${BUNDLE-SYMBOLICNAME} OR _COLLECT_FORCE_BUILD_ALL)
        LIST(APPEND _plugins_to_build "${dir_entry}")
        SET(OPENCHERRY_BUNDLE_VARIABLES "${OPENCHERRY_BUNDLE_VARIABLES}
SET(BUILD_${BUNDLE-SYMBOLICNAME} 1)")
      ENDIF(BUILD_${BUNDLE-SYMBOLICNAME} OR _COLLECT_FORCE_BUILD_ALL)
      
    ENDIF(BUNDLE-SYMBOLICNAME)
  ENDIF(EXISTS "${dir_entry}/META-INF/MANIFEST.MF")
ENDFOREACH(dir_entry ${all_dirs})

# add Poco directories for all plugins
INCLUDE_DIRECTORIES(${Poco_INCLUDE_DIRS})
LINK_DIRECTORIES(${Poco_LIBRARY_DIR})

FOREACH(_subdir ${_plugins_to_build})
  ADD_SUBDIRECTORY(${_subdir})
ENDFOREACH(_subdir ${_plugins_to_build})

IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/BundleList.cmake.in)
  CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/BundleList.cmake.in" "${OPENCHERRY_BINARY_DIR}/${_COLLECT_BUNDLE_LIST_NAME}.cmake" @ONLY)
ENDIF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/BundleList.cmake.in)

ENDMACRO(MACRO_COLLECT_PLUGINS)
