project($(plugin-target))

mitk_create_plugin(
  EXPORT_DIRECTIVE $(plugin-export-directive)
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt
)
