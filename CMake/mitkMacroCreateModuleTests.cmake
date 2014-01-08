#
# Create tests and testdriver for this module
#
# Usage: MITK_CREATE_MODULE_TESTS( [EXTRA_DRIVER_INIT init_code]  )
#
# EXTRA_DRIVER_INIT is inserted as c++ code in the testdriver and will be executed before each test
#
macro(MITK_CREATE_MODULE_TESTS)
  MACRO_PARSE_ARGUMENTS(MODULE_TEST
                        "EXTRA_DRIVER_INIT;EXTRA_DRIVER_INCLUDE;EXTRA_DEPENDS" "" ${ARGN})

  if(BUILD_TESTING AND MODULE_IS_ENABLED)
    set(OLD_MOC_H_FILES ${MOC_H_FILES})
    set(MOC_H_FILES)
    include(files.cmake)
    include_directories(.)

    if(MODULE_TEST_EXTRA_DEPENDS)
      message(WARNING "The keyword EXTRA_DEPENDS is deprecated. Use a separate call to mitk_use_modules instead.")
    endif()

    if(DEFINED MOC_H_FILES)
      QT4_WRAP_CPP(MODULE_TEST_GENERATED_MOC_CPP ${MOC_H_FILES} OPTIONS -DBOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION)
    endif(DEFINED MOC_H_FILES)

    # The PACKAGE_DEPENDS variable is filled in the MITK_CHECK_MODULE() macro
    foreach(package ${PACKAGE_DEPENDS})
      if(NOT ${package} MATCHES "^Qt[45].*$")
        foreach(dir ${MODULES_PACKAGE_DEPENDS_DIRS})
          if(EXISTS "${dir}/MITK_${package}_Config.cmake")
            include("${dir}/MITK_${package}_Config.cmake")
            break()
          endif()
        endforeach()
      endif()
    endforeach()
    if(ALL_LIBRARY_DIRS)
      list(REMOVE_DUPLICATES ALL_LIBRARY_DIRS)
      link_directories(${ALL_LIBRARY_DIRS})
    endif()

    set(TESTDRIVER ${MODULE_NAME}TestDriver)
    set(MODULE_TEST_EXTRA_DRIVER_INIT "${MODULE_TEST_EXTRA_DRIVER_INIT}")

    # Write a header file containing include directives and custom code
    # for the test driver.
    set(_extra_include_content )
    list(APPEND MODULE_TEST_EXTRA_DRIVER_INCLUDE "mitkLog.h")
    list(REMOVE_DUPLICATES MODULE_TEST_EXTRA_DRIVER_INCLUDE)
    foreach(_include ${MODULE_TEST_EXTRA_DRIVER_INCLUDE})
      set(_extra_include_content "${_extra_include_content}
#include <${_include}>")
    endforeach()
    set(_extra_include_content "${_extra_include_content}
#include <vector>
std::vector<std::string> globalCmdLineArgs;")
    set(_extra_include_file ${CMAKE_CURRENT_BINARY_DIR}/${TESTDRIVER}_extras.h)
    file(WRITE ${_extra_include_file} "${_extra_include_content}")

    set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "
for (int avIndex = 1; avIndex < ac; ++avIndex) globalCmdLineArgs.push_back(av[avIndex]);
mitk::LoggingBackend::Register();
${MODULE_TEST_EXTRA_DRIVER_INIT};"
)
    set(CMAKE_TESTDRIVER_AFTER_TESTMAIN "mitk::LoggingBackend::Unregister();")

    create_test_sourcelist(MODULETEST_SOURCE ${MODULE_NAME}TestDriver.cpp
      ${MODULE_TESTS} ${MODULE_IMAGE_TESTS} ${MODULE_SURFACE_TESTS} ${MODULE_CUSTOM_TESTS}
      EXTRA_INCLUDE ${_extra_include_file}
    )

    add_executable(${TESTDRIVER} ${MODULETEST_SOURCE} ${MODULE_TEST_GENERATED_MOC_CPP} ${TEST_CPP_FILES})
    mitk_use_modules(TARGET ${TESTDRIVER}
                     MODULES ${MODULE_PROVIDES} ${MODULE_TEST_EXTRA_DEPENDS}
                     PACKAGES CppUnit
                    )

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

    foreach(surface ${MODULE_TESTSURFACES} ${ADDITIONAL_TEST_SURFACES} )
      if(EXISTS ${surface})
        set(SURFACE_FULL_PATH ${surface})
      else(EXISTS ${surface})
        # todo: maybe search other paths as well
        # yes, please in mitk/Testing/Data, too
        set(SURFACE_FULL_PATH ${MITK_DATA_DIR}/${surface})
      endif(EXISTS ${surface})

      if(EXISTS ${SURFACE_FULL_PATH})
        foreach( test ${MODULE_SURFACE_TESTS} )
          get_filename_component(TName ${test} NAME_WE)
          get_filename_component(SurfaceName ${SURFACE_FULL_PATH} NAME)
          add_test(${TName}_${SurfaceName} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTDRIVER} ${TName} ${SURFACE_FULL_PATH})
          # Add labels for CDash subproject support
          if(MODULE_SUBPROJECTS)
            set_property(TEST ${TName}_${SurfaceName} PROPERTY LABELS ${MODULE_SUBPROJECTS} MITK)
          endif()
        endforeach( test )
      else(EXISTS ${SURFACE_FULL_PATH})
        message("!!!!! No such surface file: ${SURFACE_FULL_PATH} !!!!!")
      endif(EXISTS ${SURFACE_FULL_PATH})
    endforeach( surface )

    set(MOC_H_FILES ${OLD_MOC_H_FILES})
  endif(BUILD_TESTING AND MODULE_IS_ENABLED)

endmacro(MITK_CREATE_MODULE_TESTS)
