function(mitkFunctionEnablePlugins)
  if(MITK_WHITELIST STREQUAL "None")
    return()
  endif()

  string(FIND "${MITK_WHITELIST}" " (external)" index REVERSE)

  if(${index} EQUAL -1)
    set(whitelistFile "${MITK_WHITELISTS_PATH}/${MITK_WHITELIST}.cmake")
  else()
    string(SUBSTRING "${MITK_WHITELIST}" 0 ${index} whitelistFile)
    set(whitelistFile "${MITK_EXTERNAL_WHITELISTS_PATH}/${whitelistFile}.cmake")
  endif()

  include(${whitelistFile})

  if(NOT DEFINED enabled_plugins)
    message(FATAL_ERROR "Variable 'enabled_plugins' not set in whitelist file '${whitelistFile}'!")
  endif()

  set(plugins "")

  foreach(plugin ${${ARGV0}})
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

  set(${ARGV0} ${plugins} PARENT_SCOPE)
endfunction()
