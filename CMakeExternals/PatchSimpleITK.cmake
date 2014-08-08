# Called by ITK.cmake (ExternalProject_Add) as a patch for ITK to work with external GDCM 2.2.1
# and remove all itk video libs to resolve external windows linker errors with opencv

set(path "CMakeLists.txt")
file(STRINGS ${path} contents NEWLINE_CONSUME)
string(REPLACE "ITK_LIBRARY_DIRS}\")" "ITK_LIBRARY_DIRS}\" \"\${GDCM_DIR}/bin\")
               list(REMOVE_ITEM ITK_LIBRARIES ITKVideoBridgeOpenCV ITKVideoCore ITKVideoIO)" contents ${contents})
set(CONTENTS ${contents})
configure_file(${TEMPLATE_FILE} ${path} @ONLY)
