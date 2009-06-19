INCLUDE(files.cmake)

ADD_LIBRARY(TestLibrary ${LIB_MODE} src/TestPlugin.cpp src/TestLibrary.cpp)
SET_TARGET_PROPERTIES(TestLibrary PROPERTIES PREFIX "")
TARGET_LINK_LIBRARIES(TestLibrary PocoFoundation)

ADD_EXECUTABLE(PocoTestRunnerFoundation ${CPP_FILES})
TARGET_LINK_LIBRARIES(PocoTestRunnerFoundation PocoFoundation CppUnit)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../include
 ${CMAKE_CURRENT_SOURCE_DIR}/../../Foundation/include
${CMAKE_CURRENT_SOURCE_DIR}/../../CppUnit/include
)

#SET_TARGET_PROPERTIES(PocoTestRunnerFoundation PROPERTIES COMPILE_FLAGS "${POCO_UNIX_COMPILE_FLAGS}")
