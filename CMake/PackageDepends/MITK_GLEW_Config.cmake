# first look for the superbuild GLEW version, pointed to by GLEW_DIR
find_package(GLEW QUIET NO_MODULE)
if(NOT GLEW_FOUND)
  # now try invoking a FindGLEW.cmake script
  find_package(GLEW REQUIRED)
endif()

list(APPEND ALL_LIBRARIES ${GLEW_LIBRARIES})
list(APPEND ALL_INCLUDE_DIRECTORIES ${GLEW_INCLUDE_DIRS})


