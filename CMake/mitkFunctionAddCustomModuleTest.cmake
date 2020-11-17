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
    if(MITK_XVFB_TESTING)
      set(xvfb_run ${MITK_XVFB_TESTING_COMMAND})
    else()
      set(xvfb_run )
    endif()
    add_test(NAME ${test_name} COMMAND ${xvfb_run} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTDRIVER} ${test_function} ${ARGN})
    mitkFunctionGetLibrarySearchPaths(MITK_RUNTIME_PATH_RELEASE release RELEASE)
    mitkFunctionGetLibrarySearchPaths(MITK_RUNTIME_PATH_DEBUG debug DEBUG)
    set(test_env_path ${MITK_RUNTIME_PATH_RELEASE} ${MITK_RUNTIME_PATH_DEBUG} $ENV{PATH})
    list(REMOVE_DUPLICATES test_env_path)
    string (REGEX REPLACE "\;" "\\\;" test_env_path "${test_env_path}")
    set_property(TEST ${test_name} PROPERTY ENVIRONMENT "PATH=${test_env_path}" APPEND)
    set_property(TEST ${test_name} PROPERTY SKIP_RETURN_CODE 77)
  endif()

endfunction()

function(mitkAddCustomModuleRenderingTest test_name test_function)

  mitkAddCustomModuleTest(${test_name} ${test_function} ${ARGN})
  if(TEST ${test_name})
    set_property(TEST ${test_name} APPEND PROPERTY LABELS "Rendering Tests")
    set_property(TEST ${test_name} PROPERTY RUN_SERIAL TRUE)
  endif()

endfunction()
