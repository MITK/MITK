if(BUILD_TESTING)

  include(ExternalProject)

  set(proj PT) # Means ProjectTemplate (use a short name due to Windows limitations)
  set(MITK-ProjectTemplate_SOURCE_DIR "${MITK_BINARY_DIR}/${proj}")
  set(MITK-ProjectTemplate_BINARY_DIR "${MITK_BINARY_DIR}/${proj}-bin")

  add_test(NAME mitkProjectTemplateRmSrcTest
           COMMAND ${CMAKE_COMMAND} -E remove_directory "${MITK-ProjectTemplate_SOURCE_DIR}"
          )
  set_tests_properties(mitkProjectTemplateRmSrcTest PROPERTIES
                       LABELS "MITK;BlueBerry")

  add_test(NAME mitkProjectTemplateRmBinTest
           COMMAND ${CMAKE_COMMAND} -E remove_directory "${MITK-ProjectTemplate_BINARY_DIR}"
          )
  set_tests_properties(mitkProjectTemplateRmBinTest PROPERTIES
                       LABELS "MITK;BlueBerry")

  add_test(NAME mitkProjectTemplateMakeBinTest
           COMMAND ${CMAKE_COMMAND} -E make_directory "${MITK-ProjectTemplate_BINARY_DIR}"
          )
  set_tests_properties(mitkProjectTemplateMakeBinTest PROPERTIES
                       DEPENDS mitkProjectTemplateRmBinTest
                       LABELS "MITK;BlueBerry")

  add_test(NAME mitkProjectTemplateCloneTest
           COMMAND ${GIT_EXECUTABLE} clone http://git.mitk.org/MITK-ProjectTemplate.git ${MITK-ProjectTemplate_SOURCE_DIR}
          )
  set_tests_properties(mitkProjectTemplateCloneTest PROPERTIES
                       DEPENDS mitkProjectTemplateRmSrcTest
                       LABELS "MITK;BlueBerry")

  set(configure_options
    -DMITK_DIR:PATH=${MITK_BINARY_DIR}
    -DCMAKE_C_COMPILER:STRING=${CMAKE_C_COMPILER}
    -DCMAKE_CXX_COMPILER:STRING=${CMAKE_CXX_COMPILER}
    -DCMAKE_OSX_SYSROOT:PATH=${CMAKE_OSX_SYSROOT}
    -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=${CMAKE_OSX_DEPLOYMENT_TARGET}
    -DCMAKE_OSX_ARCHITECTURES:STRING=${CMAKE_OSX_ARCHITECTURES}
    -DAwesomeProject_BUILD_ALL_PLUGINS:BOOL=ON
    -DAwesomeProject_BUILD_ALL_APPS:BOOL=ON
    -G${CMAKE_GENERATOR}
    )
  if(MITK_USE_Qt4)
    list(APPEND configure_options -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE})
  endif()
  if(CMAKE_PREFIX_PATH)
    list(APPEND configure_options -DCMAKE_PREFIX_PATH:PATH=${CMAKE_PREFIX_PATH})
  endif()

  if(CMAKE_CONFIGURATION_TYPES)
    foreach(config ${CMAKE_CONFIGURATION_TYPES})
      add_test(NAME mitkProjectTemplateConfigureTest-${config} CONFIGURATIONS ${config}
               WORKING_DIRECTORY "${MITK-ProjectTemplate_BINARY_DIR}"
               COMMAND ${CMAKE_COMMAND} ${configure_options}
                                        "${MITK-ProjectTemplate_SOURCE_DIR}")
      set_tests_properties(mitkProjectTemplateConfigureTest-${config} PROPERTIES
                           DEPENDS "mitkProjectTemplateCloneTest;mitkProjectTemplateMakeBinTest"
                           LABELS "MITK;BlueBerry")

      add_test(NAME mitkProjectTemplateBuildTest-${config} CONFIGURATIONS ${config}
               COMMAND ${CMAKE_COMMAND} --build ${MITK-ProjectTemplate_BINARY_DIR} --config ${config})
      set_tests_properties(mitkProjectTemplateBuildTest-${config} PROPERTIES
                           DEPENDS mitkProjectTemplateConfigureTest-${config}
                           LABELS "MITK;BlueBerry")
    endforeach()
  else()
    add_test(NAME mitkProjectTemplateConfigureTest-${CMAKE_BUILD_TYPE}
             WORKING_DIRECTORY "${MITK-ProjectTemplate_BINARY_DIR}"
             COMMAND ${CMAKE_COMMAND} ${configure_options}
                                      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                                      "${MITK-ProjectTemplate_SOURCE_DIR}")
    set_tests_properties(mitkProjectTemplateConfigureTest-${CMAKE_BUILD_TYPE} PROPERTIES
                         DEPENDS "mitkProjectTemplateCloneTest;mitkProjectTemplateMakeBinTest"
                         LABELS "MITK;BlueBerry")

    add_test(NAME mitkProjectTemplateBuildTest-${CMAKE_BUILD_TYPE}
             COMMAND ${CMAKE_COMMAND} --build ${MITK-ProjectTemplate_BINARY_DIR} --config ${CMAKE_BUILD_TYPE})
    set_tests_properties(mitkProjectTemplateBuildTest-${CMAKE_BUILD_TYPE} PROPERTIES
                         DEPENDS mitkProjectTemplateConfigureTest-${CMAKE_BUILD_TYPE}
                         LABELS "MITK;BlueBerry")
  endif()

  set(package_test_configurations)
  if(WIN32)
    # Only test packaging if build type is "Release" on Windows
    set(package_test_configurations CONFIGURATIONS Release)
  endif()

  if(NOT MITK_FAST_TESTING)

    if(WIN32)
      # Only test packaging if build type is "Release" on Windows
      add_test(NAME mitkProjectTemplatePackageTest CONFIGURATIONS Release
               COMMAND ${CMAKE_COMMAND} --build ${MITK-ProjectTemplate_BINARY_DIR}/AwesomeProject-build --config Release --target package)
      set_tests_properties(mitkProjectTemplatePackageTest PROPERTIES
                           DEPENDS mitkProjectTemplateBuildTest-Release
                           TIMEOUT 6000
                           LABELS "MITK;BlueBerry;PACKAGE_TESTS")
    elseif(CMAKE_BUILD_TYPE)
      add_test(NAME mitkProjectTemplatePackageTest
               COMMAND ${CMAKE_COMMAND} --build ${MITK-ProjectTemplate_BINARY_DIR}/AwesomeProject-build --config ${CMAKE_BUILD_TYPE} --target package)
      set_tests_properties(mitkProjectTemplatePackageTest PROPERTIES
                           DEPENDS mitkProjectTemplateBuildTest-${CMAKE_BUILD_TYPE}
                           TIMEOUT 6000
                           LABELS "MITK;BlueBerry;PACKAGE_TESTS")
    endif()

  endif()

endif()
