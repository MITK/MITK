MITK_CREATE_MODULE(
  DEPENDS MitkCore
  PACKAGE_DEPENDS PUBLIC VTK|RenderingAnnotation PRIVATE VTK|IOImage
)

add_subdirectory(test)
