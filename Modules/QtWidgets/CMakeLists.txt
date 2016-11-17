MITK_CREATE_MODULE(
  INCLUDE_DIRS PRIVATE resource # for xpm includes
  DEPENDS MitkPlanarFigure MitkAnnotation
  PACKAGE_DEPENDS
    PUBLIC ITK|ITKIOImageBase VTK|vtkGUISupportQt+vtkGUISupportQtOpenGL Qt5|Widgets+OpenGL+Core
  SUBPROJECTS MITK-CoreUI
)

add_subdirectory(test)
