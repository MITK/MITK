project(org_mitk_gui_qt_voxelvalue)

mitk_create_plugin(
  EXPORT_DIRECTIVE MITK_QT_VOXELVALUE
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt
)

