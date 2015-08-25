MITK_CREATE_MODULE(
  DEPENDS MitkCore MitkAlgorithmsExt MitkSceneSerializationBase
  PACKAGE_DEPENDS PRIVATE ITK|ITKQuadEdgeMesh+ITKAntiAlias+ITKIONRRD
  WARNINGS_AS_ERRORS
)

add_subdirectory(autoload/IO)
if(BUILD_TESTING)
 add_subdirectory(Testing)
endif()
