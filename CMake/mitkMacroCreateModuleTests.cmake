#
# Create tests and testdriver for this module
# 
# Usage: MITK_CREATE_MODULE_TESTS( [EXTRA_DRIVER_INIT init_code]  )
# 
# EXTRA_DRIVER_INIT is inserted as c++ code in the testdriver and will be executed before each test
#
MACRO(MITK_CREATE_MODULE_TESTS)
  MACRO_PARSE_ARGUMENTS(MODULE_TEST 
                        "EXTRA_DRIVER_INIT;EXTRA_DRIVER_INCLUDE" "" ${ARGN})

  IF(BUILD_TESTING AND MODULE_IS_ENABLED)
    SET(OLD_MOC_H_FILES ${MOC_H_FILES})
    SET(MOC_H_FILES)
    INCLUDE(files.cmake)
    INCLUDE_DIRECTORIES(.)
    
    IF(DEFINED MOC_H_FILES)
      QT4_WRAP_CPP(MODULE_TEST_GENERATED_MOC_CPP ${MOC_H_FILES})
    ENDIF(DEFINED MOC_H_FILES)
     
    SET(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "mitk::LoggingBackend::Register(); ${MODULE_TEST_EXTRA_DRIVER_INIT};")
    SET(CMAKE_TESTDRIVER_AFTER_TESTMAIN "mitk::LoggingBackend::Unregister();")
    IF(NOT MODULE_TEST_EXTRA_DRIVER_INCLUDE)
      # this is necessary to make the LoggingBackend calls available if nothing else is included
      SET(MODULE_TEST_EXTRA_DRIVER_INCLUDE "mitkLog.h")
    ENDIF(NOT MODULE_TEST_EXTRA_DRIVER_INCLUDE)
    
    CREATE_TEST_SOURCELIST(MODULETEST_SOURCE ${MODULE_NAME}TestDriver.cpp 
      ${MODULE_TESTS} ${MODULE_IMAGE_TESTS} ${MODULE_CUSTOM_TESTS}
      EXTRA_INCLUDE ${MODULE_TEST_EXTRA_DRIVER_INCLUDE}
    )
    
    SET(TESTDRIVER ${MODULE_NAME}TestDriver)
    ADD_EXECUTABLE(${TESTDRIVER} ${MODULETEST_SOURCE} ${MODULE_TEST_GENERATED_MOC_CPP})
    TARGET_LINK_LIBRARIES(${TESTDRIVER} ${MODULE_PROVIDES} ${ALL_LIBRARIES})
    
    IF(MODULE_SUBPROJECTS)
      FOREACH(subproject ${MODULE_SUBPROJECTS})
        ADD_DEPENDENCIES(${subproject} ${TESTDRIVER})
      ENDFOREACH()
    ENDIF()
    
    #
    # Now tell CMake which tests should be run. This is done automatically 
    # for all tests in ${KITNAME}_TESTS and ${KITNAME}_IMAGE_TESTS. The IMAGE_TESTS
    # are run for each image in the TESTIMAGES list.
    #
    FOREACH( test ${MODULE_TESTS} )
      GET_FILENAME_COMPONENT(TName ${test} NAME_WE)
      ADD_TEST(${TName} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTDRIVER} ${TName})
      # Add labels for CDash subproject support
      IF(MODULE_SUBPROJECTS)
        SET_PROPERTY(TEST ${TName} PROPERTY LABELS ${MODULE_SUBPROJECTS} MITK)
      ENDIF()
    ENDFOREACH( test )

    FOREACH(image ${MODULE_TESTIMAGES} ${ADDITIONAL_TEST_IMAGES} )
      IF(EXISTS ${image})
        SET(IMAGE_FULL_PATH ${image})
      ELSE(EXISTS ${image})
        # todo: maybe search other paths as well
        # yes, please in mitk/Testing/Data, too
        SET(IMAGE_FULL_PATH ${MITK_DATA_DIR}/${image})
      ENDIF(EXISTS ${image})

      IF(EXISTS ${IMAGE_FULL_PATH})
        FOREACH( test ${MODULE_IMAGE_TESTS} )
          GET_FILENAME_COMPONENT(TName ${test} NAME_WE)
          GET_FILENAME_COMPONENT(ImageName ${IMAGE_FULL_PATH} NAME)
          ADD_TEST(${TName}_${ImageName} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTDRIVER} ${TName} ${IMAGE_FULL_PATH})
          # Add labels for CDash subproject support
          IF(MODULE_SUBPROJECTS)
            SET_PROPERTY(TEST ${TName}_${ImageName} PROPERTY LABELS ${MODULE_SUBPROJECTS} MITK)
          ENDIF()
        ENDFOREACH( test )
      ELSE(EXISTS ${IMAGE_FULL_PATH})
        MESSAGE("!!!!! No such file: ${IMAGE_FULL_PATH} !!!!!")
      ENDIF(EXISTS ${IMAGE_FULL_PATH})
    ENDFOREACH( image )

    SET(MOC_H_FILES ${OLD_MOC_H_FILES})
  ENDIF(BUILD_TESTING AND MODULE_IS_ENABLED)

ENDMACRO(MITK_CREATE_MODULE_TESTS)
