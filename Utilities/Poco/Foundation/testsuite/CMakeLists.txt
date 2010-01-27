INCLUDE(files.cmake)

# Additional "test data"
ADD_LIBRARY(TestLibrary ${LIB_MODE} src/TestPlugin.cpp src/TestLibrary.cpp)
SET_TARGET_PROPERTIES(TestLibrary PROPERTIES PREFIX "")
TARGET_LINK_LIBRARIES(TestLibrary PocoFoundation)

ADD_EXECUTABLE(TestApp src/TestApp.cpp)
SET_TARGET_PROPERTIES(TestApp PROPERTIES
                      DEBUG_OUTPUT_NAME TestAppd)


# The test runner
ADD_EXECUTABLE(PocoTestRunnerFoundation ${CPP_FILES})
TARGET_LINK_LIBRARIES(PocoTestRunnerFoundation PocoFoundation CppUnit)
ADD_DEPENDENCIES(PocoTestRunnerFoundation TestLibrary TestApp)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../include
 ${CMAKE_CURRENT_SOURCE_DIR}/../../Foundation/include
${CMAKE_CURRENT_SOURCE_DIR}/../../CppUnit/include
)
IF (WIN32)
  TARGET_LINK_LIBRARIES(PocoTestRunnerFoundation
    WinTestRunner
  )
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../../CppUnit/WinTestRunner/include)
  add_definitions(-D_AFXDLL)
ENDIF(WIN32)
#SET_TARGET_PROPERTIES(PocoTestRunnerFoundation PROPERTIES COMPILE_FLAGS "${POCO_UNIX_COMPILE_FLAGS}")

CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/data/testdata.txt" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/data/testdata.txt" COPYONLY)
