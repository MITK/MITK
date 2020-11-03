project(org_mitk_gui_qt_mitkworkbench_intro)

mitk_create_plugin(
  EXPORT_DIRECTIVE MITK_QT_EXTAPP
  EXPORTED_INCLUDE_SUFFIXES src
  PACKAGE_DEPENDS Qt5|WebEngineWidgets
)
