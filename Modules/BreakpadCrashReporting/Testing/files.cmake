if(CMAKE_SYSTEM MATCHES "Windows") # currently only for windows (see documentation of test mitkBreakpadCrashReportingDumpTest)
set(MODULE_TESTS
    mitkBreakpadCrashReportingDumpTest.cpp
)
endif()
