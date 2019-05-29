MITK_CREATE_MODULE(
  DEPENDS MitkCore
  PACKAGE_DEPENDS
    PRIVATE ITK|ITKIOImageBase+ITKIOGDCM Poco
)

add_subdirectory(autoload/IO)
add_subdirectory(test)
