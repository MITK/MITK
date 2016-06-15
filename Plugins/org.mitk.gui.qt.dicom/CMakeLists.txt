project(org_mitk_gui_qt_dicom)

# Note:
# If we use an installed version of DCMTK then DCMTK_DIR points to the subdirectory
# of the installation directory (share/dcmtk) that contains DCMTKConfig.cmake.
# Therefore we look for the the storescp command in the '../../bin' directory, too.
find_program(DCMTK_STORESCP
  NAMES storescp storescp${DCMTK_CMAKE_DEBUG_POSTFIX} storescp${CMAKE_DEBUG_POSTFIX}
  PATHS "${DCMTK_DIR}/bin" "${DCMTK_DIR}/../../bin"
  PATH_SUFFIXES Release Debug
  DOC "Dcmtk storage provider which is used to store dicom files which are transfered over network."
  NO_DEFAULT_PATH
)
mark_as_advanced(DCMTK_STORESCP)

if(NOT EXISTS ${DCMTK_STORESCP})
  message(WARNING "Couldn't find program storescp without the program query retrieve of the dicom plugin won't work!")
else(NOT EXISTS ${DCMTK_STORESCP})

  configure_file( org_mitk_gui_qt_dicom_config.h.in org_mitk_gui_qt_dicom_config.h @ONLY)

  MITK_INSTALL_HELPER_APP( EXECUTABLES ${DCMTK_STORESCP})

  mitk_create_plugin(
    EXPORT_DIRECTIVE DICOM_EXPORT
    EXPORTED_INCLUDE_SUFFIXES src
    MODULE_DEPENDS MitkQtWidgetsExt MitkDicomUI MitkDicomRT MitkRTUI MitkDICOMReader
  )

endif()
