MITK_CREATE_MODULE(DEPENDS MitkDataTypesExt MitkMapperExt MitkSceneSerialization MitkLegacyIO
                   PACKAGE_DEPENDS VTK|vtkIOPLY+vtkIOExport
                   WARNINGS_AS_ERRORS
                   AUTOLOAD_WITH MitkCore
                  )
