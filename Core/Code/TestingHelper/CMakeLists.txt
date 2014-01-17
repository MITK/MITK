mitk_create_module(MitkTestingHelper
  DEPENDS Mitk
  PACKAGE_DEPENDS VTK|vtkTestingRendering CppUnit
  EXPORT_DEFINE MITK_TESTINGHELPER_EXPORT
  WARNINGS_AS_ERRORS
)

