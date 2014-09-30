# Called by ITK.cmake (ExternalProject_Add) as a patch for ITK to work with external GDCM 2.2.1
# and remove all itk video libs to resolve external windows linker errors with opencv

set(path "CMakeLists.txt")
file(STRINGS ${path} contents NEWLINE_CONSUME)
string(REPLACE "ITK_LIBRARY_DIRS}\")" "ITK_LIBRARY_DIRS}\" \"\${GDCM_DIR}/bin\")
               list(REMOVE_ITEM ITK_LIBRARIES ITKVideoBridgeOpenCV ITKVideoCore ITKVideoIO)" contents ${contents})
set(CONTENTS ${contents})
configure_file(${TEMPLATE_FILE} ${path} @ONLY)

# fix for double import targets
set(path "SimpleITKConfig.cmake.in")
file(STRINGS ${path} contents NEWLINE_CONSUME)
string(REPLACE "if(NOT ITK_TARGETS_IMPORTED)" "if(NOT TARGET ITKCommon)" contents ${contents})
set(CONTENTS ${contents})
string(REPLACE "if(NOT SimpleITK_TARGETS_IMPORTED)" "if(NOT TARGET SimpleITKCommon)" contents ${contents})
set(CONTENTS ${contents})
configure_file(${TEMPLATE_FILE} ${path} @ONLY)
