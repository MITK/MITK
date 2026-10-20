#! Add Boost libraries that an enabled feature requires to the cache variable
#! MITK_USE_Boost_<kind>_LIBRARIES. Usage:
#!
#!   mitkFunctionRequireBoostLibraries(HEADER|COMPILED feature lib1 [lib2 ...])
#!
#! Each library missing from the cache variable is appended to it and reported
#! with the feature as the reason. Libraries are never removed, so disabling
#! the feature later leaves them in the cache variable.
#!

function(mitkFunctionRequireBoostLibraries kind feature)
  if(NOT kind MATCHES "^(HEADER|COMPILED)$")
    message(FATAL_ERROR "Unknown kind of Boost libraries: \"${kind}\" (expected HEADER or COMPILED)")
  endif()

  set(_var MITK_USE_Boost_${kind}_LIBRARIES)
  set(_libraries ${${_var}})
  set(_added FALSE)

  foreach(_lib IN LISTS ARGN)
    if(NOT _lib IN_LIST _libraries)
      message("> Adding ${_lib} to ${_var} because of ${feature}")
      list(APPEND _libraries ${_lib})
      set(_added TRUE)
    endif()
  endforeach()

  if(_added)
    get_property(_doc CACHE ${_var} PROPERTY HELPSTRING)
    set(${_var} "${_libraries}" CACHE STRING "${_doc}" FORCE)
  endif()
endfunction()
