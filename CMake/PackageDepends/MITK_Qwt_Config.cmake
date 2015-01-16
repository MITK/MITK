# first look for the superbuild Qwt version, pointed to by Gwt_DIR
find_package(Qwt QUIET NO_MODULE)
if(NOT Qwt_FOUND)
  # now try invoking a FindQwt.cmake script
  find_package(Qwt REQUIRED)
endif()

list(APPEND ALL_LIBRARIES ${Qwt_LIBRARIES})
if(Qwt_INCLUDE_DIRS)
  list(APPEND ALL_INCLUDE_DIRECTORIES ${Qwt_INCLUDE_DIRS})
endif()
