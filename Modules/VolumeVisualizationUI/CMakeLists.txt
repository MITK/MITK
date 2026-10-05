mitk_create_module(
  DEPENDS MitkQtWidgets MitkVolumeVisualization MitkQtWidgetsExt MitkAlgorithmsExt
  PACKAGE_DEPENDS PRIVATE CTK|CTKWidgets VTK|ImagingCore+RenderingVolumeOpenGL2
)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
