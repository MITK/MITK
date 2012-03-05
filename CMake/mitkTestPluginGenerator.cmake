if(BUILD_TESTING)
 
  set(proj GeneratedTestProject)
  set(test_project_out_dir "${MITK_BINARY_DIR}")
  set(test_project_source_dir "${MITK_BINARY_DIR}/${proj}")
  set(test_project_binary_dir "${MITK_BINARY_DIR}/${proj}-bin")
  
  add_test(NAME mitkPluginGeneratorCleanTest
           COMMAND ${CMAKE_COMMAND} -E remove_directory "${test_project_source_dir}"
          )
  set_tests_properties(mitkPluginGeneratorCleanTest PROPERTIES
                       LABELS "MITK;BlueBerry")
  
  add_test(NAME mitkPluginGeneratorCleanTest2
           COMMAND ${CMAKE_COMMAND} -E remove_directory "${test_project_binary_dir}"
          )
  set_tests_properties(mitkPluginGeneratorCleanTest2 PROPERTIES
                       LABELS "MITK;BlueBerry")

  add_test(NAME mitkPluginGeneratorCleanTest3
           COMMAND ${CMAKE_COMMAND} -E make_directory "${test_project_binary_dir}"
          )
  set_tests_properties(mitkPluginGeneratorCleanTest3 PROPERTIES
                       DEPENDS mitkPluginGeneratorCleanTest2
                       LABELS "MITK;BlueBerry")
  
  add_test(NAME mitkPluginGeneratorCreateTest
           COMMAND ${exec_target} --project-name "${proj}" --project-app-name "TestApp"
                                  -ps org.test.plugin -pn "Test Plugin" -vn "Test View"
                                  -o ${test_project_out_dir} -y -n
          )
  set_tests_properties(mitkPluginGeneratorCreateTest PROPERTIES
                       DEPENDS "${exec_target};mitkPluginGeneratorCleanTest;mitkPluginGeneratorCleanTest3"
                       LABELS "MITK;BlueBerry")
                         
  if(CMAKE_CONFIGURATION_TYPES)
    foreach(config ${CMAKE_CONFIGURATION_TYPES})
      add_test(NAME mitkPluginGeneratorConfigureTest-${config} CONFIGURATIONS ${config}
               WORKING_DIRECTORY "${test_project_binary_dir}"
               COMMAND ${CMAKE_COMMAND} -D MITK_DIR:PATH=${MITK_BINARY_DIR}
                                        -G ${CMAKE_GENERATOR}
                                        "${test_project_source_dir}")
      set_tests_properties(mitkPluginGeneratorConfigureTest-${config} PROPERTIES
                           DEPENDS mitkPluginGeneratorCreateTest
                           LABELS "MITK;BlueBerry")
                           
      add_test(NAME mitkPluginGeneratorBuildTest-${config} CONFIGURATIONS ${config}
               COMMAND ${CMAKE_COMMAND} --build ${test_project_binary_dir} --config ${config})
      set_tests_properties(mitkPluginGeneratorBuildTest-${config} PROPERTIES
                           DEPENDS mitkPluginGeneratorConfigureTest-${config}
                           LABELS "MITK;BlueBerry")
    endforeach()
  else()
    add_test(NAME mitkPluginGeneratorConfigureTest-${CMAKE_BUILD_TYPE}
             WORKING_DIRECTORY "${test_project_binary_dir}"
             COMMAND ${CMAKE_COMMAND} -D MITK_DIR:PATH=${MITK_BINARY_DIR}
                                      -D CMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
                                      -G ${CMAKE_GENERATOR}
                                      "${test_project_source_dir}")
    set_tests_properties(mitkPluginGeneratorConfigureTest-${CMAKE_BUILD_TYPE} PROPERTIES
                         DEPENDS mitkPluginGeneratorCreateTest
                         LABELS "MITK;BlueBerry")
  
    add_test(NAME mitkPluginGeneratorBuildTest-${CMAKE_BUILD_TYPE}
             COMMAND ${CMAKE_COMMAND} --build ${test_project_binary_dir} --config ${CMAKE_BUILD_TYPE})
    set_tests_properties(mitkPluginGeneratorBuildTest-${CMAKE_BUILD_TYPE} PROPERTIES
                         DEPENDS mitkPluginGeneratorConfigureTest-${CMAKE_BUILD_TYPE}
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
      add_test(NAME mitkPluginGeneratorPackageTest CONFIGURATIONS Release
               COMMAND ${CMAKE_COMMAND} --build ${test_project_binary_dir}/${proj}-build --config Release --target package)
      set_tests_properties(mitkPluginGeneratorPackageTest PROPERTIES
                           DEPENDS mitkPluginGeneratorBuildTest-Release
                           TIMEOUT 3600
                           LABELS "MITK;BlueBerry")
    elseif(CMAKE_BUILD_TYPE)
      add_test(mitkPluginGeneratorPackageTest
               ${CMAKE_COMMAND} --build ${test_project_binary_dir}/${proj}-build --config ${CMAKE_BUILD_TYPE} --target package)
      set_tests_properties(mitkPluginGeneratorPackageTest PROPERTIES
                           DEPENDS mitkPluginGeneratorBuildTest-${CMAKE_BUILD_TYPE}
                           TIMEOUT 3600
                           LABELS "MITK;BlueBerry")
    endif()

  endif()
  
endif()

