MITK_CREATE_MODULE(
  DEPENDS MitkCLCore MitkCLUtilities MitkSceneSerializationBase
  #AUTOLOAD_WITH MitkCore
  PACKAGE_DEPENDS
    PRIVATE ITK|IONRRD Vigra
)

add_subdirectory(test)
