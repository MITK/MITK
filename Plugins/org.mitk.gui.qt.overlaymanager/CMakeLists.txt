project(org_mitk_gui_qt_overlaymanager)

mitk_create_plugin(
  EXPORT_DIRECTIVE OVERLAYMANAGER_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt
)
