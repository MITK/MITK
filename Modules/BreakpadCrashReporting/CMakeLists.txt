# TODOs
#  LATER nicer separation of Linux/Window/.. code
#  OK test should check existence of dump file
#  - find nice script to use linux symbol writer tool dump_.. on all relevant libraries
#  - update documentation
#  OK check buildtype (O2/-g/..) in cmake
#  - use the same library structure in "our" cmake script as google uses in their build
#    - otherwise we cannot switch between custom-built versions of breakpad and our superbuild version

if(MITK_USE_BREAKPAD)

  find_package(Breakpad)

  if(CMAKE_SYSTEM MATCHES "Windows")
    set(ADDITIONAL_LIBS ${ADDITIONAL_LIBS}
      ${Breakpad_DIR}/Debug/breakpad_client.lib
#      ${Breakpad_DIR}/Release/common.lib # ??
#      ${Breakpad_DIR}/Release/crash_generation_client.lib # ???
#      ${Breakpad_DIR}/Release/crash_generation_server.lib # out-of-process dump generator
#      ${Breakpad_DIR}/Release/exception_handler.lib # dump creator (ExceptionHandler)
#      ${Breakpad_DIR}/Release/crash_report_sender.lib
      ) # ??? unused, I guess

    set(EXECUTABLE_PROPERTY WIN32)

  endif()

  MITK_CREATE_MODULE(BreakpadCrashReporting
    INTERNAL_INCLUDE_DIRS ${Breakpad_INCLUDE_DIR}
    EXPORT_DEFINE MITK_BREAKPAD_EXPORT
    ADDITIONAL_LIBS ${Breakpad_LIBRARIES}
    PACKAGE_DEPENDS QT
    )

  if(CMAKE_SYSTEM MATCHES "Windows")
    add_executable(CrashReportingServer ${EXECUTABLE_PROPERTY} mitkCrashReportingServer.cpp ${SERVER_MOC_CPP})

    target_link_libraries(CrashReportingServer ${ALL_LIBRARIES} ${QT_QTMAIN_LIBRARY} BreakpadCrashReporting)
  endif()

add_subdirectory(Testing)

endif(MITK_USE_BREAKPAD)
