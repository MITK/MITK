project(org_mitk_gui_qt_dicom)

set(QT_USE_QTSQL 1)

include_directories(${CTK_INCLUDE_DIRS})

MACRO_CREATE_MITK_CTK_PLUGIN(
  EXPORT_DIRECTIVE DICOM_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDENCIES QmitkExt mitkDicomUI
)

target_link_libraries(${PLUGIN_TARGET} ${CTK_LIBRARIES})
