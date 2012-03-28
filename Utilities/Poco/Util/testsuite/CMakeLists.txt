include(files.cmake)

add_executable(PocoTestRunnerUtil ${CPP_FILES})
target_link_libraries(PocoTestRunnerUtil 
 PocoUtil
 PocoXML
 PocoFoundation
 CppUnit)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include
 ${CMAKE_CURRENT_SOURCE_DIR}/../../Foundation/include
 ${CMAKE_CURRENT_SOURCE_DIR}/../../XML/include
 ${CMAKE_CURRENT_SOURCE_DIR}/../../CppUnit/include
)

if(WIN32)
  target_link_libraries(PocoTestRunnerUtil 
    WinTestRunner
  )
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../CppUnit/WinTestRunner/include)
  add_definitions(-D_AFXDLL)
endif(WIN32)

#set_target_properties(PocoTestRunnerUtil PROPERTIES COMPILE_FLAGS "${POCO_UNIX_COMPILE_FLAGS}")
