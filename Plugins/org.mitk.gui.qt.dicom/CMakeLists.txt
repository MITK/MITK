project(org_mitk_gui_qt_dicom)

set(QT_USE_QTSQL 1)
set(BUILD_TYPE_DIRECTORY Release)
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
set(BUILD_TYPE_DIRECTORY Debug)
endif()
set(DCMTK_INSTALL_BIN ${DCMTK_DIR}/bin/${BUILD_TYPE_DIRECTORY})

if(WIN32)
  set(DCMTK_STORESCP ${DCMTK_INSTALL_BIN}/storescp.exe)
else(WIN32)
  set(DCMTK_STORESCP ${DCMTK_INSTALL_BIN}/storescp)
endif()
include_directories(${CTK_INCLUDE_DIRS})

#find_program(DCMTK_STORESCP storescp PATHS "${DCMTK_INSTALL_BIN}" DOC "Dcmtk storage provider which is used to store dicom files which are transfered over network." NO_DEFAULT_PATH)
#find_program(DCMTK_STORESCP storescp)

if(NOT EXISTS ${DCMTK_STORESCP})
  message(SEND_ERROR "Couldn't find program storescp without the program query retrieve of the dicom plugin won't work!")

else(NOT EXISTS ${DCMTK_STORESCP})

configure_file( org_mitk_gui_qt_dicom_config.h.in org_mitk_gui_qt_dicom_config.h @ONLY)

MITK_INSTALL_HELPER_APP(${DCMTK_STORESCP})


MACRO_CREATE_MITK_CTK_PLUGIN(
  EXPORT_DIRECTIVE DICOM_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDENCIES QmitkExt mitkDicomUI
)

target_link_libraries(${PLUGIN_TARGET} ${CTK_LIBRARIES})
endif()
