function(mitkFunctionCreateWhitelistPaths)
  set(${ARGV0}_WHITELISTS_INTERNAL_PATH "${${ARGV0}_SOURCE_DIR}/CMake/Whitelists" CACHE PATH "")
  set(${ARGV0}_WHITELISTS_EXTERNAL_PATH ".MITK/Whitelists")

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



function(mitkFunctionFindWhitelists)
  set(whitelists "None")

  file(GLOB internalWhitelistFiles "${MITK_WHITELISTS_INTERNAL_PATH}/*.cmake")

  foreach(whitelistFile "${internalWhitelistFiles}")
    get_filename_component(whitelistFile "${whitelistFile}" NAME_WE)
    list(APPEND whitelists "${whitelistFile}")
  endforeach()

  file(GLOB externalWhitelistFiles "${MITK_WHITELISTS_EXTERNAL_PATH}/*.cmake")

  foreach(whitelistFile "${externalWhitelistFiles}")
    get_filename_component(whitelistFile "${whitelistFile}" NAME_WE)
    list(APPEND whitelists "${whitelistFile} (external)")
  endforeach()

  set(${ARGV0}_WHITELIST "None" CACHE STRING "")
  set_property(CACHE ${ARGV0}_WHITELIST PROPERTY STRINGS ${whitelists})

  mark_as_advanced(${ARGV0}_WHITELIST)
endfunction()



function(mitkFunctionWhitelistModules)
  if(${ARGV0}_WHITELIST STREQUAL "None")
    foreach(module ${${ARGV1}})
      add_subdirectory(${module})
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
        add_subdirectory(${module})
      endif()
    endforeach()
  endif()
endfunction()



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

