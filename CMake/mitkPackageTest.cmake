if(BUILD_TESTING)

#package testing
  if(NOT MITK_FAST_TESTING)
    
    # package testing in windows only for release
    if(WIN32)
      add_test(NAME mitkPackageTest CONFIGURATIONS Release
               COMMAND ${CMAKE_COMMANDS} --build ${MITK_BINARY_DIR} --config Release --target package)

      set_tests_properties( mitkPackageest PROPERTIES
                          TIMEOUT 1800
                          LABELS "MITK")
    elseif(CMAKE_BUILD_TYPE)
      add_test( NAME mitkPackageTargetTest
                COMMAND ${CMAKE_COMMAND} --build ${MITK_BINARY_DIR} --config ${CMAKE_BUILD_TYPE} --target package)

      set_tests_properties( mitkPackageTargetTest PROPERTIES
                          TIMEOUT 1800
                          LABELS "MITK")
    endif()

  endif() # NOT MITK_FAST_TESTING

endif(BUILD_TESTING)
