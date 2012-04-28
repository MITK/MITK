if(BUILD_TESTING)

  include(ExternalProject)

  set(proj MITK-ProjectTemplate)
  set(MITK-ProjectTemplate_BINARY_DIR "${MITK_BINARY_DIR}/${proj}-build")

  ExternalProject_Add(${proj}
    GIT_REPOSITORY http://git.mitk.org/MITK-ProjectTemplate.git
    GIT_TAG origin/master
    SOURCE_DIR "${MITK_BINARY_DIR}/${proj}"
    BINARY_DIR "${MITK-ProjectTemplate_BINARY_DIR}"
    PREFIX "${proj}-cmake"
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    CMAKE_GENERATOR "${CMAKE_GENERATOR}"
    CMAKE_ARGS
      -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
      -DMITK_DIR:PATH=${MITK_BINARY_DIR}
      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
      -DAwesomeProject_BUILD_ALL_PLUGINS:BOOL=ON
      -DAwesomeProject_BUILD_ALL_APPS:BOOL=ON
  )
  
  add_test(NAME mitkProjectTemplateCleanTest
           COMMAND ${CMAKE_COMMAND} -E remove_directory "${MITK-ProjectTemplate_BINARY_DIR}"
          )
  set_tests_properties(mitkProjectTemplateCleanTest PROPERTIES
                       LABELS "MITK;BlueBerry")

  add_test(NAME mitkProjectTemplateCleanTest2
           COMMAND ${CMAKE_COMMAND} -E make_directory "${MITK-ProjectTemplate_BINARY_DIR}"
          )
  set_tests_properties(mitkProjectTemplateCleanTest2 PROPERTIES
                       DEPENDS mitkProjectTemplateCleanTest
                       LABELS "MITK;BlueBerry")
                       
                         
  if(CMAKE_CONFIGURATION_TYPES)
    foreach(config ${CMAKE_CONFIGURATION_TYPES})
      add_test(NAME mitkProjectTemplateConfigureTest-${config} CONFIGURATIONS ${config}
               COMMAND ${CMAKE_COMMAND} --build ${MITK_BINARY_DIR} --config ${config} --target ${proj})
      set_tests_properties(mitkProjectTemplateConfigureTest-${config} PROPERTIES
                           DEPENDS mitkProjectTemplateCleanTest2
                           LABELS "MITK;BlueBerry")
                           
      add_test(NAME mitkProjectTemplateBuildTest-${config} CONFIGURATIONS ${config}
               COMMAND ${CMAKE_COMMAND} --build ${MITK-ProjectTemplate_BINARY_DIR} --config ${config})
      set_tests_properties(mitkProjectTemplateBuildTest-${config} PROPERTIES
                           DEPENDS mitkProjectTemplateConfigureTest-${config}
                           LABELS "MITK;BlueBerry")
    endforeach()
  else()
    add_test(NAME mitkProjectTemplateConfigureTest-${CMAKE_BUILD_TYPE}
             COMMAND ${CMAKE_COMMAND} --build ${MITK_BINARY_DIR} --config ${CMAKE_BUILD_TYPE} --target ${proj})
    set_tests_properties(mitkProjectTemplateConfigureTest-${CMAKE_BUILD_TYPE} PROPERTIES
                         DEPENDS mitkProjectTemplateCleanTest2
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
                           TIMEOUT 3600 
                           LABELS "MITK;BlueBerry")
    elseif(CMAKE_BUILD_TYPE)
      add_test(NAME mitkProjectTemplatePackageTest
               COMMAND ${CMAKE_COMMAND} --build ${MITK-ProjectTemplate_BINARY_DIR}/AwesomeProject-build --config ${CMAKE_BUILD_TYPE} --target package)
      set_tests_properties(mitkProjectTemplatePackageTest PROPERTIES
                           DEPENDS mitkProjectTemplateBuildTest-${CMAKE_BUILD_TYPE}
                           TIMEOUT 3600
                           LABELS "MITK;BlueBerry")
    endif() 
    
  endif()
  
endif()
