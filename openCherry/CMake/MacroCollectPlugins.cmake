# Recursively searches for plugins (i.e. directories containing a
# META-INF/MANIFEST.MF file). The macro adds the found directories
# to cmake and writes information about the configured plug-ins in a file
#
# MACRO_COLLECT_PLUGINS(OUTPUT_DIR plugin_output_dir
#                       [CACHE_PLUGIN_SOURCE_DIRS cache_src_dirs]
#                       [CACHE_PLUGIN_OUTPUT_DIRS cache_bin_dirs]
#                       [CACHE_PLUGIN_TARGETS cache_plugin_targets]
#                       [BUNDLE_LIST_PATH bundle_list_path]
#                       [CMAKE_CACHE_PREFIX cache_prefix]
#                       [ENABLE_PLUGIN_MACROS macro...]
#                       [BUNDLE_DEFAULT_ON id...]
#                       [DEFAULT_BUILD_ON]
#                       [FORCE_BUILD_ALL]
#                       )
#
# plugin_output_dir is the common binary output directory for all 
# plugins found. It is available as PLUGINS_OUTPUT_BASE_DIR
# and used by the MACRO_CREATE_PLUGIN macro.
#
# CACHE_PLUGIN_SOURCE_DIRS and CACHE_PLUGIN_OUTPUT_DIRS
# names of CMake cache variables where the base plugin source
# and output directories will be appended. This is useful if this
# macro is invoked multiple times in different places and you want
# to record the directories. The variables
# can than later be used to configure your applications .ini file.
# Be sure to clear the cache variables before the first invokation
# of this macro.
#
# CACHE_PLUGIN_TARGETS
# <cache_plugin_targets> is used as the name of a cache variable which
# stores a list of enabled plug-ins (their CMake target names)
#
# BUNDLE_LIST_PATH
# The full path for the generated cmake file containing the _BUILD_<plugin-id>
# variables. If not set, "${PROJECT_BINARY_DIR}/${PROJECT_NAME}BundleList.cmake"
# is used.
# 
# CMAKE_CACHE_PREFIX
# <cache_prefix> is prepended to the CMake cache variables
#
# ENABLE_PLUGIN_MACROS
# macro... is a list containing macro names which will be evaluated
# to determine if a specific plug-in should be build. The parameter list of
# the macro is (<bundle-symbolicname>) and the macro must set the variable
# ENABLE_PLUGIN to true or false. For Qt4, a default macro is provided.
#
# BUNDLE_DEFAULT_ON
# id... is a list of bundle symbolic names for which the
# CMake build option should default to ON
#
# DEFAULT_BUILD_ON if set, the generated CMake option for building plug-ins
# defaults to ON. Otherwise, it is set to OFF.
#
# FORCE_BUILD_ALL if set, the BUILD_pluginname variables are ignored and all
# plugins under this directory are build
#
#
# The following variables can be used after the macro is invoked:
#
# PLUGINS_SOURCE_BASE_DIR the directory where this macro has been invoked
# PLUGINS_OUTPUT_BASE_DIR the output directory for the bundles
# ENABLED_PLUGINS_RELATIVE_DIRS a list of bundle directories relative to
#    PLUGINS_SOURCE_BASE_DIR which are build
# ENABLED_PLUGINS_ABSOLUTE_DIRS the same as ENABLED_PLUGINS_RELATIVE_DIRS
#    but with absolute paths
#
MACRO(MACRO_COLLECT_PLUGINS)

MACRO_PARSE_ARGUMENTS(_COLLECT "OUTPUT_DIR;CACHE_PLUGIN_SOURCE_DIRS;CACHE_PLUGIN_OUTPUT_DIRS;CACHE_PLUGIN_TARGETS;BUNDLE_LIST_PATH;BUNDLE_DEFAULT_ON;CMAKE_CACHE_PREFIX;ENABLE_PLUGIN_MACROS" "DEFAULT_BUILD_ON;FORCE_BUILD_ALL" ${ARGN})

IF(NOT _COLLECT_ADD_DIR)
  SET(_COLLECT_ADD_DIR 1)
ENDIF(NOT _COLLECT_ADD_DIR)

IF(NOT _COLLECT_BUNDLE_LIST_PATH)
  SET(_COLLECT_BUNDLE_LIST_PATH "${PROJECT_BINARY_DIR}/${PROJECT_NAME}BundleList.cmake")
ENDIF(NOT _COLLECT_BUNDLE_LIST_PATH)

IF(_COLLECT_CMAKE_CACHE_PREFIX)
  SET(_COLLECT_CMAKE_CACHE_PREFIX "${_COLLECT_CMAKE_CACHE_PREFIX}_")
ENDIF(_COLLECT_CMAKE_CACHE_PREFIX)

LIST(APPEND _COLLECT_ENABLE_PLUGIN_MACROS "_MACRO_ENABLE_QT4_PLUGINS")
 
SET(PLUGINS_OUTPUT_BASE_DIR ${_COLLECT_OUTPUT_DIR})

# writes the file ${CMAKE_CURRENT_BINARY_DIR}/cherryEnablePlugin.cmake
_MACRO_CREATE_ENABLE_PLUGIN_CODE(${_COLLECT_ENABLE_PLUGIN_MACROS})
  
SET(PLUGINS_SOURCE_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

IF(_COLLECT_CACHE_PLUGIN_SOURCE_DIRS)  
  SET(${_COLLECT_CACHE_PLUGIN_SOURCE_DIRS} ${${_COLLECT_CACHE_PLUGIN_SOURCE_DIRS}} "${PLUGINS_SOURCE_BASE_DIR}"
      CACHE INTERNAL "List of base plugin source directories" FORCE)
ENDIF(_COLLECT_CACHE_PLUGIN_SOURCE_DIRS)

IF(_COLLECT_CACHE_PLUGIN_OUTPUT_DIRS)  
  SET(${_COLLECT_CACHE_PLUGIN_OUTPUT_DIRS} ${${_COLLECT_CACHE_PLUGIN_OUTPUT_DIRS}} "${_COLLECT_OUTPUT_DIR}"
      CACHE INTERNAL "List of base plugin output directories" FORCE)
ENDIF(_COLLECT_CACHE_PLUGIN_OUTPUT_DIRS)
  
SET(ENABLED_PLUGINS_RELATIVE_DIRS )
SET(ENABLED_PLUGINS_ABSOLUTE_DIRS )
SET(_plugins_target_list )
FILE(GLOB all_dirs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *)
FOREACH(dir_relative_entry ${all_dirs})
  SET(dir_entry "${CMAKE_CURRENT_SOURCE_DIR}/${dir_relative_entry}")
  IF(EXISTS "${dir_entry}/META-INF/MANIFEST.MF")
    MACRO_PARSE_MANIFEST("${dir_entry}/META-INF/MANIFEST.MF")
    IF(BUNDLE-SYMBOLICNAME)

	# include the generated file with the custom macro code for
	# checking if a bundle should be enabled
    INCLUDE("${_enable_plugins_file}")
        
    IF(_enable_bundle)
	  # The bundle is considered valid for the current configuration
	  # i.e. a build option will be displayed and internal variables are set
	  
      SET(${BUNDLE-SYMBOLICNAME}_SRC_DIR "${dir_entry}")
      SET(${BUNDLE-SYMBOLICNAME}_BIN_DIR "${_COLLECT_OUTPUT_DIR}/${BUNDLE-SYMBOLICNAME}")
      # write the variable in .cmake file, so external projects have access to them
      SET(OPENCHERRY_BUNDLE_VARIABLES "${OPENCHERRY_BUNDLE_VARIABLES}
SET(${BUNDLE-SYMBOLICNAME}_SRC_DIR \"${dir_entry}\")
SET(${BUNDLE-SYMBOLICNAME}_BIN_DIR \"${_COLLECT_OUTPUT_DIR}/${BUNDLE-SYMBOLICNAME}\")")
      
	  # compute the default for the build option (ON/OFF)
      SET(_default_bundle_option ${_COLLECT_DEFAULT_BUILD_ON})
      LIST(FIND _COLLECT_BUNDLE_DEFAULT_ON ${BUNDLE-SYMBOLICNAME} _bundle_default_on_found)
      IF(_bundle_default_on_found GREATER -1)
        SET(_default_bundle_option 1)
      ENDIF()
      
      OPTION("${_COLLECT_CMAKE_CACHE_PREFIX}BUILD_${BUNDLE-SYMBOLICNAME}" "Build ${BUNDLE-SYMBOLICNAME} Plugin" ${_default_bundle_option})
      
	  # test if the bundle should be build
	  IF(${_COLLECT_CMAKE_CACHE_PREFIX}BUILD_${BUNDLE-SYMBOLICNAME} OR _COLLECT_FORCE_BUILD_ALL)
        LIST(APPEND ENABLED_PLUGINS_RELATIVE_DIRS "${dir_relative_entry}")
		LIST(APPEND ENABLED_PLUGINS_ABSOLUTE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/${dir_relative_entry}")
        STRING(REPLACE . _ _plugin_target ${BUNDLE-SYMBOLICNAME})
        LIST(APPEND _plugin_target_list ${_plugin_target})
        
		# record that this bundle is being build.
        SET(_BUILD_${BUNDLE-SYMBOLICNAME} 1)
        SET(OPENCHERRY_BUNDLE_VARIABLES "${OPENCHERRY_BUNDLE_VARIABLES}
SET(_BUILD_${BUNDLE-SYMBOLICNAME} 1)")

      ELSE()
	  
	    # the build option for the bundle is off, hence we delete the MANIFEST.MF
		# file in the output directory to prevent the bundle loader from finding
		# the disabled bundle.
        FILE(RELATIVE_PATH _binary_manifest_path "${CMAKE_CURRENT_SOURCE_DIR}" "${dir_entry}/META-INF/MANIFEST.MF")
        SET(_binary_manifest_path "${_COLLECT_OUTPUT_DIR}/${_binary_manifest_path}")
        FILE(REMOVE "${_binary_manifest_path}")
      ENDIF()
      ENDIF()
    ENDIF()
  ENDIF()
ENDFOREACH()

# add Poco directories for all plugins
INCLUDE_DIRECTORIES(${Poco_INCLUDE_DIRS})
LINK_DIRECTORIES(${Poco_LIBRARY_DIR})

FOREACH(_subdir ${ENABLED_PLUGINS_RELATIVE_DIRS})
  ADD_SUBDIRECTORY(${_subdir})
ENDFOREACH(_subdir ${ENABLED_PLUGINS_RELATIVE_DIRS})

IF(_COLLECT_CACHE_PLUGIN_TARGETS)
  SET(${_COLLECT_CACHE_PLUGIN_TARGETS} ${_plugin_target_list} CACHE INTERNAL "A list of enabled plug-ins")
ENDIF(_COLLECT_CACHE_PLUGIN_TARGETS)

IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/BundleList.cmake.in)
  CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/BundleList.cmake.in" "${_COLLECT_BUNDLE_LIST_PATH}" @ONLY)
ENDIF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/BundleList.cmake.in)

ENDMACRO(MACRO_COLLECT_PLUGINS)
