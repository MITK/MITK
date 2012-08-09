project(org_mitk_gui_qt_dicom)

set(QT_USE_QTSQL 1)
set(DCMTK_INSTALL_BIN ${DCMTK_DIR}/bin)

include_directories(${CTK_INCLUDE_DIRS})

find_program(MITK_STORESCP storescp PATH ${DCMTK_INSTALL_BIN})
mark_as_advanced(MITK_STORESCP)

configure_file( org_mitk_gui_qt_dicom_config.h.in org_mitk_gui_qt_dicom_config.h @ONLY)

MITK_INSTALL_HELPER_APP(${MITK_STORESCP})


MACRO_CREATE_MITK_CTK_PLUGIN(
  EXPORT_DIRECTIVE DICOM_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDENCIES QmitkExt mitkDicomUI
)

target_link_libraries(${PLUGIN_TARGET} ${CTK_LIBRARIES})
