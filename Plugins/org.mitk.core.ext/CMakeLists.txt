project(org_mitk_core_ext)

mitk_create_plugin(
  EXPORT_DIRECTIVE MITKCOREEXT_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkCore
)

