project(org_mitk_gui_common)

mitk_create_plugin(
  EXPORT_DIRECTIVE MITK_GUI_COMMON_PLUGIN
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgets
)

