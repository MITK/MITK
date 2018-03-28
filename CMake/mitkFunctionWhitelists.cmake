###############################################################################
#
# mitkFunctionCreateWhitelistPaths
#
#! Creates advanced cache variables for setting the internal and external
#! whitelist directories.
#!
#! USAGE:
#!
#! \code
#! mitkFunctionCreateWhitelistPaths(<prefix>)
#! \endcode
#!
#! The <prefix> parameter specifies the prefix used for the created variables
#! <prefix>_WHITELISTS_INTERNAL_PATH and <prefix>_WHITELISTS_EXTERNAL_PATH.
#!
#! Default values:
#!  <prefix>_WHITELISTS_INTERNAL_PATH = <prefix>_SOURCE_DIR/CMake/Whitelists
#!  <prefix>_WHITELISTS_EXTERNAL_PATH = %HOME%/.mitk/Whitelists
#!
#! List of variables available after the function is called:
#! - <prefix>_WHITELISTS_INTERNAL_PATH
#! - <prefix>_WHITELISTS_EXTERNAL_PATH
#!
#! Parameters:
#! \param <prefix> The prefix of the created cache variables.
#
###############################################################################

function(mitkFunctionCreateWhitelistPaths)
  set(${ARGV0}_WHITELISTS_INTERNAL_PATH "${${ARGV0}_SOURCE_DIR}/CMake/Whitelists" CACHE PATH "")
  set(${ARGV0}_WHITELISTS_EXTERNAL_PATH ".mitk/Whitelists")

  if(WIN32)
    set(${ARGV0}_WHITELISTS_EXTERNAL_PATH "$ENV{HOMEDRIVE}$ENV{HOMEPATH}/${${ARGV0}_WHITELISTS_EXTERNAL_PATH}")
  else()
    set(${ARGV0}_WHITELISTS_EXTERNAL_PATH "$ENV{HOME}/${${ARGV0}_WHITELISTS_EXTERNAL_PATH}")
  endif()

  FILE(TO_CMAKE_PATH "${${ARGV0}_WHITELISTS_EXTERNAL_PATH}" ${ARGV0}_WHITELISTS_EXTERNAL_PATH)

  set(${ARGV0}_WHITELISTS_EXTERNAL_PATH "${${ARGV0}_WHITELISTS_EXTERNAL_PATH}" CACHE PATH "")

  mark_as_advanced(
    ${ARGV0}_WHITELISTS_INTERNAL_PATH
    ${ARGV0}_WHITELISTS_EXTERNAL_PATH
  )
endfunction()

###############################################################################
#
# mitkFunctionFindWhitelists
#
#! Adds all whitelists found in specfied whitelist paths to the advanced cache
#! variable <prefix>_WHITELIST as enumeration entries.
#!
#! USAGE:
#!
#! \code
#! mitkFunctionFindWhitelists(<prefix>)
#! \endcode
#!
#! The <prefix> parameter specifies the prefix used for the created
#! cache variable <prefix>_WHITELIST. Its default value is "None".
#! The function mitkFunctionCreateWhitelistPaths must be called
#! with the same <prefix> prior to this function.
#!
#! Whitelists are *.cmake files which set the two list variables
#! enabled_modules and enabled_plugins.
#!
#! List of variables available after the function is called:
#! - <prefix>_WHITELIST
#!
#! \sa mitkFunctionCreateWhitelistPaths
#!
#! Parameters:
#! \param <prefix> The prefix of the created cache variable.
#
###############################################################################

function(mitkFunctionFindWhitelists)
  set(whitelists "None")

  file(GLOB internalWhitelistFiles "${${ARGV0}_WHITELISTS_INTERNAL_PATH}/*.cmake")

  foreach(whitelistFile ${internalWhitelistFiles})
    get_filename_component(whitelistFile "${whitelistFile}" NAME_WE)
    list(APPEND whitelists "${whitelistFile}")
  endforeach()

  file(GLOB externalWhitelistFiles "${${ARGV0}_WHITELISTS_EXTERNAL_PATH}/*.cmake")

  foreach(whitelistFile ${externalWhitelistFiles})
    get_filename_component(whitelistFile "${whitelistFile}" NAME_WE)
    list(APPEND whitelists "${whitelistFile} (external)")
  endforeach()

  set(${ARGV0}_WHITELIST "None" CACHE STRING "")
  set_property(CACHE ${ARGV0}_WHITELIST PROPERTY STRINGS ${whitelists})

  mark_as_advanced(${ARGV0}_WHITELIST)
endfunction()

###############################################################################
#
# mitkFunctionWhitelistModules
#
#! Only enables modules which are present in the currently set whitelist or
#! all modules if no whitelist is specified at all.
#!
#! USAGE:
#!
#! \code
#! set(<modules>
#!   ModuleDir
#!   AnotherModuleDir
#!   ...
#! )
#! mitkFunctionWhitelistModules(<prefix> <modules>)
#! \endcode
#!
#! The <prefix> parameter specifies the prefix used to get the
#! currently set whitelist from <prefix>_WHITELIST. Both functions
#! mitkFunctionCreateWhitelistPaths and mitkFunctionFindWhitelists
#! must be called with the same <prefix> prior to this function.
#! The <modules> list must contain the module directory names instead
#! of the module names itself, as the entries are used in
#! add_directory calls.
#!
#! \sa mitkFunctionCreateWhitelistPaths
#! \sa mitkFunctionFindWhitelists
#!
#! Parameters:
#! \param <prefix> The prefix of the white list cache variable.
#! \param <modules> The module directory list variable.
#
###############################################################################

function(mitkFunctionWhitelistModules)
  if(${ARGV0}_WHITELIST STREQUAL "None")
    foreach(module ${${ARGV1}})
      add_subdirectory(Modules/${module})
    endforeach()
  else()
    string(FIND "${${ARGV0}_WHITELIST}" " (external)" index REVERSE)

    if(${index} EQUAL -1)
      set(whitelistFile "${${ARGV0}_WHITELISTS_INTERNAL_PATH}/${${ARGV0}_WHITELIST}.cmake")
    else()
      string(SUBSTRING "${${ARGV0}_WHITELIST}" 0 ${index} whitelistFile)
      set(whitelistFile "${${ARGV0}_WHITELISTS_EXTERNAL_PATH}/${whitelistFile}.cmake")
    endif()

    include(${whitelistFile})

    if(NOT DEFINED enabled_modules)
      message(FATAL_ERROR "Variable 'enabled_modules' not set in whitelist file '${whitelistFile}'!")
    endif()

    foreach(module ${${ARGV1}})
      list(FIND enabled_modules ${module} index)
      if(NOT index EQUAL -1)
        add_subdirectory(Modules/${module})
      endif()
    endforeach()
  endif()
endfunction()

###############################################################################
#
# mitkFunctionWhitelistPlugins
#
#! Only enables plugins which are present in the currently set whitelist or
#! all plugins if no whitelist is specified at all.
#!
#! USAGE:
#!
#! \code
#! set(<plugins>
#!   org.example.plugin:OFF
#!   org.example.another.plugin:ON
#!   ...
#! )
#! mitkFunctionWhitelistPlugins(<prefix> <plugins>)
#! \endcode
#!
#! The <prefix> parameter specifies the prefix used to get the
#! currently set whitelist from <prefix>_WHITELIST. Both functions
#! mitkFunctionCreateWhitelistPaths and mitkFunctionFindWhitelists
#! must be called with the same <prefix> prior to this function.
#! The <plugins> list must contain the plugin names. This function
#! removes plugins not found in the currently set whitelist from
#! the <plugins> variable. Note that plugins which are OFF by
#! default are not switched on.
#!
#! \sa mitkFunctionCreateWhitelistPaths
#! \sa mitkFunctionFindWhitelists
#!
#! Parameters:
#! \param <prefix> The prefix of the white list cache variable.
#! \param <plugins> The plugin list variable to be modified.
#
###############################################################################

function(mitkFunctionWhitelistPlugins)
  if(${ARGV0}_WHITELIST STREQUAL "None")
    return()
  endif()

  string(FIND "${${ARGV0}_WHITELIST}" " (external)" index REVERSE)

  if(${index} EQUAL -1)
    set(whitelistFile "${${ARGV0}_WHITELISTS_INTERNAL_PATH}/${${ARGV0}_WHITELIST}.cmake")
  else()
    string(SUBSTRING "${${ARGV0}_WHITELIST}" 0 ${index} whitelistFile)
    set(whitelistFile "${${ARGV0}_WHITELISTS_EXTERNAL_PATH}/${whitelistFile}.cmake")
  endif()

  include(${whitelistFile})

  if(NOT DEFINED enabled_plugins)
    message(FATAL_ERROR "Variable 'enabled_plugins' not set in whitelist file '${whitelistFile}'!")
  endif()

  set(plugins "")

  foreach(plugin ${${ARGV1}})
    string(FIND ${plugin} ":" index REVERSE)

    if (NOT index EQUAL -1)
      string(SUBSTRING ${plugin} 0 ${index} _plugin)
    else()
      set(_plugin ${plugin})
    endif()

    list(FIND enabled_plugins ${_plugin} index)

    if(NOT index EQUAL -1)
      list(APPEND plugins ${plugin})
    endif()
  endforeach()

  set(${ARGV1} ${plugins} PARENT_SCOPE)
endfunction()
