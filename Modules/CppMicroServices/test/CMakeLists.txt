# Configure usTestingConfig.h with output directories
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/usTestingConfig.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/usTestingConfig.h"
)

# Build test modules (shared libraries loaded dynamically by tests)
include(usFunctionCreateTestModule)
set(US_TEST_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})
set(_us_test_module_libs "" CACHE INTERNAL "" FORCE)
add_subdirectory(modules)

if(BUILD_TESTING AND MODULE_IS_ENABLED)
  include(files.cmake)

  set(TESTDRIVER ${MODULE_NAME}TestDriver)

  set(MITK_MODULE_NAME_REGEX_MATCH)
  set(MITK_MODULE_NAME_REGEX_NOT_MATCH)

  create_test_sourcelist(_srcs ${TESTDRIVER}.cpp ${MODULE_TESTS})
  usFunctionGenerateModuleInit(_srcs)
  usFunctionGetResourceSource(TARGET ${TESTDRIVER} OUT _srcs)

  add_executable(${TESTDRIVER} ${_srcs} ${CPP_FILES})

  target_include_directories(${TESTDRIVER} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_BINARY_DIR})

  target_compile_definitions(${TESTDRIVER} PRIVATE US_MODULE_NAME=main)
  set_property(TARGET ${TESTDRIVER} PROPERTY US_MODULE_NAME main)
  set_property(TARGET ${TESTDRIVER} PROPERTY ENABLE_EXPORTS YES)
  set_property(TARGET ${TESTDRIVER} PROPERTY FOLDER "${MITK_ROOT_FOLDER}/Modules/Tests")

  target_link_libraries(${TESTDRIVER} PRIVATE ${MODULE_NAME})

  if(LINUX)
    target_link_libraries(${TESTDRIVER} PRIVATE rt)
  endif()

  # Embed test resources and merge resources from test modules
  usFunctionEmbedResources(TARGET ${TESTDRIVER}
    FILES ${MODULE_RESOURCE_FILES}
    ZIP_ARCHIVES ${MODULE_NAME} ${_us_test_module_libs})

  foreach(_test_module ${_us_test_module_libs})
    add_dependencies(${TESTDRIVER} ${_test_module})
  endforeach()

  # Register tests
  foreach(_test ${MODULE_TESTS})
    get_filename_component(_test_name ${_test} NAME_WE)
    add_test(NAME ${_test_name} COMMAND ${TESTDRIVER} ${_test_name})
  endforeach()
endif()
