set(path "CMakeLists.txt")
file(STRINGS ${path} contents NEWLINE_CONSUME)
if(DESIRED_QT_VERSION MATCHES "5")
  string(REPLACE "find_package(VTK" "find_package(Qt5Widgets REQUIRED)\nfind_package(VTK" contents ${contents})
endif()
string(REPLACE "ENDIF(BUILD_VOLUMEPROCESSING)" "ENDIF(BUILD_VOLUMEPROCESSING)\n\nCONFIGURE_FILE(ACVDConfig.cmake.in ACVDConfig.cmake @ONLY)" contents ${contents})
set(CONTENTS ${contents})
configure_file(${TEMPLATE_FILE} ${path} @ONLY)

# Create <name>Config.cmake file to make ACVD findable through config mode of find_package()

file(WRITE "ACVDConfig.cmake.in"
"set(ACVD_INCLUDE_DIRS \"@VTKSURFACE_INCLUDE_DIR@;@VTKDISCRETEREMESHING_INCLUDE_DIR@;@VTKVOLUMEPROCESSING_INCLUDE_DIR@\")
set(ACVD_LIBRARY_DIRS \"@PROJECT_BINARY_DIR@/bin\")
set(ACVD_LIBRARIES vtkSurface vtkDiscreteRemeshing vtkVolumeProcessing)
add_definitions(-DDOmultithread)")

# Add vtkVersionMacros.h header file

set(path "Common/vtkCurvatureMeasure.cxx")
file(STRINGS ${path} contents NEWLINE_CONSUME)
string(REPLACE "vtkNeighbourhoodComputation.h\"" "vtkNeighbourhoodComputation.h\"\n#include <vtkVersionMacros.h>" contents ${contents})
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

# Replace VTK 5 module names by VTK 6 module names
# Link to POSIX thread library

set(path "DiscreteRemeshing/CMakeLists.txt")
file(STRINGS ${path} contents NEWLINE_CONSUME)
string(REPLACE "vtkCommon" "vtkCommonCore" contents ${contents})
string(REPLACE "vtkFiltering" "vtkFiltersCore" contents ${contents})
string(REPLACE "vtkImaging" "vtkImagingCore" contents ${contents})
string(REPLACE "vtkIO" "vtkIOCore" contents ${contents})
string(REPLACE "vtkRendering" "vtkRenderingCore" contents ${contents})
string(REPLACE "vtkHybrid" "vtkFiltersHybrid vtkImagingHybrid" contents ${contents})

string(REPLACE "ENDFOREACH(loop_var)" "ENDFOREACH()\nendif()" contents ${contents})
string(REPLACE "FOREACH(loop_var" "option(ACVD_BUILD_EXAMPLES \"build examples\" OFF)
if(ACVD_BUILD_EXAMPLES)
FOREACH(loop_var" contents ${contents})


set(CONTENTS ${contents})
configure_file(${TEMPLATE_FILE} ${path} @ONLY)
