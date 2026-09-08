mitk_create_module(
  DEPENDS MitkQtWidgets MitkVolumeVisualization MitkQtWidgetsExt
  PACKAGE_DEPENDS PRIVATE CTK|CTKWidgets VTK|RenderingVolumeOpenGL2
)
