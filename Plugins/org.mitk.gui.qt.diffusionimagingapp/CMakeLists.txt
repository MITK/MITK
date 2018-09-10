project(org_mitk_gui_qt_diffusionimagingapp)

mitk_create_plugin(
  SUBPROJECTS MITK-DTI
  EXPORT_DIRECTIVE DIFFUSIONIMAGING_APP_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgets MitkSceneSerialization
  PACKAGE_DEPENDS Qt5|WebEngineWidgets
)
