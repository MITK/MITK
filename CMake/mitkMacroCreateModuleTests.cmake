#
# Create tests and testdriver for this module
#
# Usage: MITK_CREATE_MODULE_TESTS( [EXTRA_DRIVER_INIT init_code]  )
#
# EXTRA_DRIVER_INIT is inserted as c++ code in the testdriver and will be executed before each test
#
macro(MITK_CREATE_MODULE_TESTS)
  MACRO_PARSE_ARGUMENTS(MODULE_TEST
                        "EXTRA_DRIVER_INIT;EXTRA_DRIVER_INCLUDE" "" ${ARGN})

  if(BUILD_TESTING AND MODULE_IS_ENABLED)
    set(OLD_MOC_H_FILES ${MOC_H_FILES})
    set(MOC_H_FILES)
    include(files.cmake)
    include_directories(.)

    if(DEFINED MOC_H_FILES)
      QT4_WRAP_CPP(MODULE_TEST_GENERATED_MOC_CPP ${MOC_H_FILES})
    endif(DEFINED MOC_H_FILES)

    set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "mitk::LoggingBackend::Register(); ${MODULE_TEST_EXTRA_DRIVER_INIT};")
    set(CMAKE_TESTDRIVER_AFTER_TESTMAIN "mitk::LoggingBackend::Unregister();")
    if(NOT MODULE_TEST_EXTRA_DRIVER_INCLUDE)
      # this is necessary to make the LoggingBackend calls available if nothing else is included
      set(MODULE_TEST_EXTRA_DRIVER_INCLUDE "mitkLog.h")
    endif(NOT MODULE_TEST_EXTRA_DRIVER_INCLUDE)

    create_test_sourcelist(MODULETEST_SOURCE ${MODULE_NAME}TestDriver.cpp
      ${MODULE_TESTS} ${MODULE_IMAGE_TESTS} ${MODULE_CUSTOM_TESTS}
      EXTRA_INCLUDE ${MODULE_TEST_EXTRA_DRIVER_INCLUDE}
    )

    set(TESTDRIVER ${MODULE_NAME}TestDriver)
    add_executable(${TESTDRIVER} ${MODULETEST_SOURCE} ${MODULE_TEST_GENERATED_MOC_CPP} ${TEST_CPP_FILES})
    target_link_libraries(${TESTDRIVER} ${MODULE_PROVIDES} ${ALL_LIBRARIES})

    if(MODULE_SUBPROJECTS)
      foreach(subproject ${MODULE_SUBPROJECTS})
        add_dependencies(${subproject} ${TESTDRIVER})
      endforeach()
    endif()

    #
    # Now tell CMake which tests should be run. This is done automatically
    # for all tests in ${KITNAME}_TESTS and ${KITNAME}_IMAGE_TESTS. The IMAGE_TESTS
    # are run for each image in the TESTIMAGES list.
    #
    foreach( test ${MODULE_TESTS} )
      get_filename_component(TName ${test} NAME_WE)
      add_test(${TName} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTDRIVER} ${TName})
      # Add labels for CDash subproject support
      if(MODULE_SUBPROJECTS)
        set_property(TEST ${TName} PROPERTY LABELS ${MODULE_SUBPROJECTS} MITK)
      endif()
    endforeach( test )

    foreach(image ${MODULE_TESTIMAGES} ${ADDITIONAL_TEST_IMAGES} )
      if(EXISTS ${image})
        set(IMAGE_FULL_PATH ${image})
      else(EXISTS ${image})
        # todo: maybe search other paths as well
        # yes, please in mitk/Testing/Data, too
        set(IMAGE_FULL_PATH ${MITK_DATA_DIR}/${image})
      endif(EXISTS ${image})

      if(EXISTS ${IMAGE_FULL_PATH})
        foreach( test ${MODULE_IMAGE_TESTS} )
          get_filename_component(TName ${test} NAME_WE)
          get_filename_component(ImageName ${IMAGE_FULL_PATH} NAME)
          add_test(${TName}_${ImageName} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTDRIVER} ${TName} ${IMAGE_FULL_PATH})
          # Add labels for CDash subproject support
          if(MODULE_SUBPROJECTS)
            set_property(TEST ${TName}_${ImageName} PROPERTY LABELS ${MODULE_SUBPROJECTS} MITK)
          endif()
        endforeach( test )
      else(EXISTS ${IMAGE_FULL_PATH})
        message("!!!!! No such file: ${IMAGE_FULL_PATH} !!!!!")
      endif(EXISTS ${IMAGE_FULL_PATH})
    endforeach( image )

    set(MOC_H_FILES ${OLD_MOC_H_FILES})
  endif(BUILD_TESTING AND MODULE_IS_ENABLED)

endmacro(MITK_CREATE_MODULE_TESTS)
