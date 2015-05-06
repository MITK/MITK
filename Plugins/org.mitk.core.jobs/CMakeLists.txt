project(org_mitk_core_jobs)

mitk_create_plugin(
  EXPORT_DIRECTIVE MITK_JOBS_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkCore
)
