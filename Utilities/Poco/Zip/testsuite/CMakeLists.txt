INCLUDE(files.cmake)

ADD_Executable(PocoTestRunnerZip ${CPP_FILES})
TARGET_LINK_LIBRARIES(PocoTestRunnerZip 
 PocoUtil
 PocoZip
 PocoFoundation
 CppUnit)

IF (WIN32)
  TARGET_LINK_LIBRARIES(PocoTestRunnerZip
    WinTestRunner
  )
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../../CppUnit/WinTestRunner/include)
  add_definitions(-D_AFXDLL)
ENDIF(WIN32)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../include
 ${CMAKE_CURRENT_SOURCE_DIR}/../../Foundation/include
 ${CMAKE_CURRENT_SOURCE_DIR}/../../XML/include
 ${CMAKE_CURRENT_SOURCE_DIR}/../../Zip/include
 ${CMAKE_CURRENT_SOURCE_DIR}/../../CppUnit/include
)

