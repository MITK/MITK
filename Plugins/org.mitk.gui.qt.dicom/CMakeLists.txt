project(org_mitk_gui_qt_dicom)

set(QT_USE_QTSQL 1)
set(DCMTK_INSTALL_BIN ${DCMTK_DIR}/bin)

include_directories(${CTK_INCLUDE_DIRS})

find_program( VAR_STORESCP storescp PATH ${DCMTK_INSTALL_BIN})
file(WRITE resources/config.txt ${VAR_STORESCP} )

MACRO_CREATE_MITK_CTK_PLUGIN(
  EXPORT_DIRECTIVE DICOM_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDENCIES QmitkExt mitkDicomUI
)

target_link_libraries(${PLUGIN_TARGET} ${CTK_LIBRARIES})
