# Called by DCMTK.cmake (ExternalProject_Add) as a patch for DCMTK.
# Changes DCMTK_BUILD_DATE so that superbuild will work with Xcode

# read whole file
file(STRINGS CMake/dcmtkPrepare.cmake sourceCode NEWLINE_CONSUME)

# Changing the way DCMTK_BUILD_DATE is set in CMakeLists.txt
# This way it is not a compiler flag but set in the dcmtk config file
string(REGEX REPLACE "ADD_DEFINITIONS[(]\"-DDCMTK_BUILD_DATE" "set(DCMTK_BUILD_DATE \"\\\\\"2012-11-02\\\\\"\")\n#ADD_DEFINITIONS[(]\"-DDCMTK_BUILD_DATE" sourceCode ${sourceCode})

# set variable CONTENTS, which is substituted in TEMPLATE_FILE
set(CONTENTS ${sourceCode})
configure_file(${TEMPLATE_FILE} CMake/dcmtkPrepare.cmake @ONLY)

# read whole file
file(STRINGS CMake/osconfig.h.in sourceCode2 NEWLINE_CONSUME)

# Add DCMTK_BUILD_DATE to osconfig.h.in
string(REGEX REPLACE "\"@DCMTK_PACKAGE_DATE@\"" "\"@DCMTK_PACKAGE_DATE@\"\n\n#define DCMTK_BUILD_DATE @DCMTK_BUILD_DATE@\n" sourceCode2 ${sourceCode2})

# set variable CONTENTS, which is substituted in TEMPLATE_FILE
set(CONTENTS ${sourceCode2})
configure_file(${TEMPLATE_FILE} CMake/osconfig.h.in @ONLY)
