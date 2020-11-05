MITK_CREATE_MODULE(
  DEPENDS MitkImageExtraction MitkPlanarFigure MitkMultilabel
  PACKAGE_DEPENDS
    PUBLIC  ITK|ITKIOXML
    PRIVATE ITK|ITKVTK+ITKConvolution VTK|IOImage
)

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
