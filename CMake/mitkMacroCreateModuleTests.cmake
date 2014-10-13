#
# Create tests and testdriver for this module
#
# Usage: MITK_CREATE_MODULE_TESTS( [EXTRA_DRIVER_INIT init_code]  )
#
# EXTRA_DRIVER_INIT is inserted as c++ code in the testdriver and will be executed before each test
#
macro(MITK_CREATE_MODULE_TESTS)
  MACRO_PARSE_ARGUMENTS(MODULE_TEST
                        "EXTRA_DRIVER_INIT;EXTRA_DRIVER_INCLUDE;EXTRA_DEPENDS" "US_MODULE" ${ARGN})

  if(BUILD_TESTING AND MODULE_IS_ENABLED)
    include(files.cmake)
    include_directories(.)

    set(TESTDRIVER ${MODULE_NAME}TestDriver)

    set(MODULE_TEST_EXTRA_DRIVER_INIT "${MODULE_TEST_EXTRA_DRIVER_INIT}")

    set(_no_init NO_INIT)
    if(MODULE_TEST_US_MODULE)
      set(_no_init )
    endif()

    set(MITK_MODULE_NAME_REGEX_MATCH )
    set(MITK_MODULE_NAME_REGEX_NOT_MATCH )

    set(_testdriver_file_list ${CMAKE_CURRENT_BINARY_DIR}/testdriver_files.cmake)
    configure_file(${MITK_CMAKE_DIR}/mitkTestDriverFiles.cmake.in ${_testdriver_file_list} @ONLY)
    mitk_create_executable(${TESTDRIVER}
                           DEPENDS ${MODULE_NAME} ${MODULE_TEST_EXTRA_DEPENDS} MitkTestingHelper
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
    endforeach()

    foreach(image ${MODULE_TESTIMAGES} ${ADDITIONAL_TEST_IMAGES} )
      if(EXISTS ${image})
        set(IMAGE_FULL_PATH ${image})
      else()
        # todo: maybe search other paths as well
        # yes, please in mitk/Testing/Data, too
        set(IMAGE_FULL_PATH ${MITK_DATA_DIR}/${image})
      endif()

      if(EXISTS ${IMAGE_FULL_PATH})
        foreach( test ${MODULE_IMAGE_TESTS} )
          get_filename_component(TName ${test} NAME_WE)
          get_filename_component(ImageName ${IMAGE_FULL_PATH} NAME)
          add_test(${TName}_${ImageName} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTDRIVER} ${TName} ${IMAGE_FULL_PATH})
          # Add labels for CDash subproject support
          if(MODULE_SUBPROJECTS)
            set_property(TEST ${TName}_${ImageName} PROPERTY LABELS ${MODULE_SUBPROJECTS} MITK)
          endif()
        endforeach()
      else()
        message("!!!!! No such file: ${IMAGE_FULL_PATH} !!!!!")
      endif()
    endforeach()

    foreach(surface ${MODULE_TESTSURFACES} ${ADDITIONAL_TEST_SURFACES} )
      if(EXISTS ${surface})
        set(SURFACE_FULL_PATH ${surface})
      else()
        # todo: maybe search other paths as well
        # yes, please in mitk/Testing/Data, too
        set(SURFACE_FULL_PATH ${MITK_DATA_DIR}/${surface})
      endif()

      if(EXISTS ${SURFACE_FULL_PATH})
        foreach( test ${MODULE_SURFACE_TESTS} )
          get_filename_component(TName ${test} NAME_WE)
          get_filename_component(SurfaceName ${SURFACE_FULL_PATH} NAME)
          add_test(${TName}_${SurfaceName} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTDRIVER} ${TName} ${SURFACE_FULL_PATH})
          # Add labels for CDash subproject support
          if(MODULE_SUBPROJECTS)
            set_property(TEST ${TName}_${SurfaceName} PROPERTY LABELS ${MODULE_SUBPROJECTS} MITK)
          endif()
        endforeach()
      else()
        message("!!!!! No such surface file: ${SURFACE_FULL_PATH} !!!!!")
      endif()
    endforeach()

  endif()

endmacro()
