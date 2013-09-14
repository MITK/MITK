# Called by Poco.cmake (ExternalProject_Add) as a patch for Poco.
# Fixes the CMake build system

# read CMakeLists.txt
file(STRINGS CMakeLists.txt sourceCode NEWLINE_CONSUME)

# Fix CMAKE_DEBUG_POSTFIX, see see https://github.com/pocoproject/poco/issues/279
string(REPLACE
"if (CMAKE_BUILD_TYPE STREQUAL \"Debug\")
  message(STATUS \"Debug output enabled\")
  set(CMAKE_DEBUG_POSTFIX \"d\" CACHE STRING \"Set debug library postfix\" FORCE)
else ()
  message(STATUS \"Optimized output enabled\")
  set(CMAKE_DEBUG_POSTFIX \"\" CACHE STRING \"Set debug library postfix\" FORCE)
endif ()"
       "set(CMAKE_DEBUG_POSTFIX \"d\")"
       sourceCode ${sourceCode}
      )

# set variable CONTENTS, which is substituted in TEMPLATE_FILE
set(CONTENTS ${sourceCode})
configure_file(${CMAKE_CURRENT_LIST_DIR}/EmptyFileForPatching.dummy CMakeLists.txt @ONLY)

# Fix check for ODBC, see https://github.com/pocoproject/poco/issues/285
file(STRINGS cmake/FindODBC.cmake sourceCode NEWLINE_CONSUME)

# Check if header files exist
string(REPLACE

"exec_program(\${UNIX_ODBC_CONFIG} ARGS \"--include-prefix\" OUTPUT_VARIABLE ODBC_INCLUDE_DIR)
\tinclude_directories(\${ODBC_INCLUDE_DIR})
\texec_program(\${UNIX_ODBC_CONFIG} ARGS \"--libs\" OUTPUT_VARIABLE ODBC_LINK_FLAGS)
\tadd_definitions(-DPOCO_UNIXODBC)"

"exec_program(\${UNIX_ODBC_CONFIG} ARGS \"--include-prefix\" OUTPUT_VARIABLE ODBC_INCLUDE_DIR)
\tif(EXISTS \"\${ODBC_INCLUDE_DIR}/sqlext.h\")
\t\tinclude_directories(\${ODBC_INCLUDE_DIR})
\t\texec_program(\${UNIX_ODBC_CONFIG} ARGS \"--libs\" OUTPUT_VARIABLE ODBC_LINK_FLAGS)
\t\tadd_definitions(-DPOCO_UNIXODBC)
\telse()
\t\tset(UNIX_ODBC_CONFIG \"UNIX_ODBC_CONFIG-NOTFOUND\")
\tendif()"

  sourceCode ${sourceCode})

# set variable CONTENTS, which is substituted in TEMPLATE_FILE
set(CONTENTS ${sourceCode})
configure_file(${CMAKE_CURRENT_LIST_DIR}/EmptyFileForPatching.dummy cmake/FindODBC.cmake @ONLY)


# For the next fixes, see https://github.com/pocoproject/poco/issues/274

# read whole Foundation/CMakeLists.txt
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
