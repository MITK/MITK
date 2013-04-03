option(MITK_USE_BREAKPAD_CRASH_REPORTING "Enable support for Google Breakpad Crash Reporting" OFF)


if(MITK_USE_BREAKPAD_CRASH_REPORTING)

  if(CMAKE_SYSTEM MATCHES "Windows")

    find_path(MITK_BREAKPAD_SRC_DIR breakpad_googletest_includes.h ${MITK_BREAKPAD_SRC_DIR} DOC  "Directory breakpad/src/")

    set(INCLUDE_DIRS_INTERNAL ${INCLUDE_DIRS_INTERNAL} ${MITK_BREAKPAD_SRC_DIR})
    set(ADDITIONAL_LIBS ${ADDITIONAL_LIBS}
      ${MITK_BREAKPAD_SRC_DIR}/client/windows/Release/lib/common.lib # ??
      ${MITK_BREAKPAD_SRC_DIR}/client/windows/Release/lib/crash_generation_client.lib # ???
      ${MITK_BREAKPAD_SRC_DIR}/client/windows/Release/lib/crash_generation_server.lib # out-of-process dump generator
      ${MITK_BREAKPAD_SRC_DIR}/client/windows/Release/lib/exception_handler.lib # dump creator (ExceptionHandler)
      ${MITK_BREAKPAD_SRC_DIR}/client/windows/Release/lib/crash_report_sender.lib
      ) # ??? unused, I guess

    set(EXECUTABLE_PROPERTY WIN32)
  elseif(CMAKE_SYSTEM MATCHES "Linux")
    find_path(MITK_BREAKPAD_SRC_DIR breakpad_googletest_includes.h ${MITK_BREAKPAD_SRC_DIR} DOC  "Directory breakpad/src/")

    set(INCLUDE_DIRS_INTERNAL ${INCLUDE_DIRS_INTERNAL} ${MITK_BREAKPAD_SRC_DIR}) # TODO why the repetition, is INCLUDE_DIRS_INTERNAL not empty before??
    set(ADDITIONAL_LIBS ${ADDITIONAL_LIBS}
      ${MITK_BREAKPAD_SRC_DIR}/client/linux/libbreakpad_client.a
      )
  else()
    message(FATAL_ERROR "Unsupported platform for Breakpad crash reporting: ${CMAKE_SYSTEM}")
  endif()

  message(STATUS "Building BreakpadCrashReporting module.")
  message(STATUS "  Including ${INCLUDE_DIRS_INTERNAL}")
  message(STATUS "  Linking ${ADDITIONAL_LIBS}")

  MITK_CREATE_MODULE(BreakpadCrashReporting
    INCLUDE_DIRS ${MITK_BIN_DIR}
    INTERNAL_INCLUDE_DIRS ${INCLUDE_DIRS_INTERNAL}
    DEPENDS Mitk Qmitk
    EXPORT_DEFINE MITK_BREAKPAD_EXPORT
    ADDITIONAL_LIBS ${ADDITIONAL_LIBS}
    QT_MODULE
    )

  if(CMAKE_SYSTEM MATCHES "Windows")
    add_executable(CrashReportingServer ${EXECUTABLE_PROPERTY} mitkCrashReportingServer.cpp ${SERVER_MOC_CPP})

    target_link_libraries(CrashReportingServer ${ALL_LIBRARIES} ${QT_QTMAIN_LIBRARY} BreakpadCrashReporting)
  endif()

add_subdirectory(Testing)

endif(MITK_USE_BREAKPAD_CRASH_REPORTING)
