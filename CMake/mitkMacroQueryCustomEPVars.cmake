#[[ Iterate over all MITK extension directories and collect custom external
    project variables for a specific external project (${proj}). ]]

macro(mitk_query_custom_ep_vars)
  set(${proj}_CUSTOM_CMAKE_ARGS "")
  set(${proj}_CUSTOM_CMAKE_CACHE_ARGS "")
  set(${proj}_CUSTOM_CMAKE_CACHE_DEFAULT_ARGS "")

  foreach(MITK_EXTENSION_DIR ${MITK_ABSOLUTE_EXTENSION_DIRS})
    set(MITK_CMAKE_EXTERNALS_CUSTOM_DIR "${MITK_EXTENSION_DIR}/CMakeExternals/Customization")
    if(EXISTS "${MITK_CMAKE_EXTERNALS_CUSTOM_DIR}/${proj}.cmake")
      include("${MITK_CMAKE_EXTERNALS_CUSTOM_DIR}/${proj}.cmake")
      list(APPEND ${proj}_CUSTOM_CMAKE_ARGS ${CUSTOM_CMAKE_ARGS})
      list(APPEND ${proj}_CUSTOM_CMAKE_CACHE_ARGS ${CUSTOM_CMAKE_CACHE_ARGS})
      list(APPEND ${proj}_CUSTOM_CMAKE_CACHE_DEFAULT_ARGS ${CUSTOM_CMAKE_CACHE_DEFAULT_ARGS})
    endif()
  endforeach()

  list(REMOVE_DUPLICATES ${proj}_CUSTOM_CMAKE_ARGS)
  list(REMOVE_DUPLICATES ${proj}_CUSTOM_CMAKE_CACHE_ARGS)
  list(REMOVE_DUPLICATES ${proj}_CUSTOM_CMAKE_CACHE_DEFAULT_ARGS)

  if(${proj}_CUSTOM_CMAKE_ARGS)
    message(STATUS "Custom ${proj} CMake args: ${${proj}_CUSTOM_CMAKE_ARGS}")
  endif()

  if(${proj}_CUSTOM_CMAKE_CACHE_ARGS)
    message(STATUS "Custom ${proj} CMake cache args: ${${proj}_CUSTOM_CMAKE_CACHE_ARGS}")
  endif()

  if(${proj}_CUSTOM_CMAKE_CACHE_DEFAULT_ARGS)
    message(STATUS "Custom ${proj} CMake cache default args: ${${proj}_CUSTOM_CMAKE_CACHE_DEFAULT_ARGS}")
  endif()
endmacro()
