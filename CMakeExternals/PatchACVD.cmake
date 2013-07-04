file(WRITE "ACVDConfig.cmake.in"
"set(ACVD_INCLUDE_DIRS \"@VTKSURFACE_INCLUDE_DIR@;@VTKDISCRETEREMESHING_INCLUDE_DIR@;@VTKVOLUMEPROCESSING_INCLUDE_DIR@\")
set(ACVD_LIBRARY_DIRS \"@VTKSURFACE_BINARY_DIR@/bin\")
set(ACVD_LIBRARIES vtkSurface vtkDiscreteRemeshing vtkVolumeProcessing)
add_definitions(-DDOmultithread)")

file(APPEND "CMakeLists.txt" "CONFIGURE_FILE(ACVDConfig.cmake.in ACVDConfig.cmake @ONLY)")

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

set(path "DiscreteRemeshing/vtkVerticesProcessing.h")
file(STRINGS ${path} contents NEWLINE_CONSUME)
string(REPLACE "int N" "vtkIdType N" contents ${contents})
set(CONTENTS ${contents})
configure_file(${TEMPLATE_FILE} ${path} @ONLY)

set(path "VolumeProcessing/vtkImageDataCleanLabels.h")
file(STRINGS ${path} contents NEWLINE_CONSUME)
string(REPLACE "ss" "ss VTK_EXPORT" contents ${contents})
set(CONTENTS ${contents})
configure_file(${TEMPLATE_FILE} ${path} @ONLY)
