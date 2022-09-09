MITK_CREATE_MODULE(
  DEPENDS MitkCore
  PACKAGE_DEPENDS
    PUBLIC tinyxml2
    PRIVATE DCMTK|dcmdata+ofstd ITK|IOGDCM
  TARGET_DEPENDS
    PUBLIC gdcmMSFF
)

add_subdirectory(test)
add_subdirectory(autoload/DICOMImageIO)
