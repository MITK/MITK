# Called by CppUnit.cmake (ExternalProject_Add) as a patch for CppUnit.
# Copies the file CppUnitconfig.h.cmake to config/config.h.cmake
# and adds a missing include statement

configure_file(${CMAKE_CURRENT_LIST_DIR}/${proj}config.h.cmake  config/config.h.cmake COPYONLY)

# read whole file
file(STRINGS src/cppunit/TypeInfoHelper.cpp sourceCode NEWLINE_CONSUME)

# Add the stdlib.h include
string(REGEX REPLACE "#include <cxxabi.h>" "#include <cxxabi.h>\n#include <stdlib.h>" sourceCode ${sourceCode})

# set variable CONTENTS, which is substituted in TEMPLATE_FILE
set(CONTENTS ${sourceCode})
configure_file(${CMAKE_CURRENT_LIST_DIR}/EmptyFileForPatching.dummy src/cppunit/TypeInfoHelper.cpp @ONLY)

# Add the default CMake build system
include(${CMAKE_CURRENT_LIST_DIR}/GenerateDefaultCMakeBuildSystem.cmake)
