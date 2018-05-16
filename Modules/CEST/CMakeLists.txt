MITK_CREATE_MODULE(
  DEPENDS MitkCore
  PACKAGE_DEPENDS
    PUBLIC tinyxml
    PRIVATE ITK|ITKIOImageBase+ITKIOGDCM DCMTK Poco
)

add_subdirectory(autoload/IO)
add_subdirectory(test)
