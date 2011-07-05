if(BUILD_TESTING)

  include(ExternalProject)

  set(proj MITK-ProjectTemplate)
  set(MITK-ProjectTemplate_BINARY_DIR "${MITK_BINARY_DIR}/${proj}-build")

  ExternalProject_Add(${proj}
    GIT_REPOSITORY git://github.com/MITK/MITK-ProjectTemplate.git
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
  
  add_custom_target(MITK-ProjectTemplateBuildTest
                    ${CMAKE_COMMAND} --build ${MITK-ProjectTemplate_BINARY_DIR} --config ${CMAKE_CFG_INTDIR})

  add_test(mitkProjectTemplateBuildTest
           ${CMAKE_COMMAND} --build ${MITK_BINARY_DIR} --target MITK-ProjectTemplateBuildTest)
  set_tests_properties(mitkProjectTemplateBuildTest PROPERTIES
                       LABELS "MITK;BlueBerry")


  add_custom_target(MITK-ProjectTemplatePackageTest
                    ${CMAKE_COMMAND} --build ${MITK-ProjectTemplate_BINARY_DIR}/AwesomeProject-build --target package --config ${CMAKE_CFG_INTDIR})

  if(WIN32)
    # Only test packaging if build type is "Release" on Windows
    add_test(NAME mitkProjectTemplatePackageTest CONFIGURATIONS Release
             COMMAND ${CMAKE_COMMAND} --build ${MITK_BINARY_DIR} --target MITK-ProjectTemplatePackageTest)
  else()
    add_test(mitkProjectTemplatePackageTest
             ${CMAKE_COMMAND} --build ${MITK_BINARY_DIR} --target MITK-ProjectTemplatePackageTest)
  endif()
  
  set_tests_properties(mitkProjectTemplatePackageTest PROPERTIES
                       DEPENDS mitkProjectTemplateBuildTest
                       LABELS "MITK;BlueBerry")


endif()
