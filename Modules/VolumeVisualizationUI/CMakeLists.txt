mitk_create_module(
  DEPENDS MitkQtWidgets MitkVolumeVisualization PRIVATE MitkQtWidgetsExt
  PACKAGE_DEPENDS PRIVATE CTK|CTKWidgets VTK|RenderingVolumeOpenGL2
)
