# - Find Ant (a java build tool)
# This module defines
# ANT_VERSION version string of ant if found
# ANT_FOUND, If false, do not try to use ant

exec_program(ant ARGS -version OUTPUT_VARIABLE ANT_VERSION
             RETURN_VALUE ANT_RETURN)

if(ANT_RETURN STREQUAL "0")
  set(ANT_FOUND TRUE)
  if(NOT ANT_FIND_QUIETLY)
    message(STATUS "Found Ant: ${ANT_VERSION}")
  endif(NOT ANT_FIND_QUIETLY)
else(ANT_RETURN STREQUAL "0")
  set(ANT_FOUND FALSE)
endif(ANT_RETURN STREQUAL "0")