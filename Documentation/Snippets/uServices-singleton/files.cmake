set(_init_file ${CMAKE_CURRENT_BINARY_DIR}/uServices_singleton_init.cpp)

set(MODULE_NAME "uServices_singleton")
set(MODULE_LIBNAME "")
set(MODULE_DEPENDS_STR "Mitk")
set(MODULE_PACKAGE_DEPENDS_STR)
set(MODULE_VERSION)
set(MODULE_QT_BOOL "false")
configure_file(${MITK_SOURCE_DIR}/CMake/mitkModuleInit.cpp ${_init_file} @ONLY)

set(snippet_src_files
  main.cpp
  SingletonOne.cpp
  SingletonTwo.cpp
  ${_init_file}
)
