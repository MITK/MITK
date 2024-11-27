project(org_mitk_gui_qt_dicombrowser)

find_program(DCMTK_STORESCP
  NAMES storescp storescp${DCMTK_CMAKE_DEBUG_POSTFIX} storescp${CMAKE_DEBUG_POSTFIX}
  PATHS "${MITK_EXTERNAL_PROJECT_PREFIX}/bin"
  PATH_SUFFIXES Release Debug
  DOC "The DCMTK storage provider for storing DICOM files transferred over the network."
  NO_DEFAULT_PATH
)

mark_as_advanced(DCMTK_STORESCP)

configure_file(mitkDicomBrowserConfig.h.in mitkDicomBrowserConfig.h @ONLY)

if(NOT EXISTS ${DCMTK_STORESCP})
  message(WARNING "Couldn't find storescp program: Query/retrieve of the DICOM plugin won't work!")
else()
  mitk_install_helper_app(EXECUTABLES ${DCMTK_STORESCP})
endif()

mitk_create_plugin(
  EXPORT_DIRECTIVE DICOM_BROWSER_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS
    PRIVATE
      MitkContourModel
      MitkDICOM
      MitkDICOMUI
      MitkQtWidgetsExt
      MitkRT
      MitkRTUI
  PACKAGE_DEPENDS
    PRIVATE
      CTK|CTKDICOMWidgets
)
