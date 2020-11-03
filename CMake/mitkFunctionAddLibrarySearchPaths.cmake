#! Helper function that stores additional library search paths in order to use them
#! in the mitkFunctionGetLibrarySearchPaths function.
#!
#! Calling this function registers an external project with the MITK
#! build system. Usage:
#!
#!   mitkFunctionAddLibrarySearchPaths(NAME name
#!                                  PATHS path1 [path2 ...]
#!                                  [DEBUG_PATHS path1 [path2 ...]]
#!                                  [MINSIZEREL_PATHS path1 [path2 ...]]
#!                                  [RELWITHDEBINFO_PATHS path1 [path2 ...]]
#!   )
#!
#! The function adds the passed paths to a file stored in
#! ${MITK_SUPERBUILD_BINARY_DIR}/MITK-AdditionalLibPaths.
#! This information will be later on used by mitkFunctionGetLibrarySearchPaths.
#! This allows external projects (also of extensions) to define library search
#! paths that will be used in the superbuild. By using DEBUG_PATHS, MINSIZEREL_PATHS
#! and RELWITHDEBINFO_PATHS, you can save special paths for debug, min size release
#! or "release with debug info". If special paths are not explicitly specified,
#! PATHS will be used for all configuration.
#!

function(mitkFunctionAddLibrarySearchPaths)
  cmake_parse_arguments(LSP "" "NAME" "PATHS;DEBUG_PATHS;MINSIZEREL_PATHS;RELWITHDEBINFO_PATHS" ${ARGN})

  if(NOT LSP_NAME)
    message(SEND_ERROR "The NAME argument is missing.")
  endif()
  
  if(NOT DEFINED LSP_DEBUG_PATHS)
    set(LSP_DEBUG_PATHS ${LSP_PATHS})
  endif()

  if(NOT DEFINED LSP_MINSIZEREL_PATHS)
    set(LSP_MINSIZEREL_PATHS ${LSP_PATHS})
  endif()

  if(NOT DEFINED LSP_RELWITHDEBINFO_PATHS)
    set(LSP_RELWITHDEBINFO_PATHS ${LSP_PATHS})
  endif()
  
  set(_configured_file ${MITK_SUPERBUILD_BINARY_DIR}/MITK-AdditionalLibPaths/${LSP_NAME}.cmake)
  if(${CMAKE_PROJECT_NAME} STREQUAL "MITK-superbuild")
    set(_configured_file ${MITK_BINARY_DIR}/MITK-AdditionalLibPaths/${LSP_NAME}.cmake)
  endif(${CMAKE_PROJECT_NAME} STREQUAL "MITK-superbuild")

  configure_file(${MITK_CMAKE_DIR}/AdditionalLibrarySearchPath.cmake.in ${_configured_file} @ONLY)  
  
endfunction()
