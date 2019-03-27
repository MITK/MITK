project(org_mitk_gui_qt_mxnmultiwidgeteditor)

mitk_create_plugin(
  EXPORT_DIRECTIVE MXNMULTIWIDGETEDITOR_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgets MitkRenderWindowManager
)
