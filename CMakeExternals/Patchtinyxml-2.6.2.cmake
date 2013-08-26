# Called by tinyxml.cmake (ExternalProject_Add) as a patch for tinyxml.
# Adds #define TIXML_USE_STL to enable STL string support

# read whole file
file(STRINGS tinyxml.h sourceCode NEWLINE_CONSUME)

# Add the TIXML_USE_STL define
string(REGEX REPLACE "#define TINYXML_INCLUDED" "#define TINYXML_INCLUDED\n\n#ifndef TIXML_USE_STL\n  #define TIXML_USE_STL\n#endif" sourceCode ${sourceCode})

# set variable CONTENTS, which is substituted in TEMPLATE_FILE
set(CONTENTS ${sourceCode})
configure_file(${CMAKE_CURRENT_LIST_DIR}/EmptyFileForPatching.dummy tinyxml.h @ONLY)

# Add the default CMake build system
include(${CMAKE_CURRENT_LIST_DIR}/GenerateDefaultCMakeBuildSystem.cmake)
