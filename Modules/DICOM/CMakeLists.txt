MITK_CREATE_MODULE(
  DEPENDS MitkCore
  PACKAGE_DEPENDS
    PUBLIC tinyxml2
    PRIVATE DCMTK|dcmdata+ofstd ITK|IOGDCM
  TARGET_DEPENDS
    PUBLIC gdcmMSFF
)

# Before test: the CLI tests registered there reference the command-line app
# targets, which have to exist by then.
add_subdirectory(cmdapps)
add_subdirectory(test)
add_subdirectory(autoload/DICOMImageIO)
