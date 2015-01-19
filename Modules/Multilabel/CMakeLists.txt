MITK_CREATE_MODULE(
  DEPENDS MitkCore MitkAlgorithmsExt MitkSceneSerializationBase
  PACKAGE_DEPENDS ITK|ITKQuadEdgeMesh+ITKAntiAlias
  WARNINGS_AS_ERRORS
)

if(BUILD_TESTING)
 add_subdirectory(Testing)
endif()
