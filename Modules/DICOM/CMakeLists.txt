MITK_CREATE_MODULE(
  DEPENDS MitkCore
  PACKAGE_DEPENDS
    PUBLIC tinyxml2 GDCM
    PRIVATE ITK|ITKIOImageBase+ITKIOGDCM DCMTK
)

add_subdirectory(test)
add_subdirectory(autoload/DICOMImageIO)
