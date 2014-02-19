# Called by OpenCV.cmake (ExternalProject_Add) as a patch for OpenCV.
# Related bug: http://bugs.mitk.org/show_bug.cgi?id=5912


# fix issues with whitespace in source directories
file(STRINGS cmake/templates/OpenCVConfig.cmake.in sourceCode NEWLINE_CONSUME)
string(REPLACE "SET(OpenCV2_INCLUDE_DIRS @OpenCV2_INCLUDE_DIRS_CONFIGCMAKE@)" "SET(OpenCV2_INCLUDE_DIRS \"@OpenCV2_INCLUDE_DIRS_CONFIGCMAKE@\")" sourceCode ${sourceCode})
# set variable CONTENTS, which is substituted in TEMPLATE_FILE
set(CONTENTS ${sourceCode})
configure_file(${TEMPLATE_FILE} cmake/templates/OpenCVConfig.cmake.in @ONLY)
