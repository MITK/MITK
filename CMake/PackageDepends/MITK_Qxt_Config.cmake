# first look for the superbuild Qxt version, pointed to by Qxt_DIR
find_package(Qxt QUIET NO_MODULE)
if(NOT Qxt_FOUND)
  # now try invoking a FindQxt.cmake script
  find_package(Qxt REQUIRED)
endif()

list(APPEND ALL_LIBRARIES ${Qxt_LIBRARIES})
list(APPEND ALL_INCLUDE_DIRECTORIES ${Qxt_INCLUDE_DIRS})
