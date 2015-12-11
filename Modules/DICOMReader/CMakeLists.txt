MITK_CREATE_MODULE(
  DEPENDS MitkCore
  PACKAGE_DEPENDS
    PUBLIC tinyxml
    PRIVATE ITK|ITKIOGDCM DCMTK Boost
)

add_subdirectory(test)
