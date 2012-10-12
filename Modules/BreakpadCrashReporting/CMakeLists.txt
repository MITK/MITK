OPTION(MITK_USE_BREAKPAD_CRASH_REPORTING "Enable support for Google Breakpad Crash Reporting" ON)


if(MITK_USE_BREAKPAD_CRASH_REPORTING)

  if(CMAKE_SYSTEM MATCHES "Windows") 
  
    FIND_PATH(MITK_BREAKPAD_SRC_DIR breakpad_googletest_includes.h ${MITK_BREAKPAD_SRC_DIR} DOC  "Directory breakpad/src/")
    #FIND_PATH(MITK_PMD_INCLUDE_DIR pmdsdk2.h ${MITK_PMD_SDK_DIR}/include DOC  "Include directory of PMD-SDK.")
    SET(INCLUDE_DIRS_INTERNAL ${INCLUDE_DIRS_INTERNAL} ${MITK_BREAKPAD_SRC_DIR})
    SET(ADDITIONAL_LIBS ${ADDITIONAL_LIBS} ${MITK_BREAKPAD_SRC_DIR}/client/windows/Release/lib/common.lib)
    SET(ADDITIONAL_LIBS ${ADDITIONAL_LIBS} ${MITK_BREAKPAD_SRC_DIR}/client/windows/Release/lib/crash_generation_client.lib)
    SET(ADDITIONAL_LIBS ${ADDITIONAL_LIBS} ${MITK_BREAKPAD_SRC_DIR}/client/windows/Release/lib/crash_generation_server.lib)
    SET(ADDITIONAL_LIBS ${ADDITIONAL_LIBS} ${MITK_BREAKPAD_SRC_DIR}/client/windows/Release/lib/exception_handler.lib)
    SET(ADDITIONAL_LIBS ${ADDITIONAL_LIBS} ${MITK_BREAKPAD_SRC_DIR}/client/windows/Release/lib/crash_report_sender.lib)
    
    message(INFORMATION "Building BreakpadCrashReporting module.")

    MITK_CREATE_MODULE(BreakpadCrashReporting
    INCLUDE_DIRS ${MITK_BIN_DIR}
    INTERNAL_INCLUDE_DIRS ${INCLUDE_DIRS_INTERNAL}
    DEPENDS Mitk Qmitk
    EXPORT_DEFINE MITK_BREAKPAD_EXPORT
    ADDITIONAL_LIBS ${ADDITIONAL_LIBS}
    QT_MODULE
    )
  
  endif()

endif(MITK_USE_BREAKPAD_CRASH_REPORTING)