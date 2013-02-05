project(org_mitk_gui_qt_python)

MACRO_CREATE_MITK_CTK_PLUGIN(
  EXPORT_DIRECTIVE org_mitk_gui_qt_python_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDENCIES QmitkExt CTK)

configure_file(PythonPath.h.in
  "${CMAKE_CURRENT_BINARY_DIR}/PythonPath.h" @ONLY)
