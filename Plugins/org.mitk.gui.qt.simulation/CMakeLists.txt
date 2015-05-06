project(org_mitk_gui_qt_simulation)

mitk_create_plugin(
  EXPORT_DIRECTIVE SIMULATION_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgets MitkSimulation
)
