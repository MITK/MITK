MITK_CREATE_MODULE(DEPENDS MitkDataTypesExt MitkMapperExt MitkSceneSerialization MitkLegacyIO
                   PACKAGE_DEPENDS PRIVATE ITK|ITKIOImageBase VTK|vtkIOPLY+vtkIOExport+vtkIOParallelXML
                   AUTOLOAD_WITH MitkCore
                  )
