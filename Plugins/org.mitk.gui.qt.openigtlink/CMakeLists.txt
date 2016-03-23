project(org_mitk_gui_qt_openigtlink)

mitk_create_plugin(
  DEPENDS
  EXPORT_DIRECTIVE IGTLPLUGIN_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt MitkOpenIGTLink MitkOpenIGTLinkUI MitkIGT
)
