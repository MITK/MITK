# Called by ITK.cmake (ExternalProject_Add) as a patch for ITK to work witk GDCM 2.0.18
# Updates library dependencies

# read whole file
file(STRINGS CMakeLists.txt sourceCode NEWLINE_CONSUME)

# substitute dependency to gdcmMSFF by dependencies for more libraries
string(REGEX REPLACE "gdcmMSFF" "gdcmMSFF gdcmDICT gdcmCommon gdcmDSED" sourceCode ${sourceCode})

# set variable CONTENTS, which is substituted in TEMPLATE_FILE
set(CONTENTS ${sourceCode})
configure_file(${TEMPLATE_FILE} CMakeLists.txt @ONLY)

