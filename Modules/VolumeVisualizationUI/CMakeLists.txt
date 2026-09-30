mitk_create_module(
  DEPENDS MitkQtWidgets MitkVolumeVisualization MitkQtWidgetsExt MitkAlgorithmsExt
  PACKAGE_DEPENDS PRIVATE CTK|CTKWidgets VTK|RenderingVolumeOpenGL2
)
