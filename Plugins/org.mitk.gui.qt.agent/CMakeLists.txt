project(org_mitk_gui_qt_agent)

mitk_create_plugin(
  EXPORT_DIRECTIVE AGENT_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkAgentUI
  PACKAGE_DEPENDS PRIVATE Qt6|Concurrent
)
