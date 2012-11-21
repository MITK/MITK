OPTION(MITK_USE_BREAKPAD_CRASH_REPORTING "Enable support for Google Breakpad Crash Reporting" OFF)


if(MITK_USE_BREAKPAD_CRASH_REPORTING)

  if(CMAKE_SYSTEM MATCHES "Windows")

    FIND_PATH(MITK_BREAKPAD_SRC_DIR breakpad_googletest_includes.h ${MITK_BREAKPAD_SRC_DIR} DOC  "Directory breakpad/src/")

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


    if(WIN32) # crash report server without console window

     # how to create main.moc ??
     #QT4_WRAP_CPP(SERVER_MOC_CPP mitkCrashReportingServer.cpp)

     #use this for windows console:
     # add_executable(CrashReportingServer mitkCrashReportingServer.cpp ${SERVER_MOC_CPP})
     add_executable(CrashReportingServer WIN32 mitkCrashReportingServer.cpp ${SERVER_MOC_CPP})

     target_link_libraries(CrashReportingServer ${ALL_LIBRARIES} ${QT_QTMAIN_LIBRARY} BreakpadCrashReporting)

    endif()

  endif()

endif(MITK_USE_BREAKPAD_CRASH_REPORTING)
