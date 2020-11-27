#
# Create tests and testdriver for this module
#
# Usage: MITK_CREATE_MODULE_TESTS( [EXTRA_DRIVER_INIT init_code]  )
#
# EXTRA_DRIVER_INIT is inserted as c++ code in the testdriver and will be executed before each test
#
macro(MITK_CREATE_MODULE_TESTS)
  cmake_parse_arguments(MODULE_TEST
                        "US_MODULE;NO_INIT" "EXTRA_DRIVER_INIT;EXTRA_DRIVER_INCLUDE" "EXTRA_DEPENDS;DEPENDS;PACKAGE_DEPENDS;TARGET_DEPENDS" ${ARGN})

  if(BUILD_TESTING AND MODULE_IS_ENABLED)
    include(files.cmake)
    include_directories(.)

    set(TESTDRIVER ${MODULE_NAME}TestDriver)

    set(MODULE_TEST_EXTRA_DRIVER_INIT "${MODULE_TEST_EXTRA_DRIVER_INIT}")

    if(MITK_XVFB_TESTING)
      set(xvfb_run ${MITK_XVFB_TESTING_COMMAND})
    else()
      set(xvfb_run )
    endif()

    if(MODULE_TEST_US_MODULE)
      message(WARNING "The US_MODULE argument is deprecated and should be removed")
    endif()

    if(MODULE_TEST_US_MODULE AND MODULE_TEST_NO_INIT)
      message(WARNING "Conflicting arguments US_MODULE and NO_INIT: NO_INIT wins.")
    endif()

    set(_no_init)
    if(MODULE_TEST_NO_INIT)
      set(_no_init NO_INIT)
    endif()

    set(MITK_MODULE_NAME_REGEX_MATCH )
    set(MITK_MODULE_NAME_REGEX_NOT_MATCH )

    set(_testdriver_file_list ${CMAKE_CURRENT_BINARY_DIR}/testdriver_files.cmake)
    configure_file(${MITK_CMAKE_DIR}/mitkTestDriverFiles.cmake.in ${_testdriver_file_list} @ONLY)
    mitk_create_executable(${TESTDRIVER}
                           DEPENDS ${MODULE_NAME} ${MODULE_TEST_DEPENDS} ${MODULE_TEST_EXTRA_DEPENDS} MitkTestingHelper
                           PACKAGE_DEPENDS ${MODULE_TEST_PACKAGE_DEPENDS}
                           TARGET_DEPENDS ${MODULE_TEST_TARGET_DEPENDS}
                           FILES_CMAKE ${_testdriver_file_list}
                           NO_FEATURE_INFO
                           NO_BATCH_FILE
                           NO_INSTALL
                           ${_no_init})
    set_property(TARGET ${EXECUTABLE_TARGET} PROPERTY FOLDER "${MITK_ROOT_FOLDER}/Modules/Tests")

    #
    # Now tell CMake which tests should be run. This is done automatically
    # for all tests in ${KITNAME}_TESTS and ${KITNAME}_IMAGE_TESTS. The IMAGE_TESTS
    # are run for each image in the TESTIMAGES list.
    #
    include(files.cmake)
    foreach(test ${MODULE_TESTS} ${MODULE_RENDERING_TESTS})
      get_filename_component(TName ${test} NAME_WE)
      add_test(NAME ${TName} COMMAND ${xvfb_run} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTDRIVER} ${TName})
      mitkFunctionGetLibrarySearchPaths(MITK_RUNTIME_PATH_RELEASE release RELEASE)
      mitkFunctionGetLibrarySearchPaths(MITK_RUNTIME_PATH_DEBUG debug DEBUG)
      set(test_env_path ${MITK_RUNTIME_PATH_RELEASE} ${MITK_RUNTIME_PATH_DEBUG} $ENV{PATH})
      list(REMOVE_DUPLICATES test_env_path)
      string (REGEX REPLACE "\;" "\\\;" test_env_path "${test_env_path}")
      set_property(TEST ${TName} PROPERTY ENVIRONMENT "PATH=${test_env_path}" APPEND)
      set_property(TEST ${TName} PROPERTY SKIP_RETURN_CODE 77)
    endforeach()

    foreach(test ${MODULE_RENDERING_TESTS})
      get_filename_component(TName ${test} NAME_WE)
      set_property(TEST ${TName} APPEND PROPERTY LABELS "Rendering Tests")
      set_property(TEST ${TName} PROPERTY RUN_SERIAL TRUE)
    endforeach()

    set(TEST_TYPES IMAGE SURFACE POINTSET) # add other file types here

    foreach(test_type ${TEST_TYPES})
       foreach(test_data ${MODULE_TEST${test_type}} ${ADDITIONAL_TEST_${test_type}})
         if(EXISTS ${test_data})
           set(TEST_DATA_FULL_PATH ${test_data})
          else()
             # todo: maybe search other paths as well
             # yes, please in mitk/Testing/Data, too
             set(TEST_DATA_FULL_PATH ${MITK_DATA_DIR}/${test_data})
          endif()

           if(EXISTS ${TEST_DATA_FULL_PATH})
             foreach( test ${MODULE_${test_type}_TESTS})
               get_filename_component(TName ${test} NAME_WE)
               get_filename_component(DName ${TEST_DATA_FULL_PATH} NAME)
               add_test(NAME ${TName}_${DName} COMMAND ${xvfb_run} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTDRIVER} ${TName} ${TEST_DATA_FULL_PATH})
               set_property(TEST ${TName}_${DName} PROPERTY ENVIRONMENT "PATH=${test_env_path}" APPEND)
               set_property(TEST ${TName}_${DName} PROPERTY SKIP_RETURN_CODE 77)
             endforeach()
           else()
             message("!!!!! No such file: ${TEST_DATA_FULL_PATH} !!!!!")
           endif()
         endforeach()
    endforeach()

 endif()

endmacro()
