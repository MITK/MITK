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
macro(MITK_CREATE_DEFAULT_TESTS)
  # add tests which need a GUI if it is not disabled
  if(NOT MITK_GUI_TESTS_DISABLED)
    set( ${KITNAME}_TESTS  ${${KITNAME}_TESTS} ${${KITNAME}_GUI_TESTS} )
    set( ${KITNAME}_IMAGE_TESTS  ${${KITNAME}_IMAGE_TESTS} ${${KITNAME}_IMAGE_GUI_TESTS} )
  endif()

  #
  # Create the TestDriver binary which contains all the tests.
  #
  create_test_sourcelist(MITKTEST_SOURCE ${KITNAME}TestDriver.cpp
    ${${KITNAME}_TESTS} ${${KITNAME}_IMAGE_TESTS} ${${KITNAME}_CUSTOM_TESTS}
  )
  add_executable(${KITNAME}TestDriver ${MITKTEST_SOURCE})
  set_property(TARGET ${KITNAME}TestDriver PROPERTY LABELS ${PROJECT_NAME})
  target_link_libraries(${KITNAME}TestDriver ${${KITNAME}_CORE_LIBRARIES} ${${KITNAME}_LIBRARIES} ${LIBRARIES_FOR_${KITNAME}_CORE})
  #
  # Now tell CMake which tests should be run. This is done automatically
  # for all tests in ${KITNAME}_TESTS and ${KITNAME}_IMAGE_TESTS. The IMAGE_TESTS
  # are run for each image in the TESTIMAGES list.
  #
  foreach( test ${${KITNAME}_TESTS} )
    get_filename_component(TName ${test} NAME_WE)
    add_test(${TName} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${KITNAME}TestDriver ${TName})
    set_property(TEST ${TName} PROPERTY LABELS ${PROJECT_NAME})
    
    mitkFunctionGetLibrarySearchPaths(MITK_RUNTIME_PATH_RELEASE release RELEASE)
    mitkFunctionGetLibrarySearchPaths(MITK_RUNTIME_PATH_DEBUG debug DEBUG)
    string (REGEX REPLACE "\;" "\\\;" MITK_RUNTIME_PATH_RELEASE "${MITK_RUNTIME_PATH_RELEASE}")
    string (REGEX REPLACE "\;" "\\\;" MITK_RUNTIME_PATH_DEBUG "${MITK_RUNTIME_PATH_DEBUG}")
    set_property(TEST ${TName} PROPERTY ENVIRONMENT "PATH=${MITK_RUNTIME_PATH_RELEASE}\;${MITK_RUNTIME_PATH_DEBUG}" APPEND)
    set_property(TEST ${TName} PROPERTY SKIP_RETURN_CODE 77)
  endforeach()

  foreach(image ${${KITNAME}_TESTIMAGES} ${ADDITIONAL_TEST_IMAGES} )
    if(EXISTS "${image}")
      set(IMAGE_FULL_PATH ${image})
    else()
      # todo: maybe search other paths as well
      # yes, please in mitk/Testing/Data, too
      set(IMAGE_FULL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Data/${image})
    endif()

    if(EXISTS "${IMAGE_FULL_PATH}")
      foreach( test ${${KITNAME}_IMAGE_TESTS} )
        get_filename_component(TName ${test} NAME_WE)
        get_filename_component(ImageName ${IMAGE_FULL_PATH} NAME)
        add_test(${TName}_${ImageName} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${KITNAME}TestDriver ${TName} ${IMAGE_FULL_PATH})
        set_property(TEST ${TName}_${ImageName} PROPERTY LABELS ${PROJECT_NAME})
        set_property(TEST ${TName}_${ImageName} PROPERTY ENVIRONMENT "PATH=${MITK_RUNTIME_PATH_RELEASE}\;${MITK_RUNTIME_PATH_DEBUG}" APPEND)
        set_property(TEST ${TName}_${ImageName} PROPERTY SKIP_RETURN_CODE 77)
      endforeach()
    else()
      message("!!!!! No such file: ${IMAGE_FULL_PATH} !!!!!")
    endif()
  endforeach()
endmacro()

