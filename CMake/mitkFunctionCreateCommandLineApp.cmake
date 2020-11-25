#!
#! Create a Command Line App.
#!
#! \brief This function will create a command line executable and the scripts required to run it
#!
#! \param NAME (required) Name of the command line app
#! \param DEPENDS (optional) Required MITK modules beyond MitkCommandLine
#! \param PACKAGE_DEPENDS (optional) list of "packages" this command line app depends on (e.g. ITK, VTK, etc.)
#! \param TARGET_DEPENDS (optional) list of additional CMake targets this command line app depends on
#! \param CPP_FILES (optional) list of cpp files, if it is not given NAME.cpp is assumed
#!
#! Assuming that there exists a file called <code>MyApp.cpp</code>, an example call looks like:
#! \code
#! mitkFunctionCreateCommandLineApp(
#!   NAME MyApp
#!   DEPENDS MitkCore MitkPlanarFigure
#!   PACKAGE_DEPENDS ITK VTK
#! )
#! \endcode
#!

function(mitkFunctionCreateCommandLineApp)

  set(_function_params
      NAME                # Name of the command line app
     )

  set(_function_multiparams
      DEPENDS                # list of modules this command line app depends on
      PACKAGE_DEPENDS        # list of "packages" this command line app depends on (e.g. ITK, VTK, etc.)
      TARGET_DEPENDS         # list of additional CMake targets this command line app depends on
      CPP_FILES              # (optional) list of cpp files, if it is not given NAME.cpp is assumed
     )

  set(_function_options
       WARNINGS_NO_ERRORS
     )

  cmake_parse_arguments(CMDAPP "${_function_options}" "${_function_params}" "${_function_multiparams}" ${ARGN})

  if(NOT CMDAPP_NAME)
    message(FATAL_ERROR "NAME argument cannot be empty.")
  endif()

  if(NOT CMDAPP_CPP_FILES)
    set(CMDAPP_CPP_FILES ${CMDAPP_NAME}.cpp)
  endif()
  if(CMDAPP_WARNINGS_NO_ERRORS)
    LIST(APPEND _CMDAPP_OPTIONS WARNINGS_NO_ERRORS)
  endif()
  mitk_create_executable(${CMDAPP_NAME}
  DEPENDS MitkCommandLine ${CMDAPP_DEPENDS}
  PACKAGE_DEPENDS ${CMDAPP_PACKAGE_DEPENDS}
  TARGET_DEPENDS ${TARGET_DEPENDS}
  CPP_FILES ${CMDAPP_CPP_FILES}
  ${_CMDAPP_OPTIONS}
  )
endfunction()
