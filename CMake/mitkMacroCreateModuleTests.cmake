#
# Create tests and testdriver for this module
#
# Usage: MITK_CREATE_MODULE_TESTS( [EXTRA_DRIVER_INIT init_code]  )
#
# EXTRA_DRIVER_INIT is inserted as c++ code in the testdriver and will be executed before each test
#
macro(MITK_CREATE_MODULE_TESTS)
  cmake_parse_arguments(MODULE_TEST
                        "US_MODULE;NO_INIT" "EXTRA_DRIVER_INIT;EXTRA_DRIVER_INCLUDE" "EXTRA_DEPENDS;DEPENDS;PACKAGE_DEPENDS" ${ARGN})

  if(BUILD_TESTING AND MODULE_IS_ENABLED)
    include(files.cmake)
    include_directories(.)

    set(TESTDRIVER ${MODULE_NAME}TestDriver)

    set(MODULE_TEST_EXTRA_DRIVER_INIT "${MODULE_TEST_EXTRA_DRIVER_INIT}")

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
                           SUBPROJECTS ${MODULE_SUBPROJECTS}
                           FILES_CMAKE ${_testdriver_file_list}
                           NO_FEATURE_INFO NO_BATCH_FILE ${_no_init})

    #
    # Now tell CMake which tests should be run. This is done automatically
    # for all tests in ${KITNAME}_TESTS and ${KITNAME}_IMAGE_TESTS. The IMAGE_TESTS
    # are run for each image in the TESTIMAGES list.
    #
    include(files.cmake)
    foreach( test ${MODULE_TESTS} )
      get_filename_component(TName ${test} NAME_WE)
      add_test(${TName} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTDRIVER} ${TName})
      # Add labels for CDash subproject support
      if(MODULE_SUBPROJECTS)
        set_property(TEST ${TName} PROPERTY LABELS ${MODULE_SUBPROJECTS} MITK)
      endif()
      mitkFunctionGetLibrarySearchPaths(MITK_RUNTIME_PATH_RELEASE release RELEASE)
      mitkFunctionGetLibrarySearchPaths(MITK_RUNTIME_PATH_DEBUG debug DEBUG)
      string (REGEX REPLACE "\;" "\\\;" MITK_RUNTIME_PATH_RELEASE "${MITK_RUNTIME_PATH_RELEASE}")
      string (REGEX REPLACE "\;" "\\\;" MITK_RUNTIME_PATH_DEBUG "${MITK_RUNTIME_PATH_DEBUG}")
      set_property(TEST ${TName} PROPERTY ENVIRONMENT "PATH=${MITK_RUNTIME_PATH_RELEASE}\;${MITK_RUNTIME_PATH_DEBUG}" APPEND)
      set_property(TEST ${TName} PROPERTY SKIP_RETURN_CODE 77)
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
               add_test(${TName}_${DName} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTDRIVER} ${TName} ${TEST_DATA_FULL_PATH})
               # Add labels for CDash subproject support
               if(MODULE_SUBPROJECTS)
                 set_property(TEST ${TName}_${DName} PROPERTY LABELS ${MODULE_SUBPROJECTS} MITK)
               endif()
               set_property(TEST ${TName}_${DName} PROPERTY ENVIRONMENT "PATH=${MITK_RUNTIME_PATH_RELEASE}\;${MITK_RUNTIME_PATH_DEBUG}" APPEND)
               set_property(TEST ${TName}_${DName} PROPERTY SKIP_RETURN_CODE 77)
             endforeach()
           else()
             message("!!!!! No such file: ${TEST_DATA_FULL_PATH} !!!!!")
           endif()
         endforeach()
    endforeach()

 endif()

endmacro()
