if(BUILD_TESTING)
  # currently breakpad testing is only available for windows
  # ToDo: currently a problem can occure during crash dump generation on linux. please have a look at the documentation of mitkBreakpadCrashReportingDumpTest
  #set_tests_properties(mitkBreakpadCrashReportingDumpTest PROPERTIES TIMEOUT 5) # the test should not need 5 seconds

if(CMAKE_SYSTEM MATCHES "Windows")
    MITK_CREATE_MODULE_TESTS()

  set_property(TEST mitkBreakpadCrashReportingDumpTest PROPERTY WILL_FAIL TRUE) # this one SHOULD crash
  set_property(TEST mitkBreakpadCrashReportingDumpCheckTest PROPERTY DEPENDS mitkBreakpadCrashReportingDumpTest) # this one checks if a dump was created

endif()
