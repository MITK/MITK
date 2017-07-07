MITK_CREATE_MODULE(
  DEPENDS MitkCore
  PACKAGE_DEPENDS
    PUBLIC tinyxml
    PRIVATE ITK|ITKIOImageBase+ITKIOGDCM DCMTK Poco
  WARNINGS_AS_ERRORS
)

add_subdirectory(autoload/IO)
add_subdirectory(test)
