# Create <name>Config.cmake file to make ACVD findable through config mode of find_package()

file(WRITE "ACVDConfig.cmake.in"
"set(ACVD_INCLUDE_DIRS \"@VTKSURFACE_INCLUDE_DIR@;@VTKDISCRETEREMESHING_INCLUDE_DIR@;@VTKVOLUMEPROCESSING_INCLUDE_DIR@\")
set(ACVD_LIBRARY_DIRS \"@VTKSURFACE_BINARY_DIR@/bin\")
set(ACVD_LIBRARIES vtkSurface vtkDiscreteRemeshing vtkVolumeProcessing)
add_definitions(-DDOmultithread)")

file(APPEND "CMakeLists.txt" "CONFIGURE_FILE(ACVDConfig.cmake.in ACVDConfig.cmake @ONLY)")

# Replace VTK_COMMON_EXPORT by VTK_EXPORT in class declarations

set(path "Common/vtkMyMinimalStandardRandomSequence.h")
file(STRINGS ${path} contents NEWLINE_CONSUME)
string(REPLACE "COMMON_" "" contents ${contents})
set(CONTENTS ${contents})
configure_file(${TEMPLATE_FILE} ${path} @ONLY)

set(path "DiscreteRemeshing/vtkThreadedClustering.h")
file(STRINGS ${path} contents NEWLINE_CONSUME)
string(REPLACE "COMMON_" "" contents ${contents})
set(CONTENTS ${contents})
configure_file(${TEMPLATE_FILE} ${path} @ONLY)

# Add missing VTK_EXPORT to class declaration

set(path "VolumeProcessing/vtkImageDataCleanLabels.h")
file(STRINGS ${path} contents NEWLINE_CONSUME)
string(REPLACE "ss" "ss VTK_EXPORT" contents ${contents})
set(CONTENTS ${contents})
configure_file(${TEMPLATE_FILE} ${path} @ONLY)

# Replace int by vtkIdType, which are types of different size (x64)

set(path "DiscreteRemeshing/vtkVerticesProcessing.h")
file(STRINGS ${path} contents NEWLINE_CONSUME)
string(REPLACE "int N" "vtkIdType N" contents ${contents})
set(CONTENTS ${contents})
configure_file(${TEMPLATE_FILE} ${path} @ONLY)

# Link to POSIX thread library

set(path "DiscreteRemeshing/CMakeLists.txt")
file(STRINGS ${path} contents NEWLINE_CONSUME)
string(REPLACE "TARGET_LINK_LIBRARIES(v" "IF(UNIX AND NOT APPLE)\n  LIST(APPEND LIB_ADDED pthread)\nENDIF(UNIX AND NOT APPLE)\n\nTARGET_LINK_LIBRARIES(v" contents ${contents})
set(CONTENTS ${contents})
configure_file(${TEMPLATE_FILE} ${path} @ONLY)
