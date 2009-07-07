# Create TestDriver and default tests for MITK
#
# CMake variables used:
#
# ${${KITNAME}_GUI_TESTS} : filenames of all tests that run without a parameter
# ${${KITNAME}_IMAGE_GUI_TESTS : filenames of all tests that run with an image filename as parameter
# ${${KITNAME}_TESTS} : filenames of all tests that run without a parameter
# ${${KITNAME}_IMAGE_TESTS : filenames of all tests that run with an image filename as parameter
# ${${KITNAME}_TESTIMAGES} : list of images passed as parameter for the IMAGE_TESTS
# ${${KITNAME}_CUSTOM_TESTS} : filenames of custom tests which are just added to the TestDriver. Execution
#                              of these has to be specified manually with the ADD_TEST CMake command.
# 
MACRO(MITK_CREATE_DEFAULT_TESTS)
  # add tests which need a GUI if it is not disabled
  IF(NOT MITK_GUI_TESTS_DISABLED)
    SET( ${KITNAME}_TESTS  ${${KITNAME}_TESTS} ${${KITNAME}_GUI_TESTS} )
    SET( ${KITNAME}_IMAGE_TESTS  ${${KITNAME}_IMAGE_TESTS} ${${KITNAME}_IMAGE_GUI_TESTS} )
  ENDIF(NOT MITK_GUI_TESTS_DISABLED)

  #
  # Create the TestDriver binary which contains all the tests.
  #  
  CREATE_TEST_SOURCELIST(MITKTEST_SOURCE ${KITNAME}TestDriver.cpp 
    ${${KITNAME}_TESTS} ${${KITNAME}_IMAGE_TESTS} ${${KITNAME}_CUSTOM_TESTS}
  )
  ADD_EXECUTABLE(${KITNAME}TestDriver ${MITKTEST_SOURCE})
  TARGET_LINK_LIBRARIES(${KITNAME}TestDriver ${${KITNAME}_CORE_LIBRARIES} ${${KITNAME}_LIBRARIES} ${LIBRARIES_FOR_${KITNAME}_CORE}) 
  GET_TARGET_PROPERTY(TESTDRIVER_LOCATION ${KITNAME}TestDriver LOCATION) 
  #
  # Now tell CMake which tests should be run. This is done automatically 
  # for all tests in ${KITNAME}_TESTS and ${KITNAME}_IMAGE_TESTS. The IMAGE_TESTS
  # are run for each image in the TESTIMAGES list.
  #
  FOREACH( test ${${KITNAME}_TESTS} )
    GET_FILENAME_COMPONENT(TName ${test} NAME_WE)
    ADD_TEST(${TName} ${TESTDRIVER_LOCATION} ${TName})
  ENDFOREACH( test )

  FOREACH(image ${${KITNAME}_TESTIMAGES} ${ADDITIONAL_TEST_IMAGES} )
    IF(EXISTS ${image})
      SET(IMAGE_FULL_PATH ${image})
    ELSE(EXISTS ${image})
      # todo: maybe search other paths as well
      # yes, please in mitk/Testing/Data, too
      SET(IMAGE_FULL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Data/${image})
    ENDIF(EXISTS ${image})

    IF(EXISTS ${IMAGE_FULL_PATH})
    FOREACH( test ${${KITNAME}_IMAGE_TESTS} )
      GET_FILENAME_COMPONENT(TName ${test} NAME_WE)
      GET_FILENAME_COMPONENT(ImageName ${IMAGE_FULL_PATH} NAME)
      ADD_TEST(${TName}_${ImageName} ${TESTDRIVER_LOCATION} ${TName} ${IMAGE_FULL_PATH})
    ENDFOREACH( test )
    ELSE(EXISTS ${IMAGE_FULL_PATH})
      MESSAGE("!!!!! No such file: ${IMAGE_FULL_PATH} !!!!!")
    ENDIF(EXISTS ${IMAGE_FULL_PATH})
  ENDFOREACH( image )
ENDMACRO(MITK_CREATE_DEFAULT_TESTS)

