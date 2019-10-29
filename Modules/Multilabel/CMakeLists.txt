MITK_CREATE_MODULE(
  DEPENDS MitkCore MitkAlgorithmsExt MitkSceneSerializationBase MitkDICOMQI
  PACKAGE_DEPENDS PRIVATE ITK|ITKQuadEdgeMesh+ITKAntiAlias+ITKIONRRD
)

add_subdirectory(autoload/IO)
add_subdirectory(autoload/DICOMSegIO)
if(BUILD_TESTING)
 add_subdirectory(Testing)
endif()
