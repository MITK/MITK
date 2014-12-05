MITK_CREATE_MODULE(
  DEPENDS MitkImageExtraction MitkPlanarFigure
  PACKAGE_DEPENDS ITK|ITKVTK+ITKConvolution+ITKIOXML
  # WARNINGS_AS_ERRORS
)

if(BUILD_TESTING)

  add_subdirectory(Testing)

endif(BUILD_TESTING)
