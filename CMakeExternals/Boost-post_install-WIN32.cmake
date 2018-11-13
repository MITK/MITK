message(STATUS "Moving DLLs from lib to bin directory")

file(GLOB boost_dlls boost_*.dll)

execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ../bin)

foreach(boost_dll ${boost_dlls})
  execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${boost_dll} ../bin)
  execute_process(COMMAND ${CMAKE_COMMAND} -E remove ${boost_dll})
endforeach()
