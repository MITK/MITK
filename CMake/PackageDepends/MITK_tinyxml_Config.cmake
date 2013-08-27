# first look for the superbuild tinyxml version, pointed to by tinyxml_DIR
find_package(tinyxml QUIET NO_MODULE)
if(NOT tinyxml_FOUND)
  # now try invoking a Findtinyxml.cmake script
  find_package(tinyxml REQUIRED)
endif()

list(APPEND ALL_LIBRARIES ${tinyxml_LIBRARIES})
list(APPEND ALL_INCLUDE_DIRECTORIES ${tinyxml_INCLUDE_DIRS})
