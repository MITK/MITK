MITK_CREATE_MODULE(DEPENDS MitkDataTypesExt MitkMapperExt MitkSceneSerialization
                   PACKAGE_DEPENDS PRIVATE VTK|IOPLY+IOExport+IOParallelXML
                   AUTOLOAD_WITH MitkCore
                  )
