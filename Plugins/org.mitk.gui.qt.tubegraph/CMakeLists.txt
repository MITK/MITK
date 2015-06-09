PROJECT(org_mitk_gui_qt_tubegraph)

mitk_create_plugin(
  EXPORT_DIRECTIVE TUBEGRAPH_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS  MitkTubeGraph  MitkQtWidgetsExt #QmitkExt MitkSegmentationUI
)
