MITK_CREATE_MODULE(
  INCLUDE_DIRS PRIVATE resource # for xpm includes
  DEPENDS
    PUBLIC MitkPlanarFigure MitkAnnotation
    PRIVATE MitkSegmentation
  PACKAGE_DEPENDS
    PUBLIC VTK|GUISupportQt+RenderingQt Qt6|Widgets+OpenGL+Core CTK|CTKWidgets
    PRIVATE nlohmann_json
)

add_subdirectory(test)
