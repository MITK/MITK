mitk_create_module(
  INCLUDE_DIRS PRIVATE src/Algorithms src/DataManagement src/IO src/Rendering
  DEPENDS MitkCore MitkSceneSerializationBase MitkAnnotation MitkMultilabel
  PACKAGE_DEPENDS PRIVATE VTK|RenderingContext2D+RenderingContextOpenGL2
  # AUTOLOAD_WITH MitkCore TODO: Create IO Submodule and autoload that one instead.
)

add_subdirectory(Testing)
