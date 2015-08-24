MITK_CREATE_MODULE(
  DEPENDS MitkCLCore MitkCLUtilities MitkSceneSerializationBase
  #AUTOLOAD_WITH MitkCore
  PACKAGE_DEPENDS
  PUBLIC Boost
  PRIVATE Vigra ITK|ITKIONRRD
 WARNINGS_AS_ERRORS
)

add_subdirectory(test)
