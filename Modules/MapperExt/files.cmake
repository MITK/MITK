file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  mitkEnhancedPointSetVtkMapper3D.cpp
  mitkSplineVtkMapper3D.cpp
  mitkUnstructuredGridMapper2D.cpp
  mitkUnstructuredGridVtkMapper3D.cpp
  mitkVectorImageMapper2D.cpp
  mitkVolumeMapperVtkSmart3D.cpp

  vtkMaskedGlyph2D.cpp
  vtkMaskedGlyph3D.cpp
  vtkUnstructuredGridMapper.cpp

  vtkPointSetSlicer.cxx
)
