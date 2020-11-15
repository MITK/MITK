project(org_mitk_gui_qt_remeshing)

mitk_create_plugin(
  EXPORT_DIRECTIVE REMESHING_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgets MitkRemeshing
)
