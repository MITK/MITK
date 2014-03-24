# Called by OpenCV.cmake (ExternalProject_Add) as a patch for OpenCV.
# Related bug: http://bugs.mitk.org/show_bug.cgi?id=5912


# fix issues with whitespace in source directories
file(STRINGS cmake/templates/OpenCVConfig.cmake.in sourceCode NEWLINE_CONSUME)
string(REPLACE "SET(OpenCV2_INCLUDE_DIRS @OpenCV2_INCLUDE_DIRS_CONFIGCMAKE@)" "SET(OpenCV2_INCLUDE_DIRS \"@OpenCV2_INCLUDE_DIRS_CONFIGCMAKE@\")" sourceCode ${sourceCode})
# set variable CONTENTS, which is substituted in TEMPLATE_FILE
set(CONTENTS ${sourceCode})
configure_file(${TEMPLATE_FILE} cmake/templates/OpenCVConfig.cmake.in @ONLY)

# fix issues with add_custom_command and whitespace escaping
# at least for CMake 2.8.12.2, this patch is not needed
file(STRINGS cmake/OpenCVModule.cmake sourceCode NEWLINE_CONSUME)
string(REPLACE "COMMAND \${CMAKE_COMMAND} -DCL_DIR=\"\${CMAKE_CURRENT_SOURCE_DIR}/src/opencl\" -DOUTPUT=\"\${CMAKE_CURRENT_BINARY_DIR}/opencl_kernels.cpp\" -P \"\${OpenCV_SOURCE_DIR}/cmake/cl2cpp.cmake\"" "COMMAND \${CMAKE_COMMAND} -DCL_DIR=\${CMAKE_CURRENT_SOURCE_DIR}/src/opencl -DOUTPUT=\${CMAKE_CURRENT_BINARY_DIR}/opencl_kernels.cpp -P \${OpenCV_SOURCE_DIR}/cmake/cl2cpp.cmake" sourceCode ${sourceCode})
# set variable CONTENTS, which is substituted in TEMPLATE_FILE
set(CONTENTS ${sourceCode})
configure_file(${TEMPLATE_FILE} cmake/OpenCVModule.cmake @ONLY)
