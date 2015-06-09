MITK_CREATE_MODULE(
  DEPENDS MitkCore
  PACKAGE_DEPENDS
    PUBLIC tinyxml
    PRIVATE ITK|ITKIOGDCM
)

add_subdirectory(test)
