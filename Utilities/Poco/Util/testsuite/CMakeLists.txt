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

IF (WIN32)
  TARGET_LINK_LIBRARIES(PocoTestRunnerUtil 
    WinTestRunner
  )
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../../CppUnit/WinTestRunner/include)
  add_definitions(-D_AFXDLL)
ENDIF(WIN32)

#SET_TARGET_PROPERTIES(PocoTestRunnerUtil PROPERTIES COMPILE_FLAGS "${POCO_UNIX_COMPILE_FLAGS}")
