# - Find Ant (a java build tool)
# This module defines
# ANT_VERSION version string of ant if found
# ANT_FOUND, If false, do not try to use ant
 
EXEC_PROGRAM(ant ARGS -version OUTPUT_VARIABLE ANT_VERSION
             RETURN_VALUE ANT_RETURN)
 
IF (ANT_RETURN STREQUAL "0")
  SET(ANT_FOUND TRUE)
  IF (NOT ANT_FIND_QUIETLY)
    MESSAGE(STATUS "Found Ant: ${ANT_VERSION}")
  ENDIF (NOT ANT_FIND_QUIETLY)
ELSE(ANT_RETURN STREQUAL "0")
  SET(ANT_FOUND FALSE)
ENDIF (ANT_RETURN STREQUAL "0")