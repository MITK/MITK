mitk_create_module(
  DEPENDS PUBLIC MitkCore
  PACKAGE_DEPENDS
    PUBLIC CppUnit
    PRIVATE VTK|vtkTestingRendering tinyxml OpenGL
)

