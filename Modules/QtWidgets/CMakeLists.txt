MITK_CREATE_MODULE(
  INCLUDE_DIRS PRIVATE resource # for xpm includes
  DEPENDS MitkPlanarFigure MitkAnnotation
  PACKAGE_DEPENDS
    PUBLIC VTK|GUISupportQt+RenderingQt Qt5|Widgets+OpenGL+Core CTK|CTKWidgets
)

add_subdirectory(test)
