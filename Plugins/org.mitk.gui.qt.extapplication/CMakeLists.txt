project(org_mitk_gui_qt_extapplication)

mitk_create_plugin(
  EXPORT_DIRECTIVE MITK_QT_EXTAPP
  EXPORTED_INCLUDE_SUFFIXES src
  PACKAGE_DEPENDS Qt4|QtWebKit Qt5|WebKitWidgets
)
