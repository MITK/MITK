MITK_CREATE_MODULE(
  DEPENDS MitkCore
  PACKAGE_DEPENDS
    PUBLIC GDCM|MSFF tinyxml2
    PRIVATE DCMTK ITK|IOGDCM
)

add_subdirectory(test)
add_subdirectory(autoload/DICOMImageIO)
