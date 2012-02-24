project(org_mitk_gui_qt_dicom)

include_directories(${CTK_INCLUDE_DIRS})

MACRO_CREATE_MITK_CTK_PLUGIN(
  EXPORT_DIRECTIVE DICOM_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDENCIES QmitkExt mitkDicomUI CTKPluginFramework
)

target_link_libraries(${PLUGIN_TARGET} ${CTK_LIBRARIES})
