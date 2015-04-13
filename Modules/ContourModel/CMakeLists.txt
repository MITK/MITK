MITK_CREATE_MODULE(
  INCLUDE_DIRS Algorithms DataManagement IO Rendering
  DEPENDS MitkCore MitkSceneSerializationBase MitkLegacyGL MitkOverlays
  PACKAGE_DEPENDS ITK|ITKReview
  # AUTOLOAD_WITH MitkCore TODO: Create IO Submodule and autoload that one instead.
  WARNINGS_AS_ERRORS
)

add_subdirectory(Testing)
