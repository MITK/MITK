include(files.cmake)

add_executable(PocoTestRunnerZip ${CPP_FILES})
target_link_libraries(PocoTestRunnerZip 
 PocoUtil
 PocoZip
 PocoFoundation
 CppUnit)

if(WIN32)
  target_link_libraries(PocoTestRunnerZip
    WinTestRunner
  )
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../CppUnit/WinTestRunner/include)
  add_definitions(-D_AFXDLL)
endif(WIN32)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include
 ${CMAKE_CURRENT_SOURCE_DIR}/../../Foundation/include
 ${CMAKE_CURRENT_SOURCE_DIR}/../../XML/include
 ${CMAKE_CURRENT_SOURCE_DIR}/../../Zip/include
 ${CMAKE_CURRENT_SOURCE_DIR}/../../CppUnit/include
)

set(_zipdata_files
  data.zip
  doc.zip
  java.zip
  test.zip
)

foreach(_zipdata ${_zipdata_files})
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/data/${_zipdata}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/data/${_zipdata}" COPYONLY)
endforeach()
