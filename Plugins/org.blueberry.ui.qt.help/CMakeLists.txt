project(org_blueberry_ui_qt_help)

set(QT_USE_QTHELP 1)

mitk_create_plugin(
  EXPORT_DIRECTIVE org_blueberry_ui_qt_help_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtHtml
  PACKAGE_DEPENDS PRIVATE Qt6|Help+Widgets+Xml
)
