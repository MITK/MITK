if(BUILD_TESTING AND 0)

  include(ExternalProject)

  set(proj MITK-ProjectTemplate)
  set(MITK-ProjectTemplate_BINARY_DIR "${MITK_BINARY_DIR}/${proj}-build")

  ExternalProject_Add(${proj}
    GIT_REPOSITORY http://git.mitk.org/MITK-ProjectTemplate.git
    GIT_TAG origin/master
    SOURCE_DIR "${MITK_BINARY_DIR}/${proj}"
    BINARY_DIR "${MITK-ProjectTemplate_BINARY_DIR}"
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    CMAKE_GENERATOR "${CMAKE_GENERATOR}"
    CMAKE_ARGS
      -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
      -DMITK_DIR:PATH=${MITK_BINARY_DIR}
      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
      -DAwesomeProject_BUILD_ALL_PLUGINS:BOOL=ON
  )
  
  add_test(NAME mitkProjectTemplateBuildTest
           COMMAND ${CMAKE_COMMAND} --build ${MITK-ProjectTemplate_BINARY_DIR} --config $<CONFIGURATION>)
  set_tests_properties(mitkProjectTemplateBuildTest PROPERTIES
                       LABELS "MITK;BlueBerry")

  set(package_test_configurations)
  if(WIN32)
    # Only test packaging if build type is "Release" on Windows
    set(package_test_configurations CONFIGURATIONS Release)
  endif()
  
  if(NOT MITK_FAST_TESTING)
    add_test(NAME mitkProjectTemplatePackageTest ${package_test_configurations}
             COMMAND ${CMAKE_COMMAND} --build ${MITK-ProjectTemplate_BINARY_DIR}/AwesomeProject-build --config $<CONFIGURATION> --target package)
           
    set_tests_properties(mitkProjectTemplatePackageTest PROPERTIES
                         DEPENDS mitkProjectTemplateBuildTest
                         TIMEOUT 1200
                         LABELS "MITK;BlueBerry")
  endif()
  
endif()
