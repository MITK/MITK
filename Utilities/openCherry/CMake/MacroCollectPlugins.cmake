# Recursively searches for plugins (i.e. directories containing a
# META-INF/MANIFEST.MF file). The macro adds the found directories
# to cmake and stores the plugin-name and absolute path in an
# internal cache variable. 
#
# MACRO_COLLECT_PLUGINS(OUTPUT_DIR plugin_output_dir
#                       [ADD_DIR on|off]
#                       [FORCE_BUILD_ALL]
#                       )
#
# plugin_output_dir is the common binary output directory for all 
# plugins found. It is available as PLUGINS_OUTPUT_BASE_DIR
# and used by the MACRO_CREATE_PLUGIN macro.
#
# ADD_DIR boolean value determining if the plugin_output_dir
# should be added to the OPENCHERRY_PLUGIN_BINARY_DIRS variable. 
# Default is on.
#
# FORCE_BUILD_ALL if set, the BUILD_pluginname variables are ignored and all
# plugins under this directory are build
#
# The variable PLUGINS_SOURCE_BASE_DIR is available after this macro completes
#
MACRO(MACRO_COLLECT_PLUGINS)

MACRO_PARSE_ARGUMENTS(_COLLECT "OUTPUT_DIR;ADD_DIR" "FORCE_BUILD_ALL" ${ARGN})

IF(NOT _COLLECT_ADD_DIR)
  SET(_COLLECT_ADD_DIR 1)
ENDIF(NOT _COLLECT_ADD_DIR)
 
SET(PLUGINS_OUTPUT_BASE_DIR ${_COLLECT_OUTPUT_DIR})

SET(CMAKE_DEBUG_POSTFIX d)

 
#MESSAGE(SEND_ERROR "_COLLECT_OUTPUT_DIR: ${_COLLECT_OUTPUT_DIR}")
#MESSAGE(SEND_ERROR "_COLLECT_ADD_DIR: ${_COLLECT_ADD_DIR}")
#MESSAGE(SEND_ERROR "_COLLECT_FORCE_BUILD_ALL: ${_COLLECT_FORCE_BUILD_ALL}")

SET(PLUGINS_SOURCE_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

IF(_COLLECT_ADD_DIR)
  
  SET(OPENCHERRY_PLUGIN_SOURCE_DIRS ${OPENCHERRY_PLUGIN_SOURCE_DIRS} ${PLUGINS_SOURCE_BASE_DIR}
      CACHE INTERNAL "List of base plugin source directories" FORCE)
  SET(OPENCHERRY_PLUGIN_BINARY_DIRS ${OPENCHERRY_PLUGIN_BINARY_DIRS} ${_COLLECT_OUTPUT_DIR}
      CACHE INTERNAL "List of base plugin binary directories" FORCE)
  
  #MESSAGE("New OPENCHERRY_PLUGIN_BINARY_DIRS is: ${OPENCHERRY_PLUGIN_BINARY_DIRS}")
  
ENDIF(_COLLECT_ADD_DIR)

SET(_plugins_to_build )
FILE(GLOB all_dirs *)
FOREACH(dir_entry ${all_dirs})
  IF(EXISTS ${dir_entry}/META-INF/MANIFEST.MF)
    #SET(plugin_dirs ${plugin_dirs} ${dir_entry})
    MACRO_PARSE_MANIFEST(${dir_entry}/META-INF/MANIFEST.MF)
    IF(BUNDLE-SYMBOLICNAME)
      SET(${BUNDLE-SYMBOLICNAME}_SRC_DIR ${dir_entry})
      SET(${BUNDLE-SYMBOLICNAME}_BIN_DIR ${_COLLECT_OUTPUT_DIR}/${BUNDLE-SYMBOLICNAME})
      # write the variable in .cmake file, so external projects have access to them
      SET(OPENCHERRY_BUNDLE_VARIABLES "${OPENCHERRY_BUNDLE_VARIABLES}
      SET(${BUNDLE-SYMBOLICNAME}_SRC_DIR ${dir_entry})
      SET(${BUNDLE-SYMBOLICNAME}_BIN_DIR ${_COLLECT_OUTPUT_DIR}/${BUNDLE-SYMBOLICNAME})")
      
      OPTION("BUILD_${BUNDLE-SYMBOLICNAME}" "Build ${BUNDLE-SYMBOLICNAME} Plugin" ON)
      IF(BUILD_${BUNDLE-SYMBOLICNAME} OR _COLLECT_FORCE_BUILD_ALL)
        LIST(APPEND _plugins_to_build ${dir_entry})
      ENDIF(BUILD_${BUNDLE-SYMBOLICNAME} OR _COLLECT_FORCE_BUILD_ALL)
      
    ENDIF(BUNDLE-SYMBOLICNAME)
  ENDIF(EXISTS ${dir_entry}/META-INF/MANIFEST.MF)
ENDFOREACH(dir_entry ${all_dirs})

FOREACH(_subdir ${_plugins_to_build})
  ADD_SUBDIRECTORY(${_subdir})
ENDFOREACH(_subdir ${_plugins_to_build})

IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/BundleList.cmake.in)
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/BundleList.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/BundleList.cmake @ONLY)
ENDIF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/BundleList.cmake.in)

INCLUDE_DIRECTORIES(${Poco_INCLUDE_DIRS})
LINK_DIRECTORIES(${Poco_LIBRARY_DIR})

#FOREACH(plugin_dir ${plugin_dirs}) 
#  MESSAGE(STATUS "Processing ${plugin_dir}")
  
  
#  _MACRO_CREATE_PLUGIN_NAME(PLUGIN_NAME 
#                            INPUT ${plugin_dir}
#                            BASEDIR ${PLUGINS_SOURCE_BASE_DIR} )
  
#  #MESSAGE(STATUS "PLUGIN_NAME = ${PLUGIN_NAME}")
#  OPTION("BUILD_${PLUGIN_NAME}" "Build ${PLUGIN_NAME} Plugin" ON)
#  IF(BUILD_${PLUGIN_NAME} OR _COLLECT_FORCE_BUILD_ALL)
#    ADD_SUBDIRECTORY(${plugin_dir})
#  ENDIF(BUILD_${PLUGIN_NAME} OR _COLLECT_FORCE_BUILD_ALL)
#ENDFOREACH(plugin_dir)

ENDMACRO(MACRO_COLLECT_PLUGINS)