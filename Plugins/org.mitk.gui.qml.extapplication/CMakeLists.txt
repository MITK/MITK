project(org_mitk_gui_qml_extapplication)

mitk_create_plugin(
  EXPORT_DIRECTIVE ORG_MITK_GUI_QML_EXTAPPLICATION
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQmlItems
  PACKAGE_DEPENDS Qt5|Core+Quick
  )
