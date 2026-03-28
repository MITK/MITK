#! Create a CppMicroServices test module (shared library with module init).
#!
#! \param name The module/target name
#!
#! Usage:
#!   usFunctionCreateTestModule(TestModuleA usTestModuleA.cpp)
#!
#!   usFunctionCreateTestModuleWithResources(TestModuleR
#!     SOURCES usTestModuleR.cpp
#!     RESOURCES foo.txt icons/logo.png
#!     BINARY_RESOURCES generated.txt
#!     RESOURCES_ROOT resources
#!     LINK_LIBRARIES SomeLib
#!     LINK_RESOURCES | APPEND_RESOURCES
#!     SKIP_MODULE_LIST
#!   )

macro(_us_create_test_module_helper)
  add_library(${name} ${_srcs})

  target_compile_definitions(${name} PRIVATE US_MODULE_NAME=${name})
  set_property(TARGET ${name} PROPERTY US_MODULE_NAME ${name})
  set_property(TARGET ${name} PROPERTY FOLDER "${MITK_ROOT_FOLDER}/Modules/Tests")
  set_property(TARGET ${name} PROPERTY POSITION_INDEPENDENT_CODE TRUE)

  target_link_libraries(${name} MitkCppMicroServices ${US_TEST_LINK_LIBRARIES})

  if(US_TEST_INCLUDE_DIRS)
    target_include_directories(${name} PRIVATE ${US_TEST_INCLUDE_DIRS})
  endif()

  if(_res_files OR US_TEST_LINK_LIBRARIES)
    usFunctionAddResources(TARGET ${name} WORKING_DIRECTORY ${_res_root}
                           FILES ${_res_files}
                           ZIP_ARCHIVES ${US_TEST_LINK_LIBRARIES})
  endif()
  if(_bin_res_files)
    usFunctionAddResources(TARGET ${name} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/resources
                           FILES ${_bin_res_files})
  endif()

  usFunctionEmbedResources(TARGET ${name} ${_mode})

  if(NOT US_TEST_SKIP_MODULE_LIST)
    set(_us_test_module_libs "${_us_test_module_libs};${name}" CACHE INTERNAL "" FORCE)
  endif()
endmacro()

function(usFunctionCreateTestModule name)
  set(_srcs ${ARGN})
  set(_res_files)
  set(_bin_res_files)
  set(_res_root ${CMAKE_CURRENT_SOURCE_DIR}/resources)
  set(_mode)
  usFunctionGenerateModuleInit(_srcs)
  _us_create_test_module_helper()
endfunction()

function(usFunctionCreateTestModuleWithResources name)
  cmake_parse_arguments(US_TEST
    "SKIP_MODULE_LIST;LINK_RESOURCES;APPEND_RESOURCES"
    "RESOURCES_ROOT"
    "SOURCES;RESOURCES;BINARY_RESOURCES;LINK_LIBRARIES"
    ${ARGN})

  set(_mode)
  if(US_TEST_LINK_RESOURCES)
    set(_mode LINK)
  elseif(US_TEST_APPEND_RESOURCES)
    set(_mode APPEND)
  endif()

  set(_srcs ${US_TEST_SOURCES})
  usFunctionGetResourceSource(TARGET ${name} OUT _srcs ${_mode})
  set(_res_files ${US_TEST_RESOURCES})
  set(_bin_res_files ${US_TEST_BINARY_RESOURCES})
  if(US_TEST_RESOURCES_ROOT)
    set(_res_root ${US_TEST_RESOURCES_ROOT})
  else()
    set(_res_root ${CMAKE_CURRENT_SOURCE_DIR}/resources)
  endif()
  usFunctionGenerateModuleInit(_srcs)
  _us_create_test_module_helper()
endfunction()
