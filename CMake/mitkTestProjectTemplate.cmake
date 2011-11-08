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
  )
                         
  if(CMAKE_CONFIGURATION_TYPES)
    foreach(config ${CMAKE_CONFIGURATION_TYPES})
      add_test(NAME mitkProjectTemplateBuildTest-${config} CONFIGURATIONS ${config}
               COMMAND ${CMAKE_COMMAND} --build ${MITK-ProjectTemplate_BINARY_DIR} --config ${config})
      set_tests_properties(mitkProjectTemplateBuildTest-${config} PROPERTIES
                           LABELS "MITK;BlueBerry")
    endforeach()
  else()
    add_test(mitkProjectTemplateBuildTest-${CMAKE_BUILD_TYPE}
             ${CMAKE_COMMAND} --build ${MITK-ProjectTemplate_BINARY_DIR} --config ${CMAKE_BUILD_TYPE})
    set_tests_properties(mitkProjectTemplateBuildTest-${CMAKE_BUILD_TYPE} PROPERTIES
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
      add_test(mitkProjectTemplatePackageTest
               ${CMAKE_COMMAND} --build ${MITK-ProjectTemplate_BINARY_DIR}/AwesomeProject-build --config ${CMAKE_BUILD_TYPE} --target package)
      set_tests_properties(mitkProjectTemplatePackageTest PROPERTIES
                           DEPENDS mitkProjectTemplateBuildTest-${CMAKE_BUILD_TYPE}
                           TIMEOUT 3600
                           LABELS "MITK;BlueBerry")
    endif() 
    
  endif()
  
endif()
