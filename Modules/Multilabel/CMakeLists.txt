MITK_CREATE_MODULE(
  DEPENDS MitkCore MitkAlgorithmsExt MitkSceneSerializationBase MitkDICOMReader
  PACKAGE_DEPENDS PRIVATE ITK|ITKQuadEdgeMesh+ITKAntiAlias+ITKIONRRD
)

add_subdirectory(autoload/IO)
add_subdirectory(autoload/DICOMQIIO)
if(BUILD_TESTING)
 add_subdirectory(Testing)
endif()
