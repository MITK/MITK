mitk_create_module(
  DEPENDS MitkImageExtraction MitkPlanarFigure MitkMultilabel
  PACKAGE_DEPENDS PRIVATE ITK|Convolution+FFT+VTK VTK|IOImage
)

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
