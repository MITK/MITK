project(org_mitk_gui_qt_xnat)

mitk_create_plugin(
  PACKAGE_DEPENDS Poco|Zip
  MODULE_DEPENDS MitkXNAT
  EXPORT_DIRECTIVE XNAT_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
)
