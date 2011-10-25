# Called by GDCM.cmake (ExternalProject_Add) as a patch for GDCM.
# Changes CMake files to properly export link directories
# (applies a change that is part of post-2.0.18 git master of gdcm)

# read whole file
file(STRINGS CMake/ExportConfiguration/GDCMConfig.cmake.in sourceCode NEWLINE_CONSUME)

set(sourceCode "${sourceCode}\nSET(GDCM_LIBRARY_DIRS \"@GDCM_LIBRARY_DIRS@\")")

# set variable CONTENTS, which is substituted in TEMPLATE_FILE
set(CONTENTS ${sourceCode})
configure_file(${TEMPLATE_FILE} CMake/ExportConfiguration/GDCMConfig.cmake.in @ONLY)

