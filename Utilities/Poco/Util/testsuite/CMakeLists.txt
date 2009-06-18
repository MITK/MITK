INCLUDE(files.cmake)

ADD_Executable(PocoTestRunnerUtil ${CPP_FILES})
TARGET_LINK_LIBRARIES(PocoTestRunnerUtil 
 PocoUtil
 PocoXML
 PocoFoundation
 CppUnit)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../include
 ${CMAKE_CURRENT_SOURCE_DIR}/../../Foundation/include
 ${CMAKE_CURRENT_SOURCE_DIR}/../../XML/include
 ${CMAKE_CURRENT_SOURCE_DIR}/../../CppUnit/include
)

#SET_TARGET_PROPERTIES(PocoTestRunnerUtil PROPERTIES COMPILE_FLAGS "${POCO_UNIX_COMPILE_FLAGS}")
