mitk_create_module(
  DEPENDS MitkCore
  PACKAGE_DEPENDS VTK|RenderingAnnotation
)

add_subdirectory(autoload/IO)
