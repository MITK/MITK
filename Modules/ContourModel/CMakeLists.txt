mitk_create_module(
  INCLUDE_DIRS Algorithms DataManagement IO Rendering
  DEPENDS MitkCore MitkSceneSerializationBase MitkLegacyGL MitkAnnotation MitkMultilabel
  PACKAGE_DEPENDS PRIVATE VTK|RenderingContext2D+RenderingContextOpenGL2
  # AUTOLOAD_WITH MitkCore TODO: Create IO Submodule and autoload that one instead.
)

add_subdirectory(Testing)
