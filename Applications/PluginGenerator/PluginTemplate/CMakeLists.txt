project($(plugin-target))

MACRO_CREATE_MITK_CTK_PLUGIN(
  EXPORT_DIRECTIVE $(plugin-export-directive)
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt
)
