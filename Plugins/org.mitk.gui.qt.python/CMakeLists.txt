if(MITK_USE_Python)

project(org_mitk_gui_qt_python)

mitk_create_plugin(
  EXPORT_DIRECTIVE org_mitk_gui_qt_python_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt MitkPython MitkQtPython)

endif()
