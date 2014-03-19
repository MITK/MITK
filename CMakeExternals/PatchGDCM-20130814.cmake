# Called by GDCM.cmake (ExternalProject_Add) as a patch for GDCM.
# Related bug: http://bugs.mitk.org/show_bug.cgi?id=15800

# read whole file
file(STRINGS Source/MediaStorageAndFileFormat/gdcmSorter.cxx sourceCode NEWLINE_CONSUME)

# Changing the way gdcmSorter.cxx behaves when encountering an unreadable file
string(REPLACE "f = NULL;" "std::cerr << \"Err: File could not be read: \" << it->c_str() << std::endl; \n      return false;" sourceCode ${sourceCode})

# set variable CONTENTS, which is substituted in TEMPLATE_FILE
set(CONTENTS ${sourceCode})
configure_file(${TEMPLATE_FILE} Source/MediaStorageAndFileFormat/gdcmSorter.cxx @ONLY)


# fix issues with whitespace in source directories
file(STRINGS CMake/ExportConfiguration/GDCMConfig.cmake.in sourceCode NEWLINE_CONSUME)
string(REPLACE "set( GDCM_INCLUDE_DIRS @GDCM_INCLUDE_PATH@)" "set( GDCM_INCLUDE_DIRS \"@GDCM_INCLUDE_PATH@\")" sourceCode ${sourceCode})
string(REPLACE "set(GDCM_LIBRARY_DIRS @GDCM_LIBRARY_DIR@)" "set(GDCM_LIBRARY_DIRS \"@GDCM_LIBRARY_DIR@\")" sourceCode ${sourceCode})
SET(GDCM_LIBRARY_DIRS @GDCM_LIBRARY_DIR@)
# set variable CONTENTS, which is substituted in TEMPLATE_FILE
set(CONTENTS ${sourceCode})
configure_file(${TEMPLATE_FILE} CMake/ExportConfiguration/GDCMConfig.cmake.in @ONLY)
