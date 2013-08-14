# Called by GDCM.cmake (ExternalProject_Add) as a patch for GDCM.
# Related bug: http://bugs.mitk.org/show_bug.cgi?id=15800

# read whole file
file(STRINGS Source/MediaStorageAndFileFormat/gdcmSorter.cxx sourceCode NEWLINE_CONSUME)

# Changing the way gdcmSorter.cxx behaves when encountering an unreadable file
string(REPLACE "f = NULL;" "std::cerr << \"Err: File could not be read: \" << it->c_str() << std::endl; \n      return false;" sourceCode ${sourceCode})

# set variable CONTENTS, which is substituted in TEMPLATE_FILE
set(CONTENTS ${sourceCode})
configure_file(${TEMPLATE_FILE} Source/MediaStorageAndFileFormat/gdcmSorter.cxx @ONLY)
