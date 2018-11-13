execute_process(
  COMMAND ${CMAKE_COMMAND} -E compare_files extract-Boost.cmake extract-Boost.replacement.cmake
  RESULT_VARIABLE is_original
)

if(is_original)
  message(STATUS "Hooking into extract script")
  execute_process(COMMAND ${CMAKE_COMMAND} -E copy extract-Boost.cmake extract-Boost.original.cmake)
  execute_process(COMMAND ${CMAKE_COMMAND} -E copy extract-Boost.replacement.cmake extract-Boost.cmake)
else()
  message(STATUS "Reusing already extracted files")
endif()
