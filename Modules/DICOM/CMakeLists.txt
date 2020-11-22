MITK_CREATE_MODULE(
  DEPENDS MitkCore
  PACKAGE_DEPENDS
    PUBLIC tinyxml2|tinyxml2 GDCM
    PRIVATE ITK|ITKIOImageBase+ITKIOGDCM DCMTK
)

if(TARGET ${MODULE_TARGET})
  target_link_libraries(${MODULE_TARGET}
    PUBLIC tinyxml2::tinyxml2
  )
endif()

add_subdirectory(test)
add_subdirectory(autoload/DICOMImageIO)
