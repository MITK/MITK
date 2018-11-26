#!
#! \brief Add a custom test for MITK module
#!
#! \param test_name Unique identifier for the test
#! \param test_function Name of the test function (the one with the argc,argv signature)
#!
#! Additional parameters will be passed as command line parameters to the test.
#!
function(mitkAddCustomModuleTest test_name test_function)

  if (BUILD_TESTING AND MODULE_IS_ENABLED)
    add_test(${test_name} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTDRIVER} ${test_function} ${ARGN})
    set_property(TEST ${test_name} PROPERTY LABELS ${MODULE_SUBPROJECTS} MITK)
    mitkFunctionGetLibrarySearchPaths(MITK_RUNTIME_PATH_RELEASE release RELEASE)
    mitkFunctionGetLibrarySearchPaths(MITK_RUNTIME_PATH_DEBUG debug DEBUG)
    string (REGEX REPLACE "\;" "\\\;" MITK_RUNTIME_PATH_RELEASE "${MITK_RUNTIME_PATH_RELEASE}")
    string (REGEX REPLACE "\;" "\\\;" MITK_RUNTIME_PATH_DEBUG "${MITK_RUNTIME_PATH_DEBUG}")
    set_property(TEST ${test_name} PROPERTY ENVIRONMENT "PATH=${MITK_RUNTIME_PATH_RELEASE}\;${MITK_RUNTIME_PATH_DEBUG}" APPEND)
    set_property(TEST ${test_name} PROPERTY SKIP_RETURN_CODE 77)
  endif()

endfunction()
