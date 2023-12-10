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

  mitkFunctionGetLibrarySearchPaths(PATH_DEBUG debug DEBUG)
  mitkFunctionGetLibrarySearchPaths(PATH_RELEASE release RELEASE)
  mitkFunctionGetLibrarySearchPaths(PATH_RELWITHDEBINFO relwithdebinfo RELWITHDEBINFO)
  mitkFunctionGetLibrarySearchPaths(PATH_MINSIZEREL minsizerel MINSIZEREL)

  set_target_properties(${VSUPF_NAME} PROPERTIES VS_DEBUGGER_ENVIRONMENT "PATH=\
$<$<CONFIG:Debug>:${PATH_DEBUG}>\
$<$<CONFIG:Release>:${PATH_RELEASE}>\
$<$<CONFIG:RelWithDebInfo>:${PATH_RELWITHDEBINFO}>\
$<$<CONFIG:MinSizeRel>:${PATH_MINSIZEREL}>\
$<SEMICOLON>%PATH%")

endfunction()
