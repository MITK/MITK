MITK_CREATE_MODULE(
  INCLUDE_DIRS PRIVATE resource # for xpm includes
  DEPENDS MitkPlanarFigure MitkAnnotation
  PACKAGE_DEPENDS
    PUBLIC ITK|ITKIOImageBase VTK|vtkGUISupportQt+vtkRenderingQt Qt5|Widgets+OpenGL+Core
)

add_subdirectory(test)
