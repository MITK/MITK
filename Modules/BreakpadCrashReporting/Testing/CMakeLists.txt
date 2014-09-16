if(BUILD_TESTING)
  # currently breakpad testing is only available for windows
  # ToDo: currently a problem can occure during crash dump generation on linux. please have a look at the documentation of mitkBreakpadCrashReportingDumpTest
  #set_tests_properties(mitkBreakpadCrashReportingDumpTest PROPERTIES TIMEOUT 5) # the test should not need 5 seconds

if(CMAKE_SYSTEM MATCHES "Windows")
    MITK_CREATE_MODULE_TESTS()

    add_executable(BreakpadCrashReportingDumpTestApplication mitkBreakpadCrashReportingDumpTestApplication.cpp)
    target_link_libraries(BreakpadCrashReportingDumpTestApplication ${ALL_LIBRARIES} MitkBreakpadCrashReporting)

endif()
