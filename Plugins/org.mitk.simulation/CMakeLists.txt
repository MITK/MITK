project(org_mitk_simulation)

mitk_create_plugin(
  EXPORT_DIRECTIVE SIMULATION_INIT_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgets MitkSimulation
)
