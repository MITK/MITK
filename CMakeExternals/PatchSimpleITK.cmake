# Called by ITK.cmake (ExternalProject_Add) as a patch for ITK to work with external GDCM 2.2.1

set(path "CMakeLists.txt")
file(STRINGS ${path} contents NEWLINE_CONSUME)
string(REPLACE "ITK_LIBRARY_DIRS}\")" "ITK_LIBRARY_DIRS}\" \"\${GDCM_DIR}/bin\" \"\${OpenCV_DIR}/lib\")" contents ${contents})
set(CONTENTS ${contents})
configure_file(${TEMPLATE_FILE} ${path} @ONLY)
