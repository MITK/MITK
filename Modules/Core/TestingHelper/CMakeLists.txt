mitk_create_module(
  DEPENDS PUBLIC MitkCore
  PACKAGE_DEPENDS
    PUBLIC CppUnit tinyxml2
    PRIVATE VTK|IOImage+TestingRendering
)
