# first look for the superbuild CppUnit version, pointed to by CppUnit_DIR
find_package(CppUnit QUIET NO_MODULE)
if(NOT CppUnit_FOUND)
  # now try invoking a FindCppUnit.cmake script
  find_package(CppUnit REQUIRED)
endif()

list(APPEND ALL_LIBRARIES ${CppUnit_LIBRARIES})
list(APPEND ALL_INCLUDE_DIRECTORIES ${CppUnit_INCLUDE_DIRS})
