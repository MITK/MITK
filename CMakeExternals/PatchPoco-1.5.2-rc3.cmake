# Called by Poco.cmake (ExternalProject_Add) as a patch for Poco.
# Adds #define TIXML_USE_STL to enable STL string support

# read whole file
file(STRINGS Foundation/CMakeLists.txt sourceCode NEWLINE_CONSUME)

# Remove files
foreach(_file gzclose.c gzlib.c gzread.c gzwrite.c uncompr.c)
  string(REGEX REPLACE "src/${_file}" "" sourceCode ${sourceCode})
endforeach()

# set variable CONTENTS, which is substituted in TEMPLATE_FILE
set(CONTENTS ${sourceCode})
configure_file(${CMAKE_CURRENT_LIST_DIR}/EmptyFileForPatching.dummy Foundation/CMakeLists.txt @ONLY)


# read whole file
file(STRINGS MongoDB/CMakeLists.txt sourceCode NEWLINE_CONSUME)

# Fix define
string(REGEX REPLACE "-DMONGODB_EXPORTS" "-DMongoDB_EXPORTS" sourceCode ${sourceCode})

# set variable CONTENTS, which is substituted in TEMPLATE_FILE
set(CONTENTS ${sourceCode})
configure_file(${CMAKE_CURRENT_LIST_DIR}/EmptyFileForPatching.dummy MongoDB/CMakeLists.txt @ONLY)


# read whole file
file(STRINGS Net/CMakeLists.txt sourceCode NEWLINE_CONSUME)

# Add missing source file entry
string(REGEX REPLACE "src/NameValueCollection.cpp" "src/NameValueCollection.cpp\n  src/Net.cpp" sourceCode ${sourceCode})

# set variable CONTENTS, which is substituted in TEMPLATE_FILE
set(CONTENTS ${sourceCode})
configure_file(${CMAKE_CURRENT_LIST_DIR}/EmptyFileForPatching.dummy Net/CMakeLists.txt @ONLY)
