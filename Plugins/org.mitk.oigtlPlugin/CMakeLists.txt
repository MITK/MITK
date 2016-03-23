project(org_mitk_oigtlPlugin)

mitk_create_plugin(
  EXPORT_DIRECTIVE OIGTLPLUGIN_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt MitkOpenIGTLink MitkIGT MitkUS
)
