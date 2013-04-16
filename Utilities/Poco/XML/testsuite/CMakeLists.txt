include(files.cmake)

add_executable(PocoTestRunnerXML ${CPP_FILES})
target_link_libraries(PocoTestRunnerXML PocoXML PocoFoundation CppUnit)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include
 ${CMAKE_CURRENT_SOURCE_DIR}/../../Foundation/include
 ${CMAKE_CURRENT_SOURCE_DIR}/../../XML/include
 ${CMAKE_CURRENT_SOURCE_DIR}/../../CppUnit/include
)

if(WIN32)
  target_link_libraries(PocoTestRunnerXML
    WinTestRunner
  )
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../CppUnit/WinTestRunner/include)
  add_definitions(-D_AFXDLL)
endif(WIN32)

#set_target_properties(PocoTestRunnerXML PROPERTIES COMPILE_FLAGS "${POCO_UNIX_COMPILE_FLAGS}")
