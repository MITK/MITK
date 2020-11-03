#!
#! Configure user file for visual studio projects
#!
#! \brief This function will configure a user file for visual studio projects
#!
#! \param NAME (required) Name of the executable
#!
#! \code
#! mitkFunctionConfigureVisualStudioUserProjectFile(
#!   NAME MyApp
#! )
#! \endcode
#!

function(mitkFunctionConfigureVisualStudioUserProjectFile)

  set(_function_params
      NAME                   # Name of the executable
     )

  set(_function_multiparams
     )

  set(_function_options

     )

  cmake_parse_arguments(VSUPF "${_function_options}" "${_function_params}" "${_function_multiparams}" ${ARGN})

  SET(VS_BUILD_TYPE_DEB debug)
  SET(VS_BUILD_TYPE_REL release)
  SET(VS_BUILD_TYPE_RELDEB relwithdebinfo)

  mitkFunctionGetLibrarySearchPaths(MITK_RUNTIME_PATH_DEB ${VS_BUILD_TYPE_DEB} DEBUG)
  mitkFunctionGetLibrarySearchPaths(MITK_RUNTIME_PATH_REL ${VS_BUILD_TYPE_REL} RELEASE)
  mitkFunctionGetLibrarySearchPaths(MITK_RUNTIME_PATH_RELWITHDEBINFO ${VS_BUILD_TYPE_RELDEB} RELWITHDEBINFO)
  CONFIGURE_FILE("${MITK_SOURCE_DIR}/CMake/MITK.vcxproj.user.in" ${CMAKE_CURRENT_BINARY_DIR}/${VSUPF_NAME}.vcxproj.user @ONLY)

endfunction()
