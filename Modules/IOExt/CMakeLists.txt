MITK_CREATE_MODULE(DEPENDS MitkDataTypesExt MitkMapperExt MitkSceneSerialization MitkLegacyIO
                   PACKAGE_DEPENDS PRIVATE VTK|IOPLY+IOExport+IOParallelXML
                   AUTOLOAD_WITH MitkCore
                  )
