MITK_CREATE_MODULE(
  INCLUDE_DIRS PRIVATE resource # for xpm includes
  PCH
    mitkBaseRenderer.h
    mitkDataNode.h
    qvariant.h
    qwidget.h
  DEPENDS
    PUBLIC MitkPlanarFigure MitkAnnotation
  PACKAGE_DEPENDS
    PUBLIC VTK|GUISupportQt+RenderingQt Qt6|Concurrent+Widgets+OpenGL+Core CTK|CTKWidgets
    PRIVATE nlohmann_json
  AUTOMOC_FORCE_INCLUDES QmitkCustomVariants.h
)

add_subdirectory(test)
