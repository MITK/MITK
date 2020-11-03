project(org_mitk_gui_qt_dicominspector)

mitk_create_plugin(
  EXPORT_DIRECTIVE DICOM_INSPECTOR_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt MitkDICOM
)
