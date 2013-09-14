
macro(_us_create_test_module_helper)

  if(_res_files)
    usFunctionEmbedResources(_srcs LIBRARY_NAME ${name} ROOT_DIR ${_res_root} FILES ${_res_files})
  endif()

  add_library(${name} ${_srcs})
  if(NOT US_BUILD_SHARED_LIBS)
    set_property(TARGET ${name} APPEND PROPERTY COMPILE_DEFINITIONS US_STATIC_MODULE)
  endif()

  if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    get_property(_compile_flags TARGET ${name} PROPERTY COMPILE_FLAGS)
    set_property(TARGET ${name} PROPERTY COMPILE_FLAGS "${_compile_flags} -fPIC")
  endif()

  target_link_libraries(${name} ${US_LIBRARY_TARGET} ${US_LINK_LIBRARIES})

  set(_us_test_module_libs "${_us_test_module_libs};${name}" CACHE INTERNAL "" FORCE)

endmacro()

function(usFunctionCreateTestModule name)
  set(_srcs ${ARGN})
  set(_res_files )
  usFunctionGenerateModuleInit(_srcs NAME "${name} Module" LIBRARY_NAME ${name})
  _us_create_test_module_helper()
endfunction()

function(usFunctionCreateTestModuleWithResources name)
  cmake_parse_arguments(US_TEST "" "RESOURCES_ROOT" "SOURCES;RESOURCES" "" ${ARGN})
  set(_srcs ${US_TEST_SOURCES})
  set(_res_files ${US_TEST_RESOURCES})
  if(US_TEST_RESOURCES_ROOT)
    set(_res_root ${US_TEST_RESOURCES_ROOT})
  else()
    set(_res_root resources)
  endif()
  usFunctionGenerateModuleInit(_srcs NAME "${name} Module" LIBRARY_NAME ${name})
  _us_create_test_module_helper()
endfunction()
