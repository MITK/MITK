project(org_mitk_gui_qt_mitkworkbench_intro)

mitk_create_plugin(
  EXPORT_DIRECTIVE MITK_QT_EXTAPP
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtHtml
  PACKAGE_DEPENDS Qt6|Widgets
)
