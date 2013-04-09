# TODOs
#  - nicer separation of Linux/Window/.. code
#  - test should check existence of dump file
#  - find nice script to use linux symbol writer tool dump_.. on all relevant libraries
#  - update documentation
#  - use the same library structure in "our" cmake script as google uses in their build
#    - otherwise we cannot switch between custom-built versions of breakpad and our superbuild version


if(MITK_USE_BREAKPAD)

  message(STATUS "Get directory hint for Breakpad:")
  message(STATUS ${Breakpad_DIR})
  message(STATUS ${Breakpad_SRC})
  # TODO this should be properly moved to some BreakpadConfig.cmake
  find_path(MITK_BREAKPAD_SRC_DIR breakpad_googletest_includes.h DOC  "Directory breakpad/src/" PATHS ${Breakpad_SRC}/src ${Breakpad_DIR})
  message(STATUS "Found Breakpad at:")
  message(STATUS ${MITK_BREAKPAD_SRC_DIR})
  if(CMAKE_SYSTEM MATCHES "Windows")

    set(INCLUDE_DIRS_INTERNAL ${INCLUDE_DIRS_INTERNAL} ${MITK_BREAKPAD_SRC_DIR})
    set(ADDITIONAL_LIBS ${ADDITIONAL_LIBS}
      ${Breakpad_DIR}/Debug/breakpad_client.lib
#      ${Breakpad_DIR}/Release/common.lib # ??
#      ${Breakpad_DIR}/Release/crash_generation_client.lib # ???
#      ${Breakpad_DIR}/Release/crash_generation_server.lib # out-of-process dump generator
#      ${Breakpad_DIR}/Release/exception_handler.lib # dump creator (ExceptionHandler)
#      ${Breakpad_DIR}/Release/crash_report_sender.lib
      ) # ??? unused, I guess

    set(EXECUTABLE_PROPERTY WIN32)
  elseif(CMAKE_SYSTEM MATCHES "Linux")

    set(INCLUDE_DIRS_INTERNAL ${INCLUDE_DIRS_INTERNAL} ${MITK_BREAKPAD_SRC_DIR}) # TODO why the repetition, is INCLUDE_DIRS_INTERNAL not empty before??
    set(ADDITIONAL_LIBS ${ADDITIONAL_LIBS}
      ${Breakpad_DIR}/libbreakpad_client.a
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

endif(MITK_USE_BREAKPAD)
