if(BUILD_TESTING)
  MITK_CREATE_MODULE_TESTS()

  #set_tests_properties(mitkBreakpadCrashReportingDumpTest PROPERTIES WILL_FAIL TRUE) # this one SHOULD crash
  if(CMAKE_SYSTEM MATCHES "Windows")
    add_executable(BreakpadCrashReportingDumpTestApplication mitkBreakpadCrashReportingDumpTestApplication.cpp)
    target_link_libraries(BreakpadCrashReportingDumpTestApplication ${ALL_LIBRARIES} BreakpadCrashReporting)
  endif()

  set_property(TEST mitkBreakpadCrashReportingDumpCheckTest PROPERTY DEPENDS mitkBreakpadCrashReportingDumpTest) # this one checks if a dump was created

endif()
