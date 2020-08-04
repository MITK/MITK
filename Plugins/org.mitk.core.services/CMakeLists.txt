project(org_mitk_core_services)

mitk_create_plugin(
  EXPORT_DIRECTIVE MITK_CORE_SERVICES_PLUGIN
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS PUBLIC MitkCore
)
