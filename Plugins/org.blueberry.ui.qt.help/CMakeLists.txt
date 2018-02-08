project(org_blueberry_ui_qt_help)

set(QT_USE_QTHELP 1)
set(QT_USE_QTWEBENGINE 1)
set(QT_USE_QTNETWORK 1)

mitk_create_plugin(
  EXPORT_DIRECTIVE org_blueberry_ui_qt_help_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  PACKAGE_DEPENDS PRIVATE Qt5|Help+OpenGL+PrintSupport+WebEngineWidgets+Xml
)
