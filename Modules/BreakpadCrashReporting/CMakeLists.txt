# TODOs
#  LATER nicer separation of Linux/Window/.. code
#  OK test should check existence of dump file
#  - find nice script to use linux symbol writer tool dump_.. on all relevant libraries
#  - update documentation
#  OK check buildtype (O2/-g/..) in cmake
#  OK use the same library structure in "our" cmake script as google uses in their build
#    OK otherwise we cannot switch between custom-built versions of breakpad and our superbuild version
#  [optional] add install step to our Breakpad build

if(MITK_USE_BREAKPAD) # from top-level CMakeLists.txt

  find_package(Breakpad)
  if (NOT Breakpad_FOUND)
    message(FATAL_ERROR "MITK_USE_BREAKPAD was set but Breakpad build cannot be found. Plaese check CMake cache variables regarding Breakpad")
  endif()

  MITK_CREATE_MODULE(BreakpadCrashReporting
    DEPENDS MitkCore
    INTERNAL_INCLUDE_DIRS ${Breakpad_INCLUDE_DIR}
    ADDITIONAL_LIBS ${Breakpad_LIBRARIES}
    FORCE_STATIC
    )

  if(CMAKE_SYSTEM MATCHES "Windows")
    add_executable(CrashReportingServer mitkCrashReportingServer.cpp)
    target_link_libraries(CrashReportingServer ${ALL_LIBRARIES} BreakpadCrashReporting)
    MITK_INSTALL(TARGETS CrashReportingServer)
  endif()

  if(CMAKE_SYSTEM MATCHES "Linux")
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ProduceBreakpadSymbols.sh.in ${MITK_CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ProduceBreakpadSymbols.sh)
  endif()

add_subdirectory(Testing)

endif(MITK_USE_BREAKPAD)
