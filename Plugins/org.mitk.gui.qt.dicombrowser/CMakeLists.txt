project(org_mitk_gui_qt_dicombrowser)

mitk_create_plugin(
  EXPORT_DIRECTIVE DICOM_BROWSER_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS
    PRIVATE
      MitkDICOM
      MitkDICOMUI
      MitkQtWidgetsExt
      MitkRT
  PACKAGE_DEPENDS
    PRIVATE
      CTK|CTKDICOMWidgets
)
