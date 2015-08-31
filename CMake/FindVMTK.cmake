include(FindPackageHandleStandardArgs)
include(SelectLibraryConfigurations)

find_path(VMTK_INCLUDE_DIR
  NAMES vtkvmtkConfigure.h
  PATHS ${MITK_EXTERNAL_PROJECT_PREFIX}
  PATH_SUFFIXES include/vmtk
)

set(VMTK_LIBRARY_DIR "${MITK_EXTERNAL_PROJECT_PREFIX}/lib/vmtk")

foreach(lib Common ComputationalGeometry DifferentialGeometry IO ITK Misc Segmentation)
  string(TOUPPER ${lib} LIB)
  if(NOT VMTK_${LIB}_LIBRARIES)
    find_library(VMTK_${LIB}_LIBRARY_RELEASE vtkvmtk${lib} ${VMTK_LIBRARY_DIR})
    find_library(VMTK_${LIB}_LIBRARY_DEBUG vtkvmtk${lib}d ${VMTK_LIBRARY_DIR})
    select_library_configurations(VMTK_${LIB})
  endif()
endforeach()

find_package_handle_standard_args(VMTK DEFAULT_MSG
  VMTK_INCLUDE_DIR
  VMTK_COMMON_LIBRARIES
  VMTK_COMPUTATIONALGEOMETRY_LIBRARIES
  VMTK_DIFFERENTIALGEOMETRY_LIBRARIES
  VMTK_IO_LIBRARIES
  VMTK_ITK_LIBRARIES
  VMTK_MISC_LIBRARIES
  VMTK_SEGMENTATION_LIBRARIES
)

if(VMTK_FOUND)
  set(VMTK_INCLUDE_DIRS ${VMTK_INCLUDE_DIR})
  set(VMTK_LIBRARIES
    ${VMTK_COMMON_LIBRARIES}
    ${VMTK_COMPUTATIONALGEOMETRY_LIBRARIES}
    ${VMTK_DIFFERENTIALGEOMETRY_LIBRARIES}
    ${VMTK_IO_LIBRARIES}
    ${VMTK_ITK_LIBRARIES}
    ${VMTK_MISC_LIBRARIES}
    ${VMTK_SEGMENTATION_LIBRARIES}
  )
endif()

mark_as_advanced(
  VMTK_INCLUDE_DIR
)
