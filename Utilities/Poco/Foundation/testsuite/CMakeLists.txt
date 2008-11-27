INCLUDE(files.cmake)

ADD_Executable(PocoTestRunnerFoundation ${CPP_FILES})
TARGET_LINK_LIBRARIES(PocoTestRunnerFoundation PocoFoundation debug PocoFoundationd PocoCppUnit debug PocoCppUnitd)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../include
 ${CMAKE_CURRENT_SOURCE_DIR}/../../Foundation/include
${CMAKE_CURRENT_SOURCE_DIR}/../../CppUnit/include
)

SET_TARGET_PROPERTIES(PocoTestRunnerFoundation PROPERTIES COMPILE_FLAGS "${POCO_UNIX_COMPILE_FLAGS}")