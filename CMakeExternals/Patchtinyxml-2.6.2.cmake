# Called by tinyxml.cmake (ExternalProject_Add) as a patch for tinyxml.
# Adds #define TIXML_USE_STL to enable STL string support

file(MD5 tinyxml.h tinyxmlhMD5)
file(MD5 tinyxml.cpp tinyxmlcppMD5)

IF(NOT ${tinyxmlhMD5} MATCHES 7f14e12e97d4b382589b4d982549c69b)
  message( FATAL_ERROR "tinyxml.h does not match the desired hash. Patch will not be applied since tinyxml.h might have changed unexpectedly.")
ENDIF(NOT ${tinyxmlhMD5} MATCHES 7f14e12e97d4b382589b4d982549c69b)

IF(NOT ${tinyxmlcppMD5} MATCHES 6d8ae54d2c123cdce718f2e929d23b9f)
  message( FATAL_ERROR "tinyxml.cpp does not match the desired hash. Patch will not be applied since tinyxml.cpp might have changed unexpectedly.")
ENDIF(NOT ${tinyxmlcppMD5} MATCHES 6d8ae54d2c123cdce718f2e929d23b9f)


configure_file(${CMAKE_CURRENT_LIST_DIR}/Patchtinyxml-2.6.2.h tinyxml.h COPYONLY)
configure_file(${CMAKE_CURRENT_LIST_DIR}/Patchtinyxml-2.6.2.cpp tinyxml.cpp COPYONLY)


# read whole file
file(STRINGS tinyxml.h sourceCode NEWLINE_CONSUME)

# Add the TIXML_USE_STL define
string(REGEX REPLACE "#define TINYXML_INCLUDED" "#define TINYXML_INCLUDED\n\n#ifndef TIXML_USE_STL\n  #define TIXML_USE_STL\n#endif" sourceCode ${sourceCode})

# set variable CONTENTS, which is substituted in TEMPLATE_FILE
set(CONTENTS ${sourceCode})
configure_file(${CMAKE_CURRENT_LIST_DIR}/EmptyFileForPatching.dummy tinyxml.h @ONLY)

# Add the default CMake build system
include(${CMAKE_CURRENT_LIST_DIR}/GenerateDefaultCMakeBuildSystem.cmake)
