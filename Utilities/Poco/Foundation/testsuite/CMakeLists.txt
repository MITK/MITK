include(files.cmake)

# Additional "test data"
add_library(TestLibrary ${LIB_MODE} src/TestPlugin.cpp src/TestLibrary.cpp)
set_target_properties(TestLibrary PROPERTIES PREFIX "")
target_link_libraries(TestLibrary PocoFoundation)

add_executable(TestApp src/TestApp.cpp)
set_target_properties(TestApp PROPERTIES
                      DEBUG_OUTPUT_NAME TestAppd)


# The test runner
add_executable(PocoTestRunnerFoundation ${CPP_FILES})
target_link_libraries(PocoTestRunnerFoundation PocoFoundation CppUnit)
add_dependencies(PocoTestRunnerFoundation TestLibrary TestApp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include
 ${CMAKE_CURRENT_SOURCE_DIR}/../../Foundation/include
${CMAKE_CURRENT_SOURCE_DIR}/../../CppUnit/include
)
if(WIN32)
  target_link_libraries(PocoTestRunnerFoundation
    WinTestRunner
  )
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../CppUnit/WinTestRunner/include)
  add_definitions(-D_AFXDLL)
endif(WIN32)
#set_target_properties(PocoTestRunnerFoundation PROPERTIES COMPILE_FLAGS "${POCO_UNIX_COMPILE_FLAGS}")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/data/testdata.txt" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/data/testdata.txt" COPYONLY)
