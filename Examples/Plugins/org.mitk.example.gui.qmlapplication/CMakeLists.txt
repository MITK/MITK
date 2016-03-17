project(org_mitk_example_gui_qmlapplication)

mitk_create_plugin(
  EXPORT_DIRECTIVE ORG_MITK_EXAMPLE_GUI_QMLAPPLICATION
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQmlItems
  PACKAGE_DEPENDS Qt5|Core+Quick
  )
