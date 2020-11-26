mitk_create_module(
  DEPENDS MitkImageExtraction MitkPlanarFigure MitkMultilabel
  PACKAGE_DEPENDS PRIVATE ITK|VTK VTK|IOImage
)

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
